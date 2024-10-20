#include <WiFi.h>
#include "EspMQTTClient.h"
#include <ArduinoJson.h>
#include <Adafruit_MLX90614.h>
#include "HX710.h"
#include <EEPROM.h>

// create an instance of the Mqtt client fro communication
EspMQTTClient client(
  "SSID",
  "Password",
  "192.168.178.76",  // MQTT Broker server ip
  "Mqtt",   // Can be omitted if not needed Username for the home assistant user used for connection
  "mqtt1",   // Can be omitted if not needed Password for the home assistant user used for connection 
  "TestClient",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);
int timelapsed;

//Name of the ingredient 
String name= "Sugar";
// Unique Id  for the container
String id= "scaletest1";

// Termperature sensor
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
//Analog digital converter for load cell
HX710 ps;
// hx710 pins
const int DOUT = 17;
const int PD_SCK = 16;

// Memory for storing Calibration data
#define EEPROM_SIZE 7

const int cal0a =0;
const int cal0b =1;
const int cal0c= 6;
const int calx =2;
const int calvala=3;
const int calvalb=4;
const int calvalc=5;

void setup()
{
  Serial.begin(115200);

  // Optional functionalities of EspMQTTClient
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
  timelapsed=millis();
  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1);
  };
  ps.initialize( PD_SCK , DOUT );
  EEPROM.begin(EEPROM_SIZE);
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  // on home assistant restart resend config data
  client.subscribe("ha/start",[](const String & payload){
    buildDiscoveryEntity();
  });
  // message that zero weight is on the scale so the value with zero weight can be stored
  client.subscribe(name+"/cal/0", [](const String & payload) {
    while( !ps.isReady() );
    ps.readAndSelectNextData( HX710_DIFFERENTIAL_INPUT_40HZ );
    int a=ps.getLastDifferentialInput();
    Serial.println(a);
    int x= a/10000;
    int y= (a-x*10000)/100;
    int z= (a-x*10000)%100;
    EEPROM.write(cal0a,x);
    EEPROM.write(cal0b,y);
    EEPROM.write(cal0c,z);
    Serial.println((EEPROM.read(cal0a)*10000)+EEPROM.read(cal0b)*100+EEPROM.read(calvalc));
    EEPROM.commit();
  });

  // message that specified weight x is on the scale so the value with specified weight x weight  and the value of x can be stored
  client.subscribe(name+"/cal/x", [](const String & topic, const String & payload) {
    EEPROM.write(calx, payload.toInt());
    while( !ps.isReady() );
    ps.readAndSelectNextData( HX710_DIFFERENTIAL_INPUT_40HZ );
    int b= ps.getLastDifferentialInput();
    Serial.println(b);
    int x= b/10000;
    int y= (b-x*10000)/100;
    int z= (b-x*10000)%100;
    EEPROM.write(calvala,x);
    EEPROM.write(calvalb,y);
    EEPROM.write(calvalc,z);
    Serial.println((EEPROM.read(calvala)*10000)+EEPROM.read(calvalb)*100+EEPROM.read(calvalc));
    EEPROM.commit();
  });

  //Startup messages that demonstrate a connection
  // Publish a message to "mytopic/test"
  client.publish("mytopic/test", "This is a message"); // You can activate the retain flag by setting the third parameter to true
  buildDiscoveryEntity();
  // Execute delayed instructions
  client.executeDelayed(5 * 1000, []() {
    client.publish("mytopic/wildcardtest/test123", "This is a message sent 5 seconds later");
  });
}


// Function for sending sensor information to the MQTT integration
void buildDiscoveryEntity(){
  JsonDocument doc;
  doc["name"]= name+" temperature";
  doc["unique_id"]=id+"temp";
  doc["state_topic"]=name+"/temp";
  doc["unit_of_measurement"]="C";
  String output;
  serializeJson(doc,output);
  Serial.println("homeassistant/sensor/"+id+"temp/config");
  client.publish("homeassistant/sensor/"+id+"temp/config",output);
  doc["name"]= name+" weight";
  doc["unique_id"]=id+"w";
  doc["state_topic"]=name+"/w";
  doc["unit_of_measurement"]="g";
  
  serializeJson(doc,output);
  Serial.println("homeassistant/sensor/"+id+"w/config");
  client.publish("homeassistant/sensor/"+id+"w/config",output);
}

//Function for calibrating weight
//convert sensor output into  weight in grams based on a linear regression using stored values
int calweight(){
  while( !ps.isReady() );
  ps.readAndSelectNextData( HX710_DIFFERENTIAL_INPUT_40HZ );
  float ret=ps.getLastDifferentialInput();
  ret-= ((EEPROM.read(cal0a)*10000)+EEPROM.read(cal0b)*100+EEPROM.read(calvalc));
  ret=ret/((EEPROM.read(calvala)*10000)+EEPROM.read(calvalb)*100+EEPROM.read(calvalc)-((EEPROM.read(cal0a)*10000)+EEPROM.read(cal0b)*100+EEPROM.read(calvalc)));
  ret=ret*(EEPROM.read(calx));
  return ret;

}

//Send Mqtt data every 10 seconds
void loop()
{
  client.loop();
  if(millis()-timelapsed>10000){
    Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC());
    client.publish(name+"/temp",String(mlx.readAmbientTempC()));
    client.publish(name+"/w",String(calweight()));
    timelapsed=millis();
  }
}
