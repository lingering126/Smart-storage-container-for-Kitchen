import logging
from homeassistant.core import HomeAssistant
from homeassistant.helpers.typing import ConfigType
from homeassistant.helpers.entity_component import EntityComponent
from homeassistant.components.input_text import DOMAIN as INPUT_TEXT_DOMAIN
from homeassistant.components.sensor import DOMAIN as SENSOR_DOMAIN

_LOGGER = logging.getLogger(__name__)
DOMAIN = "smart_food_containers"

async def async_setup(hass: HomeAssistant, config: ConfigType) -> bool:
    """Set up the Smart Food Containers component."""
    hass.data[DOMAIN] = {}

    async def add_container(call):
        """Service to add a new container."""
        name = call.data.get("name")
        if name:
            # Create input_text for container name
            await hass.services.async_call(
                INPUT_TEXT_DOMAIN,
                "set_value",
                {"entity_id": f"input_text.container_{name.lower()}_name", "value": name},
            )

            # Create sensors for weight and temperature
            component = EntityComponent(_LOGGER, SENSOR_DOMAIN, hass)
            await component.async_add_entities([
                TemplateSensor(f"sensor.container_{name.lower()}_weight", f"{name} Weight", "g"),
                TemplateSensor(f"sensor.container_{name.lower()}_temperature", f"{name} Temperature", "°C")
            ])

            _LOGGER.info(f"Added new container: {name}")

    hass.services.async_register(DOMAIN, "add_container", add_container)

    return True

class TemplateSensor:
    def __init__(self, entity_id, name, unit):
        self.entity_id = entity_id
        self._name = name
        self._unit = unit
        self._state = None

    @property
    def name(self):
        return self._name

    @property
    def state(self):
        return self._state

    @property
    def unit_of_measurement(self):
        return self._unit

    async def async_update(self):
        # In a real scenario, you would update the state based on your sensor data
        import random
        self._state = random.randint(0, 100)