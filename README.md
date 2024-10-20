# CITS 5506 Project
## Smart Storage Container for Kitchen
This is our repo for our project. The MQTT configuration file is in `IOTprojectMQTT.ino`.
### How to
#### Home Assistant
After Following the setup steps found in the report the next step is to get the complete.tar backup of Home Assistant from the github page and use it to backup the on to the new Home Assistant device using this guide.
If this causes the Home Assistant to stop responding, you may need to connect a monitor and keyboard to the raspberry Pi to set the device back to the correct wifi using the Home Assistant command line interface as seen in this video. 
#### ESP32 connection
In order to program the IOTprojectMQTT.ino file onto the ESP32 first some libraries found in the IDE’s library manager need to be installed ArduinoJSON, the Adafriut library for the MLX90614, the hx710 library, and the EspMQTTClient, #include <WiFi.h> will need to be added to the first line of this library or it will not compile. 
Once this is done, changes will need to be made to the data in the client(), notably the SSID and Password will need to be changed to the ssid and password of the wifi network and 192.168.178.76  will need to be changed to the IP address of the device running Home Assistant. With this and by selecting the firebeetle 2 ESP32-E board and the correct COM port the code should compile and upload, MQTT messages should appear in the serial monitor and if configured correctly a two new entities will appear in Home Assistant by the name of Salt_weight and Salt_temperature.
