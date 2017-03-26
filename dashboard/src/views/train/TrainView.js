import React, { PropTypes } from 'react';
import { Device } from '../../common/gateway/gatewayConstants';
import withDevice from '../../services/connectDeviceService';
import Chart from '../../components/chart/Chart';
import './train-view.scss';

const renderStation = (stationName, previousStationName, nextStationName, isInStation) => {
	const stationClassName = stationName.split(' ')[0].toLowerCase();
	let className = `route__station route__station--${stationClassName}`;

	if (isInStation && stationName === nextStationName) {
		className += ' route__station--active';
	} else if (!isInStation && stationName === nextStationName) {
		className += ' route__station--next';
	}

	return (
		<div className={className}>
			<span className="route__station__title">{stationName.split(' ')[0]}</span>
		</div>
	);
};

const getInfo = (isObstacleDetected, realSpeed, stationName) => {
	if (isObstacleDetected) {
		return (
			<div className="route__obstacle" />
		);
	}

	if (realSpeed > 0) {
		return (
			<div className="route__msg"><span><strong>{realSpeed}</strong> km/h</span></div>
		);
	}

	return (
		<div className="route__msg route__msg--station">
			Stopped at {stationName}
		</div>
	);
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

					{renderStation(
						'Central station',
						train.data.previousStationName,
						train.data.nextStationName,
						train.data.isInStation,
					)}

					{renderStation(
						'Beach station',
						train.data.previousStationName,
						train.data.nextStationName,
						train.data.isInStation,
					)}

					{renderStation(
						'Police station',
						train.data.previousStationName,
						train.data.nextStationName,
						train.data.isInStation,
					)}

					<div className="route__direction route__direction__1" />
					<div className="route__direction route__direction__2" />
					<div className="route__direction route__direction__3" />
					<div className="route__direction route__direction__4" />
					<div className="route__direction route__direction__5" />

					{getInfo(train.data.isObstacleDetected, train.data.realSpeed, train.data.nextStationName)}
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
