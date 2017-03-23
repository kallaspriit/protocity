import React from 'react';
import './parking-view.scss';

const ParkingView = () => (
	<div className="parking-view">
		<div className="header header--parking">
			<img className="header__icon" src="../../gfx/icons/parking.svg" alt="icon" />
			<h1 className="header__title">Parking</h1>
		</div>

		<div className="parking">
			<div className="parking__title">
				<h1 className="parking__title-text">slot 1</h1>
				<h1 className="parking__title-text">slot 2</h1>
				<h1 className="parking__title-text">slot 3</h1>
			</div>
			<div className="parking__data">
				<div className="parking__data__availability">
					{/* IF PARKING SLOT IS FREE: */}
					<div className="parking__data__availability--free" />
					{/* IF PARKING SLOT IS OCCUPIED: */}
					{/*<div className="parking__data__availability--occupied">
						<p className="car-type" />
						<p className="cost">Parking cost: </p>
					</div>*/}
				</div>
				<div className="parking__data__availability">
					{/* IF PARKING SLOT IS FREE: */}
					<div className="parking__data__availability--free" />
					{/* IF PARKING SLOT IS OCCUPIED: */}
					{/*<div className="parking__data__availability--occupied">
						<p className="car-type" />
						<p className="cost">Parking cost: </p>
					</div>*/}
				</div>
				<div className="parking__data__availability">
					{/* IF PARKING SLOT IS FREE: */}
					{/*<div className="parking__data__availability--free" />*/}
					{/* IF PARKING SLOT IS OCCUPIED: */}
					<div className="parking__data__availability--occupied">
						<p className="car-type" />
						<p className="cost">Parking cost: </p>
					</div>
				</div>
			</div>
			<div className="parking__area">
				<div className="parking__area-slot parking__area-slot-1" />
				<div className="parking__area-slot parking__area-slot-2" />
				<div className="parking__area-slot parking__area-slot-3" />
			</div>
		</div>		
	</div>
);

export default ParkingView;