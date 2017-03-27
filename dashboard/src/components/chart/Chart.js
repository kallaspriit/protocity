import React, { PureComponent, PropTypes } from 'react';
import ReactHighCharts from 'react-highcharts';
import roundTo from 'round-to';
import { minutesAgo } from '../../utils';

class Chart extends PureComponent {

	static propTypes = {
		children: PropTypes.any,
		data: PropTypes.arrayOf(PropTypes.array),
		currentValue: PropTypes.number,
		unit: PropTypes.string,
		minutes: PropTypes.number, // eslint-disable-line
		color: PropTypes.arrayOf(PropTypes.number),
		negativeColor: PropTypes.arrayOf(PropTypes.number),
		title: PropTypes.string,
		className: PropTypes.string,
		icon: PropTypes.string,
		size: PropTypes.oneOf(['small', 'large']),
		options: PropTypes.object,
	};

	static defaultProps = {
		color: [142, 0, 219],
		negativeColor: [255, 35, 100],
		minutes: 10,
		size: 'small',
		className: '',
		options: {},
	};

	componentDidMount = () => {
		this.updateChart();
	}

	componentWillUpdate = (nextProps) => {
		this.updateChart(nextProps);
	}

	render = () => (
		<div className={`data data--${this.props.size} ${this.props.className}`}>
			<h2 className="data__title">{this.props.title}</h2>


			<div className="data__chart__content">
				<p className="data__value">
					{typeof this.props.currentValue !== 'number'
						? this.props.currentValue
						: roundTo(this.props.currentValue, 2)
					}
					<span className="data__measure">{this.props.unit}</span>
				</p>
			</div>

			<div className={`data__chart data__chart--${this.props.size}`}>
				<div className="data__chart__graph">
					<ReactHighCharts
						config={{ ...this.getConfig(), ...this.props.options }}
						domProps={{ style: { height: '100%' } }}
						ref={(ref) => { this.chartRef = ref; }}
						neverReflow
					/>
				</div>

				{this.props.icon && (
					<div className={`data__chart__icon data__chart__icon--${this.props.icon}`} />
				)}
			</div>

			{this.props.children && (
				<div className="data__chart__content">
					{this.props.children}
				</div>
			)}
		</div>
	);

	updateChart = (props = this.props) => {
		const chart = this.chartRef.getChart();
		const series = chart.series[0];

		if (props.data) {
			const itemsToAdd = props.data.filter(item => item[0] > this.lastPointDateTime);

			if (itemsToAdd.length === 0) {
				return;
			}

			const lastItemToAdd = itemsToAdd[itemsToAdd.length - 1];
			const hasPrevData = series.data.length > 0;

			if (hasPrevData) {
				itemsToAdd.forEach((item) => {
					series.addPoint(item, true, false);
				});
			} else {
				series.setData(itemsToAdd, true, false);
			}

			chart.xAxis[0].setExtremes(minutesAgo(props.minutes).getTime(), lastItemToAdd[0]);
			this.lastPointDateTime = lastItemToAdd[0];
		}
	}

	getConfig = () => {
		const {
			data,
			color,
			negativeColor,
		} = this.props;

		return {
			series: [{
				type: 'area',
				data,
			}],
			chart: {
				margin: 0,
				// animation: false,
			},
			plotOptions: {
				area: {
					lineWidth: 5,
					marker: {
						enabled: false,
					},
					states: {
						hover: {
							enabled: false,
						},
					},
					color: `rgb(${color[0]}, ${color[1]}, ${color[2]}`,
					fillColor: {
						linearGradient: {
							x1: 0,
							y1: 0,
							x2: 0,
							y2: 1,
						},
						stops: [
							[0, `rgba(${color[0]}, ${color[1]}, ${color[2]}, 0.6)`],
							[1, `rgba(${color[0]}, ${color[1]}, ${color[2]}, 0.1)`],
						],
					},
					negativeColor: `rgb(${negativeColor[0]}, ${negativeColor[1]}, ${negativeColor[2]}`,
					negativeFillColor: {
						linearGradient: {
							x1: 0,
							y1: 1,
							x2: 0,
							y2: 0,
						},
						stops: [
							[0, `rgba(${negativeColor[0]}, ${negativeColor[1]}, ${negativeColor[2]}, 0.6)`],
							[1, `rgba(${negativeColor[0]}, ${negativeColor[1]}, ${negativeColor[2]}, 0.1)`],
						],
					},
				},
			},
			xAxis: [{
				type: 'datetime',
				visible: false,
				minPadding: 0,
				maxPadding: 0,
				endOnTick: false,
			}],
			yAxis: [{
				labels: {
					enabled: false,
				},
				title: {
					text: null,
				},
			}],
			tooltip: {
				// enabled: false,
			},
			title: {
				text: null,
			},
			legend: {
				enabled: false,
			},
			credits: {
				enabled: false,
			},
		};
	}

	chartRef = null;
	lastPointDateTime = 0;
}

export default Chart;
