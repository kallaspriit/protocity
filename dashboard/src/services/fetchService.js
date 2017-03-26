import base64 from 'base-64';
import axios from 'axios';
import { getMockInfo, requestMock } from '../utils';

const hash = base64.encode(`${process.env.APP_API_USERNAME}:${process.env.APP_API_PASSWORD}`);

const request = (method, url, body) => axios({
	method,
	url,
	headers: {
		'Authorization': `Basic ${hash}`,
		'Content-Type': 'application/json',
	},
	data: body,
}).then(data => data.data);

const requestFetch = (method, url, body) => { // eslint-disable-line
	// return mock if exists
	const mock = getMockInfo(url);

	if (mock !== undefined) {
		return requestMock(method, url, mock, body);
	}

	// setup headers
	const headers = new Headers();
	headers.append('Content-Type', 'application/json');
	headers.append('Authorization', `Basic ${hash}`);

	const init = {
		headers,
		method,
	};

	if (body !== undefined) {
		init.body = JSON.stringify(body);
	}

	return fetch(url, init)
		.then((response) => {
			if (response.ok) {
				const contentType = response.headers.get('content-type');

				if (contentType && contentType.indexOf('json') !== -1) {
					return response.json();
				}

				return response.text();
			}

			throw new Error(response.statusText);
		})
		.catch(error => Promise.reject(error.message));
};

export const get = url => request('GET', `${process.env.APP_API_URL}${url}`);

export const post = (url, body) => request('POST', `${process.env.APP_API_URL}${url}`, body);

export const getLocal = url => request('GET', url);