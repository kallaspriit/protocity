export const Device = {
	WEATHER_CONTROLLER: 'WEATHER_CONTROLLER',
	LIGHTING_CONTROLLER: 'LIGHTING_CONTROLLER',
	TRUCK_CONTROLLER: 'TRUCK_CONTROLLER',
	TRUCK_SOLAR_PANEL: 'TRUCK_SOLAR_PANEL',
	PARKING_CONTROLLER: 'PARKING_CONTROLLER',
};

export const DeviceTitle = {
	[Device.WEATHER_CONTROLLER]: 'Weather controller',
	[Device.LIGHTING_CONTROLLER]: 'Lighting controller',
	[Device.TRUCK_CONTROLLER]: 'Truck controller',
	[Device.TRUCK_SOLAR_PANEL]: 'Truck solar panel',
	[Device.PARKING_CONTROLLER]: 'Parking controller',
};

export const DeviceClass = {
	[Device.WEATHER_CONTROLLER]: 'com_stagnationlab_c8y_driver_fragments_controllers_Weather',
	[Device.LIGHTING_CONTROLLER]: 'com_stagnationlab_c8y_driver_fragments_controllers_Lighting',
	[Device.TRUCK_CONTROLLER]: 'com_stagnationlab_c8y_driver_fragments_controllers_Truck',
	[Device.TRUCK_SOLAR_PANEL]: 'com_stagnationlab_c8y_driver_fragments_sensors_AnalogInputSensor',
	[Device.PARKING_CONTROLLER]: 'com_stagnationlab_c8y_driver_fragments_controllers_Parking',
};

export const SubscriptionType = {
	DATA: 'managedobjects',
	MEASUREMENTS: 'measurements',
	EVENTS: 'events',
};

export const Action = {
	CREATE_SESSION: 'CREATE_SESSION',
	GET_INVENTORY: 'GET_INVENTORY',
	POLL_CLIENT: 'POLL_CLIENT',
	GET_DEVICE_DATA: 'GET_DEVICE_DATA',
	GET_DEVICE_MEASUREMENTS: 'GET_DEVICE_MEASUREMENTS',
	SUBSCRIBE_DEVICE: 'SUBSCRIBE_DEVICE',
	UNSUBSCRIBE_DEVICE: 'UNSUBSCRIBE_DEVICE',
};