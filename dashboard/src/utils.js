import { getLocal } from './services/fetchService';
import mockedUrls from '../config/mockedUrls';

const parseMockResponse = async (data, body) => {
	if (typeof data === 'function') {
		return data(body);
	}

	if (typeof data === 'string' && data.indexOf('.json') !== -1) {
		return getLocal(data);
	}

	return data;
};

export const getMockInfo = url => mockedUrls.find(item => item.regex.test(url));

export const requestMock = async (method, url, mock, body = '') => {
	/* eslint-disable no-console */
	const returnValue = await parseMockResponse(mock.response, body);
	const timeout = typeof mock.timeout === 'number' ? mock.timeout : 100;
	const mockError = typeof mock.error === 'function' ? mock.error(body) : mock.error;

	return new Promise((resolve, reject) => setTimeout(() => {
		console.groupCollapsed('mock', method, url);
		console.info('loading...', body);

		if (!mockError) {
			console.info('resolved:', returnValue);
		} else {
			console.info('rejected by mock params', mockError);
		}

		console.groupEnd();

		return !mockError ? resolve(returnValue) : reject(mockError);
	}, timeout));

	/* eslint-enable no-console */
};

export const formatCurrency = (number, unit = '€') => `${number.toFixed(2)} ${unit}`;

export const minutesAgo = (minutes = 15) => {
	const now = new Date();
	return new Date(now.getTime() - (60000 * minutes));
};