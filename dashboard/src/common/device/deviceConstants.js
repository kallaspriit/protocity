export const DEVICE_CLASS = {
	WEATHER_CONTROLLER: 'com_stagnationlab_c8y_driver_fragments_controllers_Weather',
	LIGHTING_CONTROLLER: 'com_stagnationlab_c8y_driver_fragments_controllers_Lighting',
	TRUCK_CONTROLLER: 'com_stagnationlab_c8y_driver_fragments_controllers_Truck',
	TRUCK_SOLAR_PANEL: 'com_stagnationlab_c8y_driver_fragments_sensors_AnalogInputSensor',
};

export const DEVICE_TITLE = {
	WEATHER_CONTROLLER: 'Weather controller',
	LIGHTING_CONTROLLER: 'Lighting controller',
	TRUCK_CONTROLLER: 'Truck controller',
	TRUCK_SOLAR_PANEL: 'Truck solar panel',
};

export const SUBSCRIPTION_TYPE = {
	DATA: 'managedobjects',
	MEASUREMENTS: 'measurements',
	EVENTS: 'events',
};

export const CREATE_SESSION = 'CREATE_SESSION';
export const GET_INVENTORY = 'GET_INVENTORY';
export const POLL_CLIENT = 'POLL_CLIENT';
export const GET_DEVICE_DATA = 'GET_DEVICE_DATA';
export const GET_DEVICE_MEASUREMENTS = 'GET_DEVICE_MEASUREMENTS';
export const SUBSCRIBE_DEVICE = 'SUBSCRIBE_DEVICE';
export const UNSUBSCRIBE_DEVICE = 'UNSUBSCRIBE_DEVICE';