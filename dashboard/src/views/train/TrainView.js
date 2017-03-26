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

			<div className="container__train-route">
				<div className={`pie ${steps[1]}`}>
					<div className="pie__label"><span>Beach</span></div>
					<div className="pie__label"><span>Police station</span></div>
					<div className="pie__label"><span>Central station</span></div>
					<div className="pie__chart" />
					<div className="pie__data">
						<p className="pie__title">Speed:</p>
						<p className="pie__value">2 km/h</p>
						{/* ALERT MESSAGES */}
						{/* <p className="pie__alert pie__alert--connection">Connection lost</p> */}
						{/* <p className="pie__alert pie__alert--obstacle">Obstacle ahead</p> */}
					</div>
				</div>
			</div>
			<div className="container__train-chart">
				<div className="header">
					<h1 className="header__title">
						Battery:
						{train.data.isCharging
							? (<span className="charging">charging</span>)
							: (<span className="charging">not charging</span>)
						}
					</h1>

					<Chart
						title="Battery level"
						data={train.measurements.chargePercentage}
						currentValue={train.data.batteryChargePercentage}
						color={[0, 153, 153]}
						minutes={1}
						unit="%"
						size="large"
						icon={train.data.isCharging && 'charging'}
					/>
				</div>
			</div>
		</div>
	</div>
);

TrainView.propTypes = {
	TRAIN_CONTROLLER: PropTypes.object,
};

export default withDevice([Device.TRAIN_CONTROLLER])(TrainView);