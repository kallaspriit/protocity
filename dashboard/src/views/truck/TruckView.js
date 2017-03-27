import React, { PropTypes } from 'react';
import withDevice from '../../services/connectDeviceService';
import { Device } from '../../common/gateway/gatewayConstants';
import Chart from '../../components/chart/Chart';
import VideoCarousel from '../../components/video-carousel/VideoCarousel';
import './truck-view.scss';

function getBatteryLevelOptions() {
	return {
		yAxis: {
			min: 0,
			max: 100,
		},
	};
}

function getPowerBalanceOptions() {
	return {
		yAxis: {
			tickPositioner: function() { // eslint-disable-line
				const maxDeviation = Math.ceil(Math.max(Math.abs(this.dataMax), Math.abs(this.dataMin)));
				const halfMaxDeviation = Math.ceil(maxDeviation / 2);

				return [-maxDeviation, -halfMaxDeviation, 0, halfMaxDeviation, maxDeviation];
			},
		},
	};
}

const TruckView = ({ TRUCK_CONTROLLER: truck, TRUCK_SOLAR_PANEL: solar, TRUCK_MOTION_SENSOR: motion }) => (
	<div className="truck-view">
		<VideoCarousel
			standbyUrl="/videos/general.eng.mp4"
			tutorialUrl="/videos/truck-introduction.eng.mp4"
			asideUrl="/videos/truck-dashboard.eng.mp4"
			lastMotionTime={motion.lastMotionDetectedTime}
			lastActivatedTime={truck.lastActivatedTime}
		/>

		<div className="container">
			<div className="header">
				<h1 className="header__title">Electric delivery truck</h1>
			</div>

			<div className="chart-container">
				<Chart
					title={truck.data.batteryChargePercentage <= 15 ? 'Battery level low' : 'Battery level'}
					data={truck.measurements.chargePercentage}
					currentValue={truck.data.batteryChargePercentage}
					color={[0, 153, 153]}
					minutes={1}
					unit="%"
					size="large"
					className={truck.data.batteryChargePercentage <= 15 ? 'data__warning' : ''}
					icon={truck.data.isCharging ? 'charging' : 'not-charging'}
					options={getBatteryLevelOptions()}
				/>
				<Chart
					title="Charging power"
					data={truck.measurements.chargePower}
					currentValue={truck.data.chargePower}
					color={[255, 155, 0]}
					minutes={1}
					unit="kW"
					size="large"
				/>
			</div>

			<div className="header">
				<h1 className="header__title">Solar panel</h1>
			</div>
			<div className="chart-container">
				<Chart
					title="Energy production"
					data={solar.measurements.value}
					currentValue={truck.data.solarOutputPower}
					color={[153, 10, 227]}
					unit="kW"
					minutes={1}
					size="large"
				/>

				<Chart
					title={truck.data.gridPowerBalance > 0 ? 'Selling to the grid' : 'Buying from the grid'}
					data={truck.measurements.gridPowerBalance ? truck.measurements.gridPowerBalance : [[0, 0]]}
					currentValue={truck.data.gridPowerBalance}
					color={[0, 204, 102]}
					negativeColor={[225, 35, 100]}
					unit="kW"
					minutes={1}
					size="large"
					options={getPowerBalanceOptions()}
				/>
			</div>
		</div>
	</div>
);

TruckView.propTypes = {
	TRUCK_CONTROLLER: PropTypes.object,
	TRUCK_SOLAR_PANEL: PropTypes.object,
	TRUCK_MOTION_SENSOR: PropTypes.object,
};

export default withDevice([Device.TRUCK_CONTROLLER, Device.TRUCK_SOLAR_PANEL, Device.TRUCK_MOTION_SENSOR])(TruckView);
