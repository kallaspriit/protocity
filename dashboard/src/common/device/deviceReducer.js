import { handleActions } from 'redux-actions';
import * as deviceConstants from './deviceConstants';

const createDevice = data => ({
	isLoading: false,
	hasDataSubscription: false,
	data: {
		...data,
	},
	measurements: {},
});

const initialState = {
	clientId: null,
	isPolling: false,
	inventory: {},
	devices: {
		WEATHER_CONTROLLER: createDevice({
			humidity: 0,
			lightLevel: 0,
			pressure: 0,
			soundLevel: 0,
			temperature: 0,
		}),
		LIGHTING_CONTROLLER: createDevice({
			outputLightLevel: 0,
			detectedLightLevel: 0,
		}),
	},
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
			} else {
				references.push(device.managedObject);
			}

			return references;
		};


		// const availableDevices = [].concat(...payload.managedObjects.map(device => getReferences(device)));
		const availableDevices = getReferences(payload.managedObjects[0]);

		const filteredDeviceKeys = Object.keys(deviceConstants.DEVICE_TITLE);
		const filteredDeviceNames = Object.values(deviceConstants.DEVICE_TITLE);

		const inventory = availableDevices.reduce((obj, device) => {
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
			inventory,
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

	[deviceConstants.POLL_DEVICE_DATA]: (state, action) => {
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
			const deviceData = payload[0].data.data[deviceClass];

			if (deviceData) {
				devices[deviceKey].data = deviceData;
			}
		});

		return {
			...state,
			error,
			devices,
			isPolling: isLoading,
		};
	},

	[deviceConstants.SUBSCRIBE_TO_DEVICE_DATA]: (state, action) => {
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


		return {
			...state,
			error,
			devices: {
				...state.devices,
				[meta.name]: {
					...state.devices[meta.name],
					hasDataSubscription: !!(payload && payload[0].successful),
				},
			},
		};
	},
}, initialState);