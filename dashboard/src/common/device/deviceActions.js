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

export const pollDeviceData = createAction(
	deviceConstants.POLL_DEVICE_DATA,
	deviceApi.startPolling,
);

export const subscribeToDeviceData = createAction(
	deviceConstants.SUBSCRIBE_TO_DEVICE_DATA,
	(clientId, deviceId) => deviceApi.subscribe(clientId, deviceId),
	(clientId, deviceId, name) => ({ name }),
);