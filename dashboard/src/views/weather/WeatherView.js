import React, { PropTypes } from 'react';
import './weather-view.scss';
import withDevice from '../../services/connectDeviceService';

export const WeatherView = ({ WEATHER_CONTROLLER: weather, LIGHTING_CONTROLLER: lighting }) => (
	<div className="weather-view">
		<div className="demo-video">
			<video src="/videos/weather-dashboard.eng.mp4" autoPlay loop />
		</div>
		<div className="header header--weather">
			<img className="header__icon" src="../../gfx/icons/weather.svg" alt="icon" />
			<h1 className="header__title">Weather station</h1>
		</div>
		<div className="container">
			<div className="data data--small">
				<h2 className="data__title">Sound level</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">{Math.round(weather.data.soundLevel)}</p>
					<p className="data__measure">db</p>
				</div>
			</div>

			<div className="data data--small">
				<h2 className="data__title">Humidity</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">{Math.round(weather.data.humidity)}</p>
					<p className="data__measure">&#37;</p>
				</div>
			</div>

			<div className="data data--small">
				<h2 className="data__title">Pressure</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">{Math.round(weather.data.pressure)}</p>
					<p className="data__measure">mmGH</p>
				</div>
			</div>

			<div className="data data--small">
				<h2 className="data__title">Light level</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">{Math.round(weather.data.lightLevel)}</p>
					<p className="data__measure">lux</p>
				</div>
			</div>

			<div className="data data--small">
				<h2 className="data__title">Temperature</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">{Math.round(weather.data.temperature)}</p>
					<p className="data__measure">&#8451;</p>
				</div>
			</div>
		</div>

		<div className="header header--street">
			<img className="header__icon" src="../../gfx/icons/street-light.svg" alt="icon" />
			<h1 className="header__title">Street lighting</h1>
		</div>
		<div className="container">
			<div className="data">
				<h2 className="data__title">Detect light level from weather station</h2>
				<div className="data__chart data__chart--large">
					<p className="data__value">{weather.data.lightLevel}</p>
					<p className="data__measure">lux</p>
				</div>
			</div>

			<div className="data">
				<h2 className="data__title">Light level / energy cost in kWh</h2>
				<div className="data__chart data__chart--large">
					<p className="data__value">{(Math.round(62 * lighting.data.outputLightLevel * 10) / 10)}</p>
					<p className="data__measure">kW</p>
				</div>
			</div>
		</div>
	</div>
);

WeatherView.propTypes = {
	WEATHER_CONTROLLER: PropTypes.object,
	LIGHTING_CONTROLLER: PropTypes.object,
};

export default withDevice(['WEATHER_CONTROLLER', 'LIGHTING_CONTROLLER'])(WeatherView);
