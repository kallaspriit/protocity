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
		<div className="demo-video">
			<div
				className="demo-video__placeholder"
				style={{ backgroundImage: 'url(/videos/train-dashboard.eng.png)' }}
			/>
		</div>

		<div className="header header--train">
			<img className="header__icon" src="../../gfx/icons/train.svg" alt="icon" />
			<h1 className="header__title">Train</h1>
		</div>

		<div className="container__train-route">
			<div className={`pie ${steps[1]}`}>
				<div className="pie__label"><span>Beach</span></div>
				<div className="pie__label"><span>Police station</span></div>
				<div className="pie__label"><span>Central station</span></div>
				<div className="pie__chart" />
				<div className="pie__data">
					<p className="pie__title">Speed:</p>
					<p className="pie__value">2 km/h</p>
					{/* ALERT MESSAGES */}
					{/* <p className="pie__alert pie__alert--connection">Connection lost</p> */}
					{/* <p className="pie__alert pie__alert--obstacle">Obstacle ahead</p> */}
				</div>
			</div>
		</div>
		<div className="container__train-chart">
			<div className="data__chart data__chart--train">
				<p className="data__label data__label--battery-charging">Battery charging</p>
				<p className="data__label data__label--battery">Battery level: 78%</p>
			</div>
		</div>
	</div>
);

export default TrainView;