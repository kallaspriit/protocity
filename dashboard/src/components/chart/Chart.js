import React, { PureComponent, PropTypes } from 'react';
import ReactHighCharts from 'react-highcharts';

const getXAxisMin = (minutes, minTime) => {
	const now = new Date().getTime() - (60000 * minutes);
	const past = new Date(now).getTime();

	return typeof minTime === 'number' ? Math.max(past, minTime) : past;
};

const getConfig = (data = [], color) => ({
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
			lineColor: `rgb(${color[0]}, ${color[1]}, ${color[2]}`,
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
			lineWidth: 5,
			marker: {
				enabled: false,
			},
			states: {
				hover: {
					enabled: false,
				},
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
});

class Chart extends PureComponent {

	static propTypes = {
		children: PropTypes.any,
		data: PropTypes.arrayOf(PropTypes.array),
		currentValue: PropTypes.number,
		unit: PropTypes.string,
		minutes: PropTypes.number, // eslint-disable-line
		color: PropTypes.arrayOf(PropTypes.number),
		title: PropTypes.string,
		className: PropTypes.string,
		icon: PropTypes.string,
		size: PropTypes.oneOf(['small', 'large']),
	};

	static defaultProps = {
		color: [142, 0, 219],
		minutes: 10,
		size: 'small',
		className: '',
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
					{Number.isNaN(this.props.currentValue) ? 0 : this.props.currentValue}
					<span className="data__measure">{this.props.unit}</span>
				</p>
			</div>

			<div className={`data__chart data__chart--${this.props.size}`}>
				<div className="data__chart__graph">
					<ReactHighCharts
						config={getConfig(this.props.data, this.props.color)}
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

		// set new data
		if (Array.isArray(props.data) && JSON.stringify(props.data) !== JSON.stringify(this.props.data)) {
			series.setData(props.data, true, true);
			chart.xAxis[0].setExtremes(
				getXAxisMin(props.minutes, props.data[0][0]), props.data[props.data.length - 1][0],
			);
		}

		// add new and remove old point
		if (props.currentValue !== this.props.currentValue && !Number.isNaN(props.currentValue)) {
			series.addPoint([new Date().getTime(), props.currentValue], true, false);
			chart.xAxis[0].setExtremes(getXAxisMin(props.minutes), new Date().getTime());
		}
	}

	chartRef = null;
}

export default Chart;