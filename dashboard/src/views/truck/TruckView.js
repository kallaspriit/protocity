import React, { PropTypes } from 'react';
import withDevice from '../../services/connectDeviceService';
import Chart from '../../components/chart/Chart';
import './truck-view.scss';

const TruckView = ({ TRUCK_CONTROLLER: truck, TRUCK_SOLAR_PANEL: solar }) => (
	<div className="truck-view">
		<div className="demo-video">
			<div
				className="demo-video__placeholder"
				style={{ backgroundImage: 'url(/videos/truck-dashboard.eng.png)' }}
			/>
		</div>

		<div className="container">
			<div className="header">
				<h1 className="header__title">Electric delivery truck</h1>
			</div>

			<div className="chart-container">
				<Chart
					title={truck.data.batteryChargePercentage <= 15 ? 'Battery level low' : 'Battery level'}
					data={truck.measurements.chargePercentage || [[0, truck.data.batteryChargePercentage]]}
					currentValue={truck.data.batteryChargePercentage}
					color={[0, 153, 153]}
					minutes={1}
					unit="%"
					size="large"
					className={truck.data.batteryChargePercentage <= 15 ? 'data__warning' : ''}
					icon="not-charging"
				>
				</Chart>
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
					currentValue={solar.data.value}
					color={[153, 10, 227]}
					unit="kW"
					minutes={1}
					size="large"
				>
				</Chart>

				<Chart
					title="Buying / selling from grid"
					data={truck.measurements.gridPowerBalance}
					currentValue={truck.data.batteryVoltage}
					color={[225, 35, 100]}
					unit="kW"
					minutes={1}
					size="large"
				>
				</Chart>
			</div>
		</div>
	</div>
);

TruckView.propTypes = {
	TRUCK_CONTROLLER: PropTypes.object,
	TRUCK_SOLAR_PANEL: PropTypes.object,
};

export default withDevice(['TRUCK_CONTROLLER', 'TRUCK_SOLAR_PANEL'])(TruckView);