import React from 'react';
import withDevice from '../../services/connectDeviceService';
import { Device } from '../../common/gateway/gatewayConstants';
import './parking-view.scss';

const ParkingView = () => (
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
					{/* IF PARKING SLOT IS FREE: */}
					<div className="parking__data__availability--free" />
				</div>
				<div className="parking__data__availability">
					{/* IF PARKING SLOT IS FREE: */}
					<div className="parking__data__availability--occupied">
						<p className="cost">Total cost<br /> <strong>1.41€</strong></p>
					</div>
				</div>
				<div className="parking__data__availability">
					{/* IF PARKING SLOT IS FREE: */}
					{/* <div className="parking__data__availability--free" /> */}
					{/* IF PARKING SLOT IS OCCUPIED: */}
					<div className="parking__data__availability--occupied">
						<p className="cost">Parking fee<br /> <strong>0.01€</strong></p>
					</div>
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
					</div>
					<div className="parking__area__slot">
						<div className="parking__car parking__car--cart parking__car--leaving" />
						<span className="parking__area__nr">2</span>
					</div>
					<div className="parking__area__slot">
						<div className="parking__car parking__car--f1" />
						<span className="parking__area__nr">3</span>
					</div>
				</div>
			</div>

			<div className="parking__data">
				<div className="parking__data__availability" />
				<div className="parking__data__availability">
					<p className="car-type">Black car</p>
				</div>
				<div className="parking__data__availability car-type">
					<p className="car-type">Black car</p>
				</div>
			</div>
		</div>
	</div>
);

export default withDevice([Device.PARKING_CONTROLLER])(ParkingView);