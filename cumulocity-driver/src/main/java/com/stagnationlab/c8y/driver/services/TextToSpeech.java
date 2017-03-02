package com.stagnationlab.c8y.driver.services;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLEncoder;
import java.util.LinkedList;
import java.util.Queue;

import lombok.extern.slf4j.Slf4j;

import javazoom.jl.player.Player;

@Slf4j
class PlayMessageTask implements Runnable {

	interface Callback {
		void onPlayMessageComplete();
	}

	private final String message;
	private final Callback callback;
	private Player player = null;
	private boolean isCancelRequested = false;

	PlayMessageTask(String message, Callback callback) {
		this.message = message;
		this.callback = callback;
	}

	void stop() {
		if (player != null) {
			log.debug("stopping already playing message '{}'", message);

			player.close();
			player = null;
		} else if (!isCancelRequested) {
			log.debug("cancelling preparing to play message '{}'", message);

			isCancelRequested = true;
		}
	}

	@Override
	public void run() {
		isCancelRequested = false;

		try {
			String requestUrl = "https://translate.google.com/translate_tts?ie=UTF-8&q=" + URLEncoder.encode(message, "UTF-8") + "&tl=en&client=tw-ob&ttsspeed=1";

			log.debug("preparing message '{}' from '{}'", message, requestUrl);

			URL url = new URL(requestUrl);
			URLConnection urlConn = url.openConnection();
			urlConn.addRequestProperty("User-Agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)");

			InputStream audioSrc = urlConn.getInputStream();
			BufferedInputStream bufferedInputStream = new BufferedInputStream(audioSrc);

			if (isCancelRequested) {
				log.debug("cancelling playing message '{}'", message);

				return;
			}

			log.info("playing message '{}'", message);

			player = new Player(bufferedInputStream);
			player.play();

			log.debug("completed playing message '{}'", message);
		} catch (Exception e) {
			log.warn("failed playing message '{}' ({} - {})", message, e.getClass().getSimpleName(), e.getMessage());

			e.printStackTrace();
		} finally {
			player = null;
			isCancelRequested = false;

			log.debug("completed task to play '{}'", message);

			callback.onPlayMessageComplete();
		}
	}
}

@SuppressWarnings("SameParameterValue")
@Slf4j
public enum TextToSpeech {
	INSTANCE;

	private final Queue<String> messageQueue = new LinkedList<>();
	private PlayMessageTask playMessageTask = null;
	private Thread playMessageThread = null;

	public void speak(String message, boolean cancelPrevious) {
		synchronized (messageQueue) {
			if (cancelPrevious) {
				log.debug("replacing message queue with '{}'", message);

				messageQueue.clear();
			} else {
				log.debug("adding message '{}' to the queue", message);
			}

			messageQueue.add(message);
		}

		if (isPlaying() && cancelPrevious) {
			log.debug("stopping currently playing message");

			stop();
		}

		if (!isPlaying()) {
			log.debug("no message is currently playing, playing next queued message");

			playNextMessage();
		}
	}

	@SuppressWarnings("unused")
	public void speak(String message) {
		speak(message, false);
	}

	@SuppressWarnings("WeakerAccess")
	public boolean isPlaying() {
		return playMessageTask != null;
	}

	private void playNextMessage() {
		log.debug("playNextMessage() ({} in queue)", messageQueue.size());

		stop();

		synchronized (messageQueue) {
			if (messageQueue.size() == 0) {
				log.debug("there are no more messages queued to play");

				return;
			}

			String message = messageQueue.remove();

			log.debug("creating task to play '{}', currently {}", message, isPlaying() ? "already playing" : "not playing");

			playMessageTask = new PlayMessageTask(message, () -> {
				log.debug("playing message completed, checking for next one");

				playMessageTask = null;
				playMessageThread = null;

				playNextMessage();
			});

			playMessageThread = new Thread(playMessageTask);
			playMessageThread.start();
		}

		/*
		Future<?> future = executorService.submit(this.playMessageTask);
		future.isDone()
		*/
	}

	@SuppressWarnings("WeakerAccess")
	public void stop() {
		try {
			if (playMessageTask != null) {
				playMessageTask.stop();
			}

			if (playMessageThread != null) {
				log.debug("joining play message thread");

				playMessageThread.join();

				log.debug("joined play message thread");
			}
		} catch (InterruptedException e) {
			log.warn("joining playing thread failed");

			e.printStackTrace();
		} finally {
			playMessageThread = null;
			//playMessageTask = null;
		}
	}
}
