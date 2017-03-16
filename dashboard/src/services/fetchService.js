import { getMockInfo, requestMock } from '../utils';

const request = (method, url, body) => {
	// return mock if exists
	const mock = getMockInfo(url);

	if (mock !== undefined) {
		return requestMock(method, url, mock, body);
	}

	// setup headers and make request to server
	const headers = new Headers();
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

				if (contentType && contentType.indexOf('application/json') !== -1) {
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