import React from 'react';
import './parking-view.scss';

const ParkingView = () => (
	<div className="parking-view">
		<div className="header header--parking">
			<img className="header__icon" src="../../gfx/icons/parking.svg" alt="icon" />
			<h1 className="header__title">Parking</h1>
		</div>
		
		<div className="content__parking-data">
			<div className="parking-slot__data">
				<p className="car-type">Taxi car</p>
				<p>has left the parking slot</p>
				<p>Parking cost:</p>
				<p className="parking-cost">1.35</p>
			</div>
			
			<div className="parking-slot__data">
				<p>Parking slot no. 2:</p>
				<p>FREE</p>
			</div>
			
			<div className="parking-slot__data">
				<p>Parking slot no. 3 occupied by:</p>
				<p className="car-type">Red roadster</p>
				<p>Parking cost:</p>
				<p className="parking-cost">0.15</p>
			</div>
		</div>

		<div className="parking">
			<div className="parking-slot parking-slot-1" />
			<div className="parking-slot parking-slot-2" />
			<div className="parking-slot parking-slot-3" />
		</div>		
	</div>
);

export default ParkingView;