import React from 'react';
import { Link } from 'react-router-dom';
import './home-view.scss';

const HomeView = () => (
	<div className="home-view">
		<Link to="train">train view</Link>
		<Link to="truck">truck view</Link>
		<Link to="weather">weather view</Link>
	</div>
);

// trainview
// truckview
// parkingview
// weatherview

export default HomeView;