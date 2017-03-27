import React, { Component, PropTypes } from 'react';
import { connect } from 'react-redux';
import * as gatewayActions from '../common/gateway/gatewayActions';
import { SubscriptionType } from '../common/gateway/gatewayConstants';


function connectToDevice(WrappedComponent) {
	return class getDevice extends Component {

		static propTypes = {
			clientId: PropTypes.string.isRequired,
			deviceNames: PropTypes.arrayOf(PropTypes.string).isRequired,
			devices: PropTypes.object.isRequired,
			deviceIds: PropTypes.object.isRequired,
			getDeviceData: PropTypes.func.isRequired,
			getDeviceMeasurements: PropTypes.func.isRequired,
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
				throw new Error(`Device "${deviceName}" does not exist in reducer`);
			}

			this.props.getDeviceData(deviceId, deviceName);
			this.props.getDeviceMeasurements(deviceId, deviceName);

			if (!device.hasDataSubscription) {
				this.props.subscribeDevice(this.props.clientId, deviceId, deviceName, SubscriptionType.DATA);
			}

			if (!device.hasMeasurementsSubscription) {
				this.props.subscribeDevice(this.props.clientId, deviceId, deviceName, SubscriptionType.MEASUREMENTS);
			}
		}

		removeDevice = (deviceName) => {
			const deviceId = this.props.deviceIds[deviceName];
			const device = this.props.devices[deviceName];

			if (!deviceId) {
				throw new Error(`Device "${deviceName}" does not exist in reducer`);
			}

			if (device.hasDataSubscription) {
				this.props.unsubscribeDevice(this.props.clientId, deviceId, deviceName, SubscriptionType.DATA);
			}

			if (device.hasMeasurementsSubscription) {
				this.props.unsubscribeDevice(this.props.clientId, deviceId, deviceName, SubscriptionType.MEASUREMENTS);
			}
		}
	};
}

export default function withDevice(deviceNames) {
	const mapStateToProps = ({ gatewayReducer }) => ({
		clientId: gatewayReducer.clientId,
		isPolling: gatewayReducer.isPolling,
		isInventoryLoaded: gatewayReducer.isInventoryLoaded,
		devices: gatewayReducer.devices,
		deviceIds: gatewayReducer.deviceIds,
		deviceNames,
	});

	const mapDispatchToProps = {
		...gatewayActions,
	};

	return WrappedComponent => connect(mapStateToProps, mapDispatchToProps)(connectToDevice(WrappedComponent));
}