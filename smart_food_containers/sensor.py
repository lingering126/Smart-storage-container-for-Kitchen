from datetime import datetime, timedelta
from homeassistant.helpers.entity import Entity
from homeassistant.components.sensor import SensorEntity
from homeassistant.helpers.typing import ConfigType, HomeAssistantType
from homeassistant.util import dt as dt_util

DOMAIN = "smart_food_containers"

async def async_setup_platform(hass: HomeAssistantType, config: ConfigType, async_add_entities, discovery_info=None):
    """Set up the Smart Food Container sensors."""
    async_add_entities([
        ConsumptionSensor(hass, "Food Container Daily Consumption", "g", "day"),
        ConsumptionSensor(hass, "Food Container Weekly Consumption", "g", "week"),
        ConsumptionSensor(hass, "Food Container Monthly Consumption", "g", "month")
    ])

class ConsumptionSensor(SensorEntity):
    def __init__(self, hass, name, unit, span):
        self._hass = hass
        self._name = name
        self._unit = unit
        self._span = span
        self._state = None
        self._attributes = {}

    @property
    def name(self):
        return self._name

    @property
    def state(self):
        return self._state

    @property
    def unit_of_measurement(self):
        return self._unit

    @property
    def extra_state_attributes(self):
        return self._attributes

    async def async_update(self):
        now = dt_util.now()
        if self._span == 'day':
            start_date = now.replace(hour=0, minute=0, second=0, microsecond=0) - timedelta(days=6)
            self._attributes['period'] = f"{start_date.strftime('%d %b')} - {now.strftime('%d %b')}"
        elif self._span == 'week':
            start_date = (now - timedelta(days=now.weekday() + 21)).replace(hour=0, minute=0, second=0, microsecond=0)
            self._attributes['period'] = f"{start_date.strftime('%d %b')} - {now.strftime('%d %b')}"
        elif self._span == 'month':
            start_date = now.replace(day=1, hour=0, minute=0, second=0, microsecond=0) - timedelta(days=330)
            self._attributes['period'] = f"{start_date.strftime('%b %Y')} - {now.strftime('%b %Y')}"

        # Calculate consumption here
        self._state = await self.calculate_consumption(start_date, now)

    async def calculate_consumption(self, start_date, end_date):
        # Placeholder for consumption calculation
        # In a real scenario, you would use the weight data from your containers
        # For this example, we'll return a random number
        import random
        return random.randint(100, 1000)