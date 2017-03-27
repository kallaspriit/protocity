import { createAction } from 'redux-actions';
import { Action, MEASUREMENT_FROM_MINUTES_AGO } from './gatewayConstants';
import * as deviceApi from '../../api/deviceApi';
import { minutesAgo } from '../../utils';

export const createSession = createAction(
	Action.CREATE_SESSION,
	deviceApi.createSession,
);

export const destroySession = createAction(
	Action.DESTROY_SESSION,
	deviceApi.destroySession,
);

export const getInventory = createAction(
	Action.GET_INVENTORY,
	async (fragmentType) => {
		const fragments = await deviceApi.getInventory(fragmentType);
		const assets = fragments.managedObjects[0].childDevices.references;

		return Promise.all(assets.map(asset => deviceApi.getDeviceData(asset.managedObject.id)));
	},
);

export const getDeviceData = createAction(
	Action.GET_DEVICE_DATA,
	deviceId => deviceApi.getDeviceData(deviceId),
	(deviceId, name) => ({ name }),
);

export const getDeviceMeasurements = createAction(
	Action.GET_DEVICE_MEASUREMENTS,
	(deviceId) => {
		const from = minutesAgo(MEASUREMENT_FROM_MINUTES_AGO);
		const to = new Date();

		return deviceApi.getDeviceMeasurements(deviceId, from.toISOString(), to.toISOString());
	},
	(deviceId, name) => ({ name }),
);

export const pollDeviceData = createAction(
	Action.POLL_CLIENT,
	deviceApi.startPolling,
);

export const subscribeDevice = createAction(
	Action.SUBSCRIBE_DEVICE,
	(clientId, deviceId, name, subscriptionType) => deviceApi.subscribe(clientId, deviceId, subscriptionType),
	(clientId, deviceId, name, subscriptionType) => ({ name, subscriptionType }),
);

export const unsubscribeDevice = createAction(
	Action.UNSUBSCRIBE_DEVICE,
	(clientId, deviceId, name, subscriptionType) => deviceApi.unsubscribe(clientId, deviceId, subscriptionType),
	(clientId, deviceId, name, subscriptionType) => ({ name, subscriptionType }),
);