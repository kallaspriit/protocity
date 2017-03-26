import React, { PropTypes, Component } from 'react';
import withDevice from '../../services/connectDeviceService';
import { Device } from '../../common/gateway/gatewayConstants';
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
						<p className="cost">Total cost<br /> <strong>1.41€</strong></p>
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
						{this.getCarName(0)}
					</div>
					<div className="parking__data__availability">
						<p className="car-type">
							{this.getCarName(1)}
						</p>
					</div>
					<div className="parking__data__availability car-type">
						<p className="car-type">
							{this.getCarName(2)}
						</p>
					</div>
				</div>
			</div>
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
				<p className="cost">
					Parking fee<br />
					<strong>{Math.round(slot.cost * 100) / 100}€</strong>
				</p>
			);
		}

		/*
		return (
			<p className="cost">Total cost<br /> <strong>1.41€</strong></p>
		);
		*/

		return null;
	}

	getCarName = (slotNumber) => {
		if (!this.hasSlotInfo(slotNumber)) {
			return '';
		}

		const slot = this.props.PARKING_CONTROLLER.data.slots[slotNumber];

		return slot.occupantName.toLowerCase();
	}

	hasSlotInfo = slotNumber =>
		this.props.PARKING_CONTROLLER.data.slots
		&& this.props.PARKING_CONTROLLER.data.slots[slotNumber] !== undefined;
}

export default withDevice([Device.PARKING_CONTROLLER])(ParkingView);