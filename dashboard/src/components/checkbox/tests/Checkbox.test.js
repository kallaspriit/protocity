import React from 'react';
import { mount } from 'enzyme';
import toJson from 'enzyme-to-json';
import Checkbox from '../Checkbox';

const onChangeMock = jest.fn();

const props = {
	className: 'test-class',
	name: 'Checkbox',
	checked: false,
	onChange: onChangeMock,
	disabled: false,
};

const component = mount(
	<Checkbox {...props}>
		<div>children</div>
	</Checkbox>,
);

describe('Checkbox', () => {
	it('have correct props', () => {
		component.setProps({
			className: 'new-test-class',
			name: 'New Checkbox',
			checked: !props.checked,
			disabled: !props.disabled,
		});

		expect(toJson(component)).toMatchSnapshot();
	});
});