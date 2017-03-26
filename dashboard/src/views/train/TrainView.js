import React, { PropTypes } from 'react';
import { Device } from '../../common/gateway/gatewayConstants';
import withDevice from '../../services/connectDeviceService';
import Chart from '../../components/chart/Chart';
import './train-view.scss';

const steps = [
	'pie--step-1',
	'pie--step-1 pie--filling',
	'pie--step-2',
	'pie--step-2 pie--filling',
	'pie--step-3',
	'pie--step-3 pie--filling',
];

const getStationClassName = (stationName, activeStationName) => {
	let activeStation;

	switch (activeStationName) {
		case 'Central station':
			activeStation = 'central';
			break;

		case 'Police station':
			activeStation = 'police';
			break;

		case 'Beach station':
			activeStation = 'beach';
			break;

		// no default
	}

	const className = `route__station route__station--${stationName}`;

	return className;
};

const TrainView = ({ TRAIN_CONTROLLER: train }) => (
	<div className="train-view">
		<div className="demo-video">
			<div
				className="demo-video__placeholder"
				style={{ backgroundImage: 'url(/videos/train-dashboard.eng.png)' }}
			/>
		</div>

		<div className="container">
			<div className="header">
				<h1 className="header__title">Train</h1>
			</div>

			<div className="route-container">

				<div className="route">
					<div className="route__track" />
					<div className="route__track route__track__2" />
					<div className="route__track route__track__3" />

					<div className={getStationClassName('central')}>
						<span className="route__station__title">Central station</span>
					</div>
					<div className={getStationClassName('beach')}>
						<span className="route__station__title">Beach</span>
					</div>
					<div className={getStationClassName('police')}>
						<span className="route__station__title">Police</span>
					</div>

					<div className="route__direction route__direction__1" />
					<div className="route__direction route__direction__2" />
					<div className="route__direction route__direction__3" />
					<div className="route__direction route__direction__4" />
					<div className="route__direction route__direction__5" />

				</div>
			</div>

			<div className="chart-container">
				<Chart
					title="Battery level"
					data={train.measurements.chargePercentage}
					currentValue={train.data.batteryChargePercentage}
					color={[0, 204, 102]}
					minutes={1}
					unit="%"
					size="large"
					icon={train.data.isCharging ? 'charging' : 'not-charging'}
				/>
			</div>
		</div>
	</div>
);

TrainView.propTypes = {
	TRAIN_CONTROLLER: PropTypes.object,
};

export default withDevice([Device.TRAIN_CONTROLLER])(TrainView);