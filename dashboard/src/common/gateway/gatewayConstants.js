export const Device = {
	WEATHER_CONTROLLER: 'WEATHER_CONTROLLER',
	LIGHTING_CONTROLLER: 'LIGHTING_CONTROLLER',
	TRUCK_CONTROLLER: 'TRUCK_CONTROLLER',
	TRUCK_SOLAR_PANEL: 'TRUCK_SOLAR_PANEL',
	PARKING_CONTROLLER: 'PARKING_CONTROLLER',
	TRAIN_CONTROLLER: 'TRAIN_CONTROLLER',
	TRAIN_MOTION_SENSOR: 'TRAIN_MOTION_SENSOR',
};

export const DeviceTitle = {
	[Device.WEATHER_CONTROLLER]: 'Weather controller',
	[Device.LIGHTING_CONTROLLER]: 'Lighting controller',
	[Device.TRUCK_CONTROLLER]: 'Truck controller',
	[Device.TRUCK_SOLAR_PANEL]: 'Truck solar panel',
	[Device.PARKING_CONTROLLER]: 'Parking controller',
	[Device.TRAIN_CONTROLLER]: 'Train controller',
	[Device.TRAIN_MOTION_SENSOR]: 'Motion sensor: train',
};

export const SubscriptionType = {
	DATA: 'managedobjects',
	MEASUREMENTS: 'measurements',
	EVENTS: 'events',
};


export const EventType = {
	MOTION_DETECTED: 'MotionDetectedEvent',
	CONTROLLER_ACTIVATED: 'ControllerActivatedEvent',
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

export const GATEWAY_DRIVER_NAME = 'com_stagnationlab_c8y_driver';
export const MEASUREMENT_FROM_MINUTES_AGO = 15;
export const MEASUREMENT_MAX_HISTORY_SIZE = 100;