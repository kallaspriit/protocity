import React, { Component, PropTypes } from 'react';
import { connect } from 'react-redux';
import * as deviceActions from '../common/device/deviceActions';
import { SUBSCRIPTION_TYPE } from '../common/device/deviceConstants';


function connectToDevice(WrappedComponent) {
	return class getDevice extends Component {

		static propTypes = {
			clientId: PropTypes.string.isRequired,
			deviceNames: PropTypes.arrayOf(PropTypes.string).isRequired,
			devices: PropTypes.object.isRequired,
			deviceIds: PropTypes.object.isRequired,
			getDeviceData: PropTypes.func.isRequired,
			subscribeDevice: PropTypes.func.isRequired,
			unsubscribeDevice: PropTypes.func.isRequired,
		};

		componentDidMount = () => {
			this.props.deviceNames.forEach(this.setupDevice);
		}

		componentWillUnmount = () => {
			this.props.deviceNames.forEach(this.removeDevice);
		}

		render = () => (
			<WrappedComponent {...this.props.devices} />
		);

		setupDevice = (deviceName) => {
			const deviceId = this.props.deviceIds[deviceName];
			const device = this.props.devices[deviceName];

			if (!deviceId) {
				return;
			}

			this.props.getDeviceData(deviceId, deviceName);

			if (!device.hasDataSubscription) {
				this.props.subscribeDevice(this.props.clientId, deviceId, deviceName, SUBSCRIPTION_TYPE.DATA);
			}

			if (!device.hasMeasurementsSubscription) {
				this.props.subscribeDevice(this.props.clientId, deviceId, deviceName, SUBSCRIPTION_TYPE.MEASUREMENTS);
			}
		}

		removeDevice = (deviceName) => {
			const deviceId = this.props.deviceIds[deviceName];
			const device = this.props.devices[deviceName];

			if (!deviceId) {
				return;
			}

			if (device.hasDataSubscription) {
				this.props.unsubscribeDevice(this.props.clientId, deviceId, deviceName, SUBSCRIPTION_TYPE.DATA);
			}

			if (device.hasMeasurementsSubscription) {
				this.props.unsubscribeDevice(this.props.clientId, deviceId, deviceName, SUBSCRIPTION_TYPE.MEASUREMENTS);
			}
		}
	};
}

export default function withDevice(deviceNames) {
	const mapStateToProps = ({ deviceReducer }) => ({
		clientId: deviceReducer.clientId,
		isPolling: deviceReducer.isPolling,
		isInventoryLoaded: deviceReducer.isInventoryLoaded,
		devices: deviceReducer.devices,
		deviceIds: deviceReducer.inventory,
		deviceNames,
	});

	const mapDispatchToProps = {
		...deviceActions,
	};

	return WrappedComponent => connect(mapStateToProps, mapDispatchToProps)(connectToDevice(WrappedComponent));
}