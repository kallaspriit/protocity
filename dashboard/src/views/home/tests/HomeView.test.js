import React from 'react';
import { mount } from 'enzyme';
import toJson from 'enzyme-to-json';
import { HomeView } from '../HomeView';

const loadApiMock = jest.fn();
const incrementMock = jest.fn();
const decrementMock = jest.fn();

const props = {
	value: 1,
	isLoading: false,
	apiData: undefined,
	apiError: null,
	loadApi: loadApiMock,
	increment: incrementMock,
	decrement: decrementMock,
};

const wrapper = mount(
	<HomeView {...props} />,
);

describe('HomeView', () => {
	it('renders correctly', () => {
		expect(toJson(wrapper)).toMatchSnapshot();
	});

	it('have correct props', () => {
		wrapper.setProps({
			value: 2,
		});

		expect(toJson(wrapper)).toMatchSnapshot();
	});

	it('increment to be called', () => {
		wrapper.find('.btn-increment').simulate('click');
		expect(toJson(wrapper)).toMatchSnapshot();
	});

	it('api to be called', () => {
		wrapper.find('.btn-load-data').first().simulate('click');
		expect(toJson(wrapper)).toMatchSnapshot();
	});

	it('have correct loading state', () => {
		wrapper.setProps({
			isLoading: true,
			apiData: undefined,
			apiError: null,
		});

		expect(toJson(wrapper)).toMatchSnapshot();
	});

	it('have correct success state', () => {
		wrapper.setProps({
			isLoading: false,
			apiData: 'Test api success',
			apiError: null,
		});

		expect(toJson(wrapper)).toMatchSnapshot();
	});

	it('have correct error state', () => {
		wrapper.setProps({
			isLoading: false,
			apiData: undefined,
			apiError: 'Api error',
		});

		expect(toJson(wrapper)).toMatchSnapshot();
	});
});