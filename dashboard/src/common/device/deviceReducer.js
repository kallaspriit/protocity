import { handleActions } from 'redux-actions';
import * as deviceConstants from './deviceConstants';

const createDevice = data => ({
	isLoading: false,
	hasDataSubscription: false,
	hasMeasurementsSubscription: false,
	data: {
		...data,
	},
	measurements: {},
	series: {},
});

const initialState = {
	clientId: null,
	isPolling: false,
	deviceIds: {},
	devices: Object.keys(deviceConstants.DEVICE_TITLE).reduce((obj, key) => ({
		...obj,
		[key]: createDevice(),
	}), {}),
	isInventoryLoaded: false,
	error: null,
};

export default handleActions({
	[deviceConstants.CREATE_SESSION]: (state, action) => {
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

	[deviceConstants.DESTROY_SESSION]: (state, action) => {
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

	[deviceConstants.GET_INVENTORY]: (state, action) => {
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

		const filteredDeviceKeys = Object.keys(deviceConstants.DEVICE_TITLE);
		const filteredDeviceNames = Object.values(deviceConstants.DEVICE_TITLE);

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

	[deviceConstants.GET_DEVICE_DATA]: (state, action) => {
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
			const deviceClassName = deviceConstants.DEVICE_CLASS[meta.name];

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

	[deviceConstants.GET_DEVICE_MEASUREMENTS]: (state, action) => {
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
				} else {
					// console.log('seriesName', index, device.series, payload.values)
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

	[deviceConstants.POLL_CLIENT]: (state, action) => {
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

		Object.entries(deviceConstants.DEVICE_CLASS).forEach(([deviceKey, deviceClass]) => {
			payload.forEach((stream) => {
				if (stream.channel.includes(deviceConstants.SUBSCRIPTION_TYPE.DATA)) {
					devices[deviceKey].data = {
						...devices[deviceKey].data,
						...stream.data.data[deviceClass],
					};
				} else if (stream.channel.includes(deviceConstants.SUBSCRIPTION_TYPE.MEASUREMENTS)) {
					// sound
					if (
						stream.data.data[deviceConstants.MEASUREMENT_CLASS.SOUND]
						&& devices[deviceKey].measurements.soundLevel
					) {
						devices[deviceKey].measurements.soundLevel.push({
							value: stream.data.data[deviceConstants.MEASUREMENT_CLASS.SOUND].soundLevel.value,
							time: stream.data.data.time,
						});

						if (devices[deviceKey].measurements.soundLevel.length > 100) {
							devices[deviceKey].measurements.soundLevel.shift();
						}
					}
				}
			});
		});

		return {
			...state,
			error,
			devices,
			isPolling: isLoading,
		};
	},

	[deviceConstants.SUBSCRIBE_DEVICE]: (state, action) => {
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
			case deviceConstants.SUBSCRIPTION_TYPE.DATA:
				device.hasDataSubscription = isSuccessful;
				break;

			case deviceConstants.SUBSCRIPTION_TYPE.MEASUREMENTS:
				device.hasMeasurementsSubscription = isSuccessful;
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

	[deviceConstants.UNSUBSCRIBE_DEVICE]: (state, action) => {
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
			case deviceConstants.SUBSCRIPTION_TYPE.DATA:
				device.hasDataSubscription = !isSuccessful;
				break;

			case deviceConstants.SUBSCRIPTION_TYPE.MEASUREMENTS:
				device.hasMeasurementsSubscription = !isSuccessful;
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