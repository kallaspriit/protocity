import React from 'react';
import { Link } from 'react-router-dom';
import './home-view.scss';

const HomeView = () => (
	<div className="home-view">
		<Link to="weather">Weather view</Link>
		<Link to="truck">Truck view</Link>
		<Link to="train">Train view</Link>
		<Link to="parking">Parking view</Link>
	</div>
);

export default HomeView;