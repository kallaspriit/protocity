import React from 'react';
import { Route } from 'react-router-dom';

import HomeView from './views/home/HomeView';
import TrainView from './views/train/TrainView';
import TruckView from './views/truck/TruckView';
import WeatherView from './views/weather/WeatherView';

export default () => (
	<div className="app">
		<Route exact path="/" component={HomeView} />
		<Route exact path="/train" component={TrainView} />
		<Route exact path="/truck" component={TruckView} />
		<Route exact path="/weather" component={WeatherView} />
	</div>
);
