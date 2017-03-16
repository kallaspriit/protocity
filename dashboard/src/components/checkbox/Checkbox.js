import React, { PropTypes } from 'react';
import classNames from 'classnames';
// import './checkbox.scss';

function Checkbox({ className, onChange, checked, defaultChecked, name, disabled, children }) {
	const wrapClassName = classNames('checkbox', className);

	return (
		<label className={wrapClassName}>
			<input
				className="checkbox__input"
				type="checkbox"
				name={name}
				onChange={onChange}
				checked={checked}
				defaultChecked={defaultChecked}
				disabled={disabled}
			/>
			{children}
		</label>
	);
}

Checkbox.propTypes = {
	children: PropTypes.node,
	className: PropTypes.string,
	name: PropTypes.string,
	checked: PropTypes.bool,
	defaultChecked: PropTypes.bool,
	onChange: PropTypes.func,
	disabled: PropTypes.bool,
};

Checkbox.defaultProps = {
	disabled: false,
};

export default Checkbox;