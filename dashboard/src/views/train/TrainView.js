import React from 'react';
import './train-view.scss';

const steps = [
	'pie--step-1',
	'pie--step-1 pie--filling',
	'pie--step-2',
	'pie--step-2 pie--filling',
	'pie--step-3',
	'pie--step-3 pie--filling',
];

const TrainView = () => (
	<div className="train-view">
		<div className="header header--truck">
			<img className="header__icon" src="../../gfx/icons/weather.svg" alt="icon" />
			<h1 className="header__title">Train</h1>
		</div>
		<div className="content--train">
			<div className={`pie ${steps[3]}`}>
				<div className="pie__label"><span>Beach</span></div>
				<div className="pie__label"><span>Police station</span></div>
				<div className="pie__label"><span>Central station</span></div>
				<div className="pie__chart" />
				<div className="pie__title">speed</div>
			</div>
		</div>
		
		<div className="data data--large data__train">
			{/*<h2 className="data__title">Energy production</h2>*/}
			<div className="data__chart data__chart--large">
				<p className="data__message data__message--energy">Using 8 kW from solar panel</p>
				{/*<p className="data__message data__message--cost">Buying 3 kW for 0.11 EUR</p>
				<p className="data__message data__message--battery-charging">Battery charging</p>
				<p className="data__message data__message--energy-use">Buying 3 kW</p>
				<p className="data__message data__message--battery">Battery level: 78%</p>*/}
			</div>
		</div>
	</div>
);

export default TrainView;