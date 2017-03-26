import React, { PropTypes } from 'react';
import withDevice from '../../services/connectDeviceService';
import { Device } from '../../common/gateway/gatewayConstants';
import Chart from '../../components/chart/Chart';
import './weather-view.scss';

export const WeatherView = ({ WEATHER_CONTROLLER: weather, LIGHTING_CONTROLLER: lighting }) => (
	<div className="weather-view">
		<div className="demo-video">
			<video src="/videos/weather-dashboard.eng.mp4" autoPlay loop />
		</div>
		<div className="container">
			<div className="header">
				<h1 className="header__title">Weather station</h1>
			</div>
			<div className="chart-container">
				<Chart
					title="Temperature"
					data={weather.measurements.temperature}
					currentValue={Math.round(weather.data.temperature * 10) / 10}
					color={[0, 205, 255]}
					minutes={1}
					unit="&#8451;"
				/>

				<Chart
					title="Humidity"
					data={weather.measurements.humidity}
					currentValue={Math.round(weather.data.humidity)}
					color={[255, 155, 0]}
					minutes={1}
					unit="&#37;"
				/>

				<Chart
					title="Pressure"
					data={weather.measurements.pressure}
					currentValue={Math.round(weather.data.pressure)}
					color={[153, 10, 227]}
					minutes={1}
					unit="mmGH"
				/>
				<Chart
					title="Sound level"
					data={weather.measurements.soundLevel}
					currentValue={Math.round(weather.data.soundLevel)}
					color={[225, 35, 100]}
					minutes={1}
					unit="db"
				/>

			</div>
			<div className="header header--street">
				<h1 className="header__title">Street lighting</h1>
			</div>

			<div className="chart-container">
				<Chart
					title="Detected light level"
					data={weather.measurements.lightLevel}
					currentValue={weather.data.lightLevel}
					color={[0, 153, 255]}
					minutes={1}
					unit="lux"
					size="large"
				/>

				<Chart
					title="Output power"
					data={lighting.measurements.power}
					currentValue={Math.round(lighting.data.outputPower * 10) / 10}
					color={[0, 204, 102]}
					minutes={1}
					unit="kW"
					size="large"
				/>
			</div>
		</div>
	</div>
);

WeatherView.propTypes = {
	WEATHER_CONTROLLER: PropTypes.object,
	LIGHTING_CONTROLLER: PropTypes.object,
};

export default withDevice([Device.WEATHER_CONTROLLER, Device.LIGHTING_CONTROLLER])(WeatherView);
