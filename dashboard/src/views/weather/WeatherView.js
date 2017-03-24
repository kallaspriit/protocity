import React, { PropTypes } from 'react';
import { connect } from 'react-redux';
import './weather-view.scss';

export const WeatherView = ({ weather, lighting }) => (
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
					<p className="data__value">{Math.round(weather.soundLevel)}</p>
					<p className="data__measure">db</p>
				</div>
			</div>

			<div className="data data--small">
				<h2 className="data__title">Humidity</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">{Math.round(weather.humidity)}</p>
					<p className="data__measure">&#37;</p>
				</div>
			</div>

			<div className="data data--small">
				<h2 className="data__title">Pressure</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">{Math.round(weather.pressure)}</p>
					<p className="data__measure">mmGH</p>
				</div>
			</div>

			<div className="data data--small">
				<h2 className="data__title">Light level</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">{Math.round(weather.lightLevel)}</p>
					<p className="data__measure">lux</p>
				</div>
			</div>

			<div className="data data--small">
				<h2 className="data__title">Temperature</h2>
				<div className="data__chart data__chart--small">
					<p className="data__value">{Math.round(weather.temperature)}</p>
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
					<p className="data__value">{lighting.detectedLightLevel}</p>
					<p className="data__measure">lux</p>
				</div>
			</div>

			<div className="data">
				<h2 className="data__title">Light level / energy cost in kWh</h2>
				<div className="data__chart data__chart--large">
					<p className="data__value">{lighting.outputLightLevel}</p>
					<p className="data__measure">kWh</p>
				</div>
			</div>
		</div>
	</div>
);

WeatherView.propTypes = {
	weather: PropTypes.shape({
		humidity: PropTypes.number.isRequired,
		lightLevel: PropTypes.number.isRequired,
		pressure: PropTypes.number.isRequired,
		soundLevel: PropTypes.number.isRequired,
		temperature: PropTypes.number.isRequired,
	}),
	lighting: PropTypes.shape({
		outputLightLevel: PropTypes.number.isRequired,
		detectedLightLevel: PropTypes.number.isRequired,
	}),
};

const mapStateToProps = ({ deviceReducer }) => ({
	weather: deviceReducer.devices.WEATHER_CONTROLLER.data,
	lighting: deviceReducer.devices.LIGHTING_CONTROLLER.data,
});

export default connect(mapStateToProps)(WeatherView);