import React from 'react';
import './truck-view.scss';

const TruckView = () => (
	<div className="truck-view">
		<div className="demo-video">
			<div
				className="demo-video__placeholder"
				style={{ backgroundImage: 'url(/videos/truck-dashboard.eng.png)' }}
			/>
		</div>

		<div className="header header--truck">
			<img className="header__icon" src="../../gfx/icons/truck.svg" alt="icon" />
			<h1 className="header__title">Electric delivery truck</h1>
		</div>

		<div className="container">
			<div className="data">
				<h2 className="data__title">Battery level</h2>
				<div className="data__chart">
					<p className="data__label--alert">Battery level under 15%, please charge!</p>
					<p className="data__label data__label--battery">Battery not charging</p>
					<p className="data__label data__label--battery-charging">Battery charging</p>
					<p className="data__label data__label--energy-use">Using 20 kW</p>
				</div>
			</div>

			<div className="data">
				<h2 className="data__title">Charging cost</h2>
				<div className="data__chart">
					<p className="data__value">0.15</p>
					<p className="data__measure">EUR</p>
				</div>
			</div>
		</div>

		<div className="header header--solar">
			<img className="header__icon" src="../../gfx/icons/solar-panel.svg" alt="icon" />
			<h1 className="header__title">Solar panel</h1>
		</div>
		<div className="container">
			<div className="data">
				<h2 className="data__title">Energy production</h2>
				<div className="data__chart">
					<p className="data__label data__label--overall">Overall energy produced: 220 kW</p>
					<p className="data__label data__label--energy">Producing 8 kW</p>
					<p className="data__label data__label--cost">Selling 3 kW</p>
					<p className="data__label data__label--energy">Using 8 kW</p>
					<p className="data__label data__label--cost">Buying 3 kW</p>
				</div>
			</div>

			<div className="data">
				<h2 className="data__title">Buying / selling from grid</h2>
				<div className="data__chart">
					<p className="data__value">15</p>
					<p className="data__measure">kW</p>
				</div>
			</div>
		</div>
	</div>
);

export default TruckView;