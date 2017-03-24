import { get, post } from '../services/fetchService';

export const createSession = () => post('cep/realtime', [{
	version: '1.0',
	minimumVersion: '0.9',
	channel: '/meta/handshake',
	supportedConnectionTypes: ['long-polling'],
	advice: {
		timeout: 60000,
		interval: 0,
	},
}]);

export const destroySession = clientId => post('cep/realtime', [{
	channel: '/meta/disconnect',
	clientId,
}]);

export const getInventory = fragmentType => get(`inventory/managedObjects?fragmentType=${fragmentType}`);

export const subscribe = (clientId, deviceId, type) => post('cep/realtime', [{
	channel: '/meta/subscribe',
	subscription: `/${type}/${deviceId}`,
	clientId,
}]);


export const unsubscribe = (clientId, deviceId, type) => post('cep/realtime', [{
	channel: '/meta/unsubscribe',
	subscription: `/${type}/${deviceId}`,
	clientId,
}]);


export const startPolling = clientId => post('cep/realtime', [{
	channel: '/meta/connect',
	connectionType: 'long-polling',
	clientId,
}]);


export const getDeviceData = deviceId => get(`inventory/managedObjects/${deviceId}`);

export const getDeviceMeasurements = (deviceId, dateFrom, dateTo) => get(
	`measurement/measurements/series?source=${deviceId}&dateFrom=${dateFrom}&dateTo=${dateTo}`,
);