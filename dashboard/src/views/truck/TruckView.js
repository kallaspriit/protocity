import React from 'react';
import './truck-view.scss';

const TruckView = () => (
	<div className="truck-view">
		<div className="header header__bg--weather">
			<img className="header__icon" src="../../gfx/icons/weather.svg" alt="icon" />
			<h1 className="header__title">Electric delivery truck</h1>
		</div>
		
		<div className="content">
			<div className="data data--large">
				<h2 className="data__title">Battery level</h2>
				<div className="data__chart data__chart--large">
					<p className="data__value">430</p>
					<p className="data__measure">lux</p>
				</div>
			</div>

			<div className="data data--large">
				<h2 className="data__title">Charging cost</h2>
				<div className="data__chart data__chart--large">
					<p className="data__value">10</p>
					<p className="data__measure">kWh</p>
				</div>
			</div>
		</div>

		<div className="header">
			<img className="header__icon" src="../../gfx/icons/weather.svg" alt="icon" />
			<h1 className="header__title">Solar panel</h1>
		</div>
		<div className="content">
			<div className="data data--large">
				<h2 className="data__title">Energy production</h2>
				<div className="data__chart data__chart--large">
					<p className="data__value">430</p>
					<p className="data__measure">lux</p>
				</div>
			</div>

			<div className="data data--large">
				<h2 className="data__title">Buying / selling from grid</h2>
				<div className="data__chart data__chart--large">
					<p className="data__value">15</p>
					<p className="data__measure">kW</p>
				</div>
			</div>
		</div>
	</div>
);

export default TruckView;