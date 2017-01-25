package com.stagnationlab.c8y.driver.services;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;

import org.apache.commons.httpclient.util.URIUtil;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javazoom.jl.player.Player;

public class TextToSpeech {

	private static final Logger log = LoggerFactory.getLogger(TextToSpeech.class);

	public static void speak(String message) {

		try {
			String requestUrl = "https://translate.google.com/translate_tts?ie=UTF-8&q=" + URIUtil.encodeQuery(message, "UTF-8") + "&tl=en&client=tw-ob&ttsspeed=1";

			log.info("playing audio message '{}'", message);
			log.debug("audio message url: {}", requestUrl);

			URL url = new URL(requestUrl);
			URLConnection urlConn = url.openConnection();
			urlConn.addRequestProperty("User-Agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)");

			InputStream audioSrc = urlConn.getInputStream();
			BufferedInputStream bufferedInputStream = new BufferedInputStream(audioSrc);

			new Player(bufferedInputStream).play();
		} catch (Exception e) {
			log.warn("playing audio message '{}' failed ({} - {})", message, e.getClass().getSimpleName(), e.getMessage());

			e.printStackTrace();
		}
	}

}
