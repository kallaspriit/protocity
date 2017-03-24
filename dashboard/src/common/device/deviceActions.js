import { createAction } from 'redux-actions';
import * as deviceConstants from './deviceConstants';
import * as deviceApi from '../../api/deviceApi';

export const createSession = createAction(
	deviceConstants.CREATE_SESSION,
	deviceApi.createSession,
);

export const destroySession = createAction(
	deviceConstants.DESTROY_SESSION,
	deviceApi.destroySession,
);

export const getInventory = createAction(
	deviceConstants.GET_INVENTORY,
	fragmentType => deviceApi.getInventory(fragmentType),
);

export const getDeviceData = createAction(
	deviceConstants.GET_DEVICE_DATA,
	deviceId => deviceApi.getDeviceData(deviceId),
	(deviceId, name) => ({ name }),
);

export const getDeviceMeasurements = createAction(
	deviceConstants.GET_DEVICE_MEASUREMENTS,
	deviceId => deviceApi.getDeviceData(deviceId),
	(deviceId, name) => ({ name }),
);

export const pollDeviceData = createAction(
	deviceConstants.POLL_CLIENT,
	deviceApi.startPolling,
);

export const subscribeDevice = createAction(
	deviceConstants.SUBSCRIBE_DEVICE,
	(clientId, deviceId, name, subscriptionType) => deviceApi.subscribe(clientId, deviceId, subscriptionType),
	(clientId, deviceId, name, subscriptionType) => ({ name, subscriptionType }),
);

export const unsubscribeDevice = createAction(
	deviceConstants.UNSUBSCRIBE_DEVICE,
	(clientId, deviceId, name, subscriptionType) => deviceApi.unsubscribe(clientId, deviceId, subscriptionType),
	(clientId, deviceId, name, subscriptionType) => ({ name, subscriptionType }),
);