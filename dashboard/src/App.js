import React from 'react';
import { Route } from 'react-router-dom';

import HomeView from './views/home/HomeView';

export default () => (
	<div className="app">
		<Route exact path="/" component={HomeView} />
	</div>
);
