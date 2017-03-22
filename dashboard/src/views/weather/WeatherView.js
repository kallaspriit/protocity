import React from 'react';
import './weather-view.scss';

const WeatherView = () => (
	<div className="weather-view">
		<div className="header header--weather">
			<img className="header__icon" src="../../gfx/icons/weather.svg" alt="icon" />
			<h1 className="header__title">Weather station</h1>
		</div>
		<div className="container">

			<div className="data data--small">
				<h2 className="data__title">Sound level</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">40</p>
					<p className="data__measure">db</p>
				</div>
			</div>

			<div className="data data--small">
				<h2 className="data__title">Humidity</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">75</p>
					<p className="data__measure">&#37;</p>
				</div>
			</div>

			<div className="data data--small">
				<h2 className="data__title">Pressure</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">760</p>
					<p className="data__measure">mmGH</p>
				</div>
			</div>

			<div className="data data--small">
				<h2 className="data__title">Light level</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">430</p>
					<p className="data__measure">lux</p>
				</div>
			</div>

			<div className="data data--small">
				<h2 className="data__title">Temperature</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">21</p>
					<p className="data__measure">&#8451;</p>
				</div>
			</div>
		</div>

		<div className="header header--street">
			<img className="header__icon" src="../../gfx/icons/street-light.svg" alt="icon" />
			<h1 className="header__title">Street lighting</h1>
		</div>
		<div className="container">
			<div className="data data--large">
				<h2 className="data__title">Detect light level from weather station</h2>
				<div className="data__chart data__chart--large">
					<p className="data__value">430</p>
					<p className="data__measure">lux</p>
				</div>
			</div>

			<div className="data data--large">
				<h2 className="data__title">Light level / energy cost in kWh</h2>
				<div className="data__chart data__chart--large">
					<p className="data__value">10</p>
					<p className="data__measure">kWh</p>
				</div>
			</div>
		</div>
	</div>
);

export default WeatherView;