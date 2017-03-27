import React, { PropTypes, Component } from 'react';
import ReactInterval from 'react-interval';
import classnames from 'classnames';
import withDevice from '../../services/connectDeviceService';
import { Device } from '../../common/gateway/gatewayConstants';
import { formatCurrency } from '../../utils';
import './parking-view.scss';

class ParkingView extends Component {
	static propTypes = {
		PARKING_CONTROLLER: PropTypes.object,
	};

	render = () => (
		<div className="parking-view">
			<div className="demo-video">
				<div
					className="demo-video__placeholder"
					style={{ backgroundImage: 'url(/videos/parking-dashboard.eng.png)' }}
				/>
			</div>

			<div className="container">
				<div className="header">
					<h1 className="header__title">Parking</h1>
				</div>
				<div className="parking__data">
					<div className="parking__data__availability">
						{this.renderSlotPaymentInfo(0)}
					</div>
					<div className="parking__data__availability">
						{this.renderSlotPaymentInfo(1)}
					</div>
					<div className="parking__data__availability">
						{this.renderSlotPaymentInfo(2)}
					</div>
				</div>

				<div className="parking">
					<div className="parking__title">
						<h1 className="parking__title-text">slot 1</h1>
						<h1 className="parking__title-text">slot 2</h1>
						<h1 className="parking__title-text">slot 3</h1>
					</div>
					<div className="parking__area">
						<div className="parking__area__slot">
							<span className="parking__area__nr">1</span>
							{this.renderCar(0)}
						</div>
						<div className="parking__area__slot">
							{this.renderCar(1)}
							<span className="parking__area__nr">2</span>
						</div>
						<div className="parking__area__slot">
							{this.renderCar(2)}
							<span className="parking__area__nr">3</span>
						</div>
					</div>
				</div>

				<div className="parking__data">
					<div className="parking__data__availability">
						{this.renderCarName(0)}
					</div>
					<div className="parking__data__availability">
						<p className="car-type">
							{this.renderCarName(1)}
						</p>
					</div>
					<div className="parking__data__availability car-type">
						<p className="car-type">
							{this.renderCarName(2)}
						</p>
					</div>
				</div>
			</div>

			<ReactInterval timeout={1000} enabled callback={this.handleIntervalTick} />
		</div>
	);

	renderCar = (slotNumber) => {
		if (!this.hasSlotInfo(slotNumber)) {
			return null;
		}

		const slot = this.props.PARKING_CONTROLLER.data.slots[slotNumber];
		let className = 'parking__car';
		const style = {};

		if (slot.occupantName && slot.occupantName.length > 0) {
			className += ` parking__car--${slot.occupantName.split(' ').join('_')}`;
			style.backgroundImage = `url(/cars/${slot.occupantName.split(' ').join('_')}.png)`;
		}

		if (!slot.isOccupied) {
			className += ' parking__car--leaving';
		}

		return (
			<div className={className} style={style} />
		);
	}

	renderSlotPaymentInfo = (slotNumber) => {
		if (!this.hasSlotInfo(slotNumber)) {
			return null;
		}

		const slot = this.props.PARKING_CONTROLLER.data.slots[slotNumber];

		if (slot.isOccupied) {
			return (
				<p className="parking__cost">
					Parking fee<br />
					<strong>{formatCurrency(slot.cost)}</strong>
				</p>
			);
		}

		const className = classnames({
			'parking__cost': true,
			'parking__info--expired': this.isSlotInfoExpired(slot.freedTimestamp),
		});

		return (
			<p className={className}>
				Total cost<br />
				<strong>{formatCurrency(slot.cost)}</strong>
			</p>
		);
	}

	renderCarName = (slotNumber) => {
		if (!this.hasSlotInfo(slotNumber)) {
			return '';
		}

		const slot = this.props.PARKING_CONTROLLER.data.slots[slotNumber];

		const className = classnames({
			'parking__car-name': true,
			'parking__info--expired': !slot.isOccupied && this.isSlotInfoExpired(slot.freedTimestamp),
		});

		return (
			<span className={className}>{slot.occupantName.toLowerCase()}</span>
		);
	}

	handleIntervalTick = () => {
		// periodically force update to evaluate which information should be displayed
		this.forceUpdate();
	}

	// how long before information such as total cost and car name is removed
	isSlotInfoExpired = freedTimestamp => freedTimestamp < Date.now() - (3 * 1000);

	hasSlotInfo = slotNumber =>
		this.props.PARKING_CONTROLLER.data.slots
		&& this.props.PARKING_CONTROLLER.data.slots[slotNumber] !== undefined;
}

export default withDevice([Device.PARKING_CONTROLLER])(ParkingView);
