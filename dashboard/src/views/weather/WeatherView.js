import React, { PropTypes } from 'react';
import withDevice from '../../services/connectDeviceService';
import Chart from '../../components/chart/Chart';
import './weather-view.scss';

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

			<Chart
				title="Sound level"
				data={weather.measurements.soundLevel}
				currentValue={Math.round(weather.data.soundLevel)}
				color={[0, 124, 153]}
				minutes={1}
				unit="db"
			/>

			<Chart
				title="Humidity"
				data={weather.measurements.humidity}
				currentValue={Math.round(weather.data.humidity)}
				color={[0, 153, 153]}
				minutes={10}
				unit="&#37;"
			/>

			<Chart
				title="Pressure"
				data={weather.measurements.pressure}
				currentValue={Math.round(weather.data.pressure)}
				color={[0, 255, 255]}
				minutes={10}
				unit="mmGH"
			/>

			<Chart
				title="Temperature"
				data={weather.measurements.temperature}
				currentValue={Math.round(weather.data.temperature * 10) / 10}
				color={[0, 255, 205]}
				minutes={10}
				unit="&#8451;"
			/>

		</div>

		<div className="header header--street">
			<img className="header__icon" src="../../gfx/icons/street-light.svg" alt="icon" />
			<h1 className="header__title">Street lighting</h1>
		</div>
		<div className="container">
			<Chart
				title="Detected light level"
				data={weather.measurements.lightLevel}
				currentValue={weather.data.lightLevel}
				color={[0, 255, 205]}
				minutes={10}
				unit="lux"
				size="large"
			/>

			<Chart
				title="Output power"
				data={lighting.measurements.power}
				currentValue={Math.round(lighting.data.outputPower * 10) / 10}
				color={[0, 255, 205]}
				minutes={10}
				unit="kW"
				size="large"
			/>
		</div>
	</div>
);

WeatherView.propTypes = {
	WEATHER_CONTROLLER: PropTypes.object,
	LIGHTING_CONTROLLER: PropTypes.object,
};

export default withDevice(['WEATHER_CONTROLLER', 'LIGHTING_CONTROLLER'])(WeatherView);
