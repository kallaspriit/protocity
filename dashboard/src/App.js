import React, { Component, PropTypes } from 'react';
import { connect } from 'react-redux';
import { Route, withRouter } from 'react-router-dom';

import * as deviceActions from './common/device/deviceActions';

import HomeView from './views/home/HomeView';
import LoadingView from './views/loading/LoadingView';
import TrainView from './views/train/TrainView';
import TruckView from './views/truck/TruckView';
import WeatherView from './views/weather/WeatherView';
import ParkingView from './views/parking/ParkingView';

export class App extends Component {

	static propTypes = {
		clientId: PropTypes.string,
		isPolling: PropTypes.bool.isRequired, // eslint-disable-line
		isInventoryLoaded: PropTypes.bool.isRequired,
		devices: PropTypes.object.isRequired, // eslint-disable-line
		inventory: PropTypes.object.isRequired, // eslint-disable-line
		createSession: PropTypes.func.isRequired,
		destroySession: PropTypes.func.isRequired,
		getInventory: PropTypes.func.isRequired,
		getDeviceData: PropTypes.func.isRequired,
		pollDeviceData: PropTypes.func.isRequired,
		subscribeToDeviceData: PropTypes.func.isRequired,
	}

	componentDidMount = () => {
		this.props.createSession();
		this.props.getInventory('com_stagnationlab_c8y_driver_fragments_Gateway');
	}

	componentWillReceiveProps = (nextProps) => {
		if (!nextProps.isPolling && nextProps.clientId) {
			this.props.pollDeviceData(nextProps.clientId);
		}

		this.setupWeatherDevice(nextProps);
	}

	componentWillUnmount = () => {
		this.props.destroySession();
	}

	render = () => (
		<div className="app">
			<Route exact path="/" component={HomeView} />
			<Route exact path="/train" component={this.isAppReady() ? TrainView : LoadingView} />
			<Route exact path="/truck" component={this.isAppReady() ? TruckView : LoadingView} />
			<Route exact path="/weather" component={this.isAppReady() ? WeatherView : LoadingView} />
			<Route exact path="/parking" component={this.isAppReady() ? ParkingView : LoadingView} />
		</div>
	);

	isAppReady = () => this.props.clientId && this.props.isInventoryLoaded;

	setupWeatherDevice(props) {
		const deviceId = props.inventory.WEATHER_CONTROLLER;
		const device = props.devices.WEATHER_CONTROLLER;

		if (!deviceId) {
			return;
		}

		if (!device.hasDataSubscription && !device.isLoading) {
			this.props.subscribeToDeviceData(props.clientId, deviceId, 'WEATHER_CONTROLLER');
			this.props.getDeviceData(deviceId, 'WEATHER_CONTROLLER');
		}
	}
}

const mapStateToProps = ({ deviceReducer }) => ({
	clientId: deviceReducer.clientId,
	isPolling: deviceReducer.isPolling,
	isInventoryLoaded: deviceReducer.isInventoryLoaded,
	devices: deviceReducer.devices,
	inventory: deviceReducer.inventory,
});

const mapDispatchToProps = {
	...deviceActions,
};

export default withRouter(
	connect(mapStateToProps, mapDispatchToProps)(App),
);