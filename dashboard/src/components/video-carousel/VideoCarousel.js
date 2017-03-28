import React, { Component, PropTypes } from 'react';
import './video-carousel.scss';

const VideoType = {
	STANDBY: 'standby',
	TUTORIAL: 'tutorial',
	ASIDE: 'aside',
};

const SyncedVideos = [VideoType.STANDBY];

const TimeFromTutorialToStandbyMs = 1000 * 20;
const TimeFromActiveToStandbyMs = 1000 * 60;

export default class VideoCarousel extends Component {
	static propTypes = {
		lastMotionTime: PropTypes.number,
		lastActivatedTime: PropTypes.number,
		lastDeactivatedTime: PropTypes.number,  // eslint-disable-line
		standbyUrl: PropTypes.string.isRequired, // eslint-disable-line
		tutorialUrl: PropTypes.string.isRequired, // eslint-disable-line
		asideUrl: PropTypes.string.isRequired, // eslint-disable-line
	};

	state = {
		activeVideoType: VideoType.STANDBY,
	};

	componentWillReceiveProps = (nextProps) => {
		if (this.props.lastDeactivatedTime !== nextProps.lastDeactivatedTime) {
			this.startStandbyTimer(TimeFromActiveToStandbyMs);
		} else if (this.props.lastActivatedTime !== nextProps.lastActivatedTime) {
			if (nextProps.lastDeactivatedTime === undefined) {
				this.startStandbyTimer(TimeFromActiveToStandbyMs);
			}

			if (this.state.activeVideoType !== VideoType.ASIDE) {
				this.setState({
					activeVideoType: VideoType.ASIDE,
				});
			}
		} else if (this.props.lastMotionTime !== nextProps.lastMotionTime) {
			this.startStandbyTimer(this.state.activeVideoType === VideoType.ASIDE
				? TimeFromActiveToStandbyMs : TimeFromTutorialToStandbyMs,
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
				<video
					src={this.props[`${name}Url`]}
					autoPlay
					loop
					ref={(ref) => { this.videoRefs[name] = ref; }}
					onLoadedData={SyncedVideos.indexOf(name) !== -1 && this.syncVideo}
				/>
			</div>
		);
	}

	isVideoActive = name => name === this.state.activeVideoType;

	startStandbyTimer = (time) => {
		window.clearTimeout(this.standbyTimerId);

		this.standbyTimerId = window.setTimeout(() => {
			const remaining = this.getVideoRemainingPlayTime(this.videoRefs[VideoType.STANDBY]) - 2000;

			this.standbyTimerId = window.setTimeout(() => {
				this.setState({
					activeVideoType: VideoType.STANDBY,
				});
			}, remaining);
		}, time);
	}

	syncVideo = (e) => {
		const video = e.target;
		const timeToWait = this.getVideoRemainingPlayTime(video);

		video.currentTime = 0;
		video.pause();

		window.setTimeout(() => {
			video.play();
		}, timeToWait);
	}

	getVideoRemainingPlayTime = (videoNode) => {
		const now = Date.now();
		const videoDuration = videoNode.duration * 1000;
		const playedTime = (now % videoDuration);

		return videoDuration - playedTime;
	}

	videoRefs = {
		standby: null,
		tutorial: null,
		aside: null,
	};

	standbyTimerId = null;
}
