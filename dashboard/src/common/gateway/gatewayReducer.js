import { handleActions } from 'redux-actions';
import {
	Action,
	DeviceTitle,
	SubscriptionType,
	EventType,
	MEASUREMENT_MAX_HISTORY_SIZE,
	GATEWAY_DRIVER_NAME,
} from './gatewayConstants';

const createDevice = data => ({
	isLoading: false,
	hasDataSubscription: false,
	hasMeasurementsSubscription: false,
	hasEventsSubscription: false,
	data: {
		...data,
	},
	measurements: {},
	series: {},
	lastMotionDetectedTime: 0,
	lastActivatedTime: 0,
});

const initialState = {
	clientId: null,
	isPolling: false,
	deviceIds: {},
	devices: Object.keys(DeviceTitle).reduce((obj, key) => ({
		...obj,
		[key]: createDevice(),
	}), {}),
	isInventoryLoaded: false,
	error: null,
};

export default handleActions({
	[Action.CREATE_SESSION]: (state, action) => {
		const {
			error,
			isLoading,
			payload,
		} = action;

		if (isLoading) {
			return state;
		}

		if (error) {
			return {
				...state,
				error,
			};
		}

		const clientId = payload ? payload[0].clientId : null;

		return {
			...state,
			clientId,
			error,
		};
	},

	[Action.DESTROY_SESSION]: (state, action) => {
		const {
			error,
		} = action;

		return {
			...state,
			clientId: null,
			inventory: {},
			error,
		};
	},

	[Action.GET_INVENTORY]: (state, action) => {
		const {
			error,
			isLoading,
			payload,
		} = action;

		if (isLoading) {
			return state;
		}

		if (error) {
			return {
				...state,
				error,
			};
		}

		const getReferences = (device, references = []) => {
			if (device.childDevices) {
				references.concat(
					...device.childDevices.references.map(childDevice => getReferences(childDevice, references)),
				);
			} else if (device.managedObject) {
				references.push(device.managedObject);
			}

			references.push(device);

			return references;
		};

		const availableDevices = getReferences({ childDevices: { references: payload } });

		const filteredDeviceKeys = Object.keys(DeviceTitle);
		const filteredDeviceNames = Object.values(DeviceTitle);

		const deviceIds = availableDevices.reduce((obj, device) => {
			const deviceKey = filteredDeviceNames.indexOf(device.name);

			if (deviceKey !== -1) {
				return {
					...obj,
					[filteredDeviceKeys[deviceKey]]: device.id,
				};
			}

			return obj;
		}, {});
		return {
			...state,
			deviceIds,
			isInventoryLoaded: true,
			error,
		};
	},

	[Action.GET_DEVICE_DATA]: (state, action) => {
		const {
			isLoading,
			error,
			payload,
			meta,
		} = action;

		if (error) {
			return {
				...state,
				error,
			};
		}


		const device = {
			...state.devices[meta.name],
			isLoading,
		};

		if (payload) {
			const deviceClassName = Object.keys(payload).find(key => key.includes(GATEWAY_DRIVER_NAME));

			device.data = {
				...device.data,
				...payload[deviceClassName],
			};
		}

		return {
			...state,
			error,
			devices: {
				...state.devices,
				[meta.name]: device,
			},
		};
	},

	[Action.GET_DEVICE_MEASUREMENTS]: (state, action) => {
		const {
			isLoading,
			error,
			payload,
			meta,
		} = action;

		if (isLoading || error || !payload) {
			return {
				...state,
				error,
			};
		}

		const device = {
			...state.devices[meta.name],
			isLoading,
		};


		// define series
		device.series = payload.series.reduce((obj, row) => ({
			...obj,
			[row.name]: row.type,
		}), {});

		const seriesNames = Object.keys(device.series);

		// define measurements
		device.measurements = seriesNames.reduce((obj, key) => ({
			...obj,
			[key]: [],
		}), {});

		// populate measurements data
		Object.entries(payload.values).forEach(([time, row]) => {
			row.forEach((series, index) => {
				if (!series) {
					return;
				}

				const seriesName = seriesNames[index];

				if (seriesName) {
					device.measurements[seriesName].push([new Date(time).getTime(), series.max]);

					if (device.measurements[seriesName].length > MEASUREMENT_MAX_HISTORY_SIZE) {
						device.measurements[seriesName].shift();
					}
				}
			});
		});

		return {
			...state,
			error,
			devices: {
				...state.devices,
				[meta.name]: device,
			},
		};
	},

	[Action.POLL_CLIENT]: (state, action) => {
		const {
			isLoading,
			error,
			payload,
		} = action;

		if (!Array.isArray(payload) || isLoading) {
			return {
				...state,
				error,
				isPolling: isLoading,
			};
		}

		const devices = {
			...state.devices,
		};

		payload.forEach((stream) => {
			if (!stream.data) {
				return;
			}

			const data = stream.data.data;

			// get device name and class name
			const deviceId = data.source ? data.source.id : data.id;
			const deviceIndex = Object.values(state.deviceIds).findIndex(id => id === deviceId);
			const deviceName = Object.keys(state.deviceIds)[deviceIndex];
			const deviceClassName = Object.keys(data).find(key => key.includes(GATEWAY_DRIVER_NAME));
			const deviceData = data[deviceClassName];
			const time = new Date(data.time).getTime();

			if (!deviceName) {
				return;
			}

			if (stream.channel.includes(SubscriptionType.DATA) && deviceClassName) {
				if (!devices[deviceName]) {
					return;
				}

				// rewrite data
				devices[deviceName].data = {
					...devices[deviceName].data,
					...deviceData,
				};
			} else if (stream.channel.includes(SubscriptionType.MEASUREMENTS) && deviceClassName) {
				// loop all keys and push new info to that Array
				// make sure to keep array size consistent to prevent memory leak
				Object.entries(deviceData).forEach(([key, info]) => {
					const measurementArray = devices[deviceName].measurements[key];
					if (!measurementArray) {
						// initial data has not been loaded
						return;
					}

					measurementArray.push([time, info.value]);

					if (measurementArray.length > MEASUREMENT_MAX_HISTORY_SIZE) {
						measurementArray.shift();
					}
				});
			} else if (stream.channel.includes(SubscriptionType.EVENTS)) {
				switch (data.type) {
					case EventType.MOTION_DETECTED:
						devices[deviceName].lastMotionDetectedTime = new Date(time).getTime();
						break;

					case EventType.CONTROLLER_ACTIVATED:
						devices[deviceName].lastActivatedTime = new Date(time).getTime();
						break;

					// no default
				}
			}
		});


		return {
			...state,
			error,
			devices,
			isPolling: isLoading,
		};
	},

	[Action.SUBSCRIBE_DEVICE]: (state, action) => {
		const {
			error,
			payload,
			isLoading,
			meta,
		} = action;

		if (isLoading) {
			return state;
		}

		if (error) {
			return {
				...state,
				error,
			};
		}

		const isSuccessful = !!(payload && payload[0].successful);

		const device = {
			...state.devices[meta.name],
		};

		switch (meta.subscriptionType) {
			case SubscriptionType.DATA:
				device.hasDataSubscription = isSuccessful;
				break;

			case SubscriptionType.MEASUREMENTS:
				device.hasMeasurementsSubscription = isSuccessful;
				break;

			case SubscriptionType.EVENTS:
				device.hasEventsSubscription = isSuccessful;
				break;

			default:
				throw new Error(`unhandled subscription type: "${meta.subscriptionType}"`);
		}

		return {
			...state,
			error,
			devices: {
				...state.devices,
				[meta.name]: device,
			},
		};
	},

	[Action.UNSUBSCRIBE_DEVICE]: (state, action) => {
		const {
			error,
			payload,
			isLoading,
			meta,
		} = action;

		if (isLoading) {
			return state;
		}

		if (error) {
			return {
				...state,
				error,
			};
		}

		const isSuccessful = !!(payload && payload[0].successful);

		const device = {
			...state.devices[meta.name],
		};

		switch (meta.subscriptionType) {
			case SubscriptionType.DATA:
				device.hasDataSubscription = !isSuccessful;
				break;

			case SubscriptionType.MEASUREMENTS:
				device.hasMeasurementsSubscription = !isSuccessful;
				break;

			case SubscriptionType.EVENTS:
				device.hasEventsSubscription = !isSuccessful;
				break;

			default:
				throw new Error(`unhandled subscription type: "${meta.subscriptionType}"`);
		}

		return {
			...state,
			error,
			devices: {
				...state.devices,
				[meta.name]: device,
			},
		};
	},
}, initialState);