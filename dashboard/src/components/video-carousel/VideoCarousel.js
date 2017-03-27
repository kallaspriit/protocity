import React, { Component, PropTypes } from 'react';
import './video-carousel.scss';

const VideoType = {
	STANDBY: 'standby',
	TUTORIAL: 'tutorial',
	ASIDE: 'aside',
};

const TimeFromTutorialToStandbyMs = 1000 * 10;
const TimeFromActiveToStandbyMs = 1000 * 20;

export default class VideoCarousel extends Component {
	static propTypes = {
		lastMotionTime: PropTypes.number,
		lastActivatedTime: PropTypes.number,
		standbyUrl: PropTypes.string.isRequired, // eslint-disable-line
		tutorialUrl: PropTypes.string.isRequired, // eslint-disable-line
		asideUrl: PropTypes.string.isRequired, // eslint-disable-line
	};

	state = {
		activeVideoType: VideoType.STANDBY,
	};

	componentDidMount() {
		/* window.setInterval(() => {
			let nextVideoType;

			switch (this.state.activeVideoType) {
				case VideoType.STANDBY:
					nextVideoType = VideoType.TUTORIAL;
					break;

				case VideoType.TUTORIAL:
					nextVideoType = VideoType.ASIDE;
					break;

				default:
					nextVideoType = VideoType.STANDBY;
					break;
			}

			this.setState({
				activeVideoType: nextVideoType,
			});
		}, 5000); */
	}

	componentWillReceiveProps = (nextProps) => {
		if (this.props.lastActivatedTime !== nextProps.lastActivatedTime) {
			this.startStandbyTimer(TimeFromActiveToStandbyMs);

			if (this.state.activeVideoType !== VideoType.ASIDE) {
				this.setState({
					activeVideoType: VideoType.ASIDE,
				});
			}
		} else if (this.props.lastMotionTime !== nextProps.lastMotionTime) {
			this.startStandbyTimer(
				this.state.activeVideoType === VideoType.ASIDE ? TimeFromActiveToStandbyMs : TimeFromTutorialToStandbyMs,
			);

			if (this.state.activeVideoType === VideoType.STANDBY) {
				this.setState({
					activeVideoType: VideoType.TUTORIAL,
				});
			}
		}
	}

	componentWillUpdate = (nextProps, nextState) => {
		if (this.state.activeVideoType !== nextState.activeVideoType) {
			this.videoRefs[this.state.activeVideoType].pause();
			this.videoRefs[nextState.activeVideoType].load();
		}
	}

	render = () => (
		<div className={`video-carousel video-carousel--${this.state.activeVideoType}`}>
			{this.renderVideo(VideoType.ASIDE)}
			{this.renderVideo(VideoType.STANDBY)}
			{this.renderVideo(VideoType.TUTORIAL)}
		</div>
	);

	renderVideo = (name, isInFullScreen) => {
		let className = `video-carousel__content video-carousel__content--${name}`;

		if (isInFullScreen) {
			className += ' video-carousel__content--full';
		}

		if (name === this.state.activeVideoType) {
			className += ' video-carousel__content--active';
		}

		return (
			<div className={className}>
				<div className="video-carousel__content__label">{name}</div>
				<video src={this.props[`${name}Url`]} autoPlay loop ref={(ref) => { this.videoRefs[name] = ref; }} />
			</div>
		);
	}

	isVideoActive = name => name === this.state.activeVideoType;

	startStandbyTimer = (time) => {
		window.clearTimeout(this.standbyTimerId);

		this.standbyTimerId = window.setTimeout(() => {
			this.setState({
				activeVideoType: VideoType.STANDBY,
			});
		}, time);
	}

	videoRefs = {
		standby: null,
		tutorial: null,
		aside: null,
	};

	standbyTimerId = null;
}