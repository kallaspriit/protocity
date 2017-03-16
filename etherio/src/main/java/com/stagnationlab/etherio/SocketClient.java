package com.stagnationlab.etherio;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.ConcurrentModificationException;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import lombok.extern.slf4j.Slf4j;

@SuppressWarnings("ForLoopReplaceableByForEach")
@Slf4j
public class SocketClient implements MessageTransport {

	public interface PingStrategy {
		String getPingMessage();
		boolean isPingResponse(String message);
	}

	private static int DEFAULT_RECONNECT_TIMEOUT = 5000;

	// runtime info
    private Socket socket;
	private BufferedReader socketIn;
    private PrintWriter socketOut;
    private Thread inputThread;
    private Thread sendPingTimeout;
    private Thread pingExpiredTimeout;
    private Thread reconnectTimeout;
	private PingStrategy pingStrategy = null;
    private final Queue<String> inputQueue = new LinkedList<>();
    private final List<EventListener> eventListeners = Collections.synchronizedList(new ArrayList<>());
    private int reconnectInterval = -1;
    private int messageCounter = 1;
    private boolean wasEverConnected = false;
	private boolean isConnecting = false;
	private boolean isFirstConnect = true;
	private boolean isPlannedClose = false;
	private long requestPingTime = 0;
	private static int inputThreadCount = 0;
	private static int timeoutThreadCount = 0;
	private int pingInterval = 0;
    private int lastConnectionTimeout = 0;
	private String hostName = "127.0.0.1";
	private int portNumber = 8080;


	@SuppressWarnings({ "unused", "WeakerAccess" })
	public SocketClient(String hostName, int portNumber, int reconnectInterval) {
		setRemoteHost(hostName, portNumber);
		setReconnectInterval(reconnectInterval);
	}

	@SuppressWarnings("unused")
	public SocketClient(String hostName, int portNumber) {
		this(hostName, portNumber, DEFAULT_RECONNECT_TIMEOUT);
	}

    @SuppressWarnings("WeakerAccess")
    public void setRemoteHost(String hostName, int portNumber) {
        this.hostName = hostName;
        this.portNumber = portNumber;
    }

	@SuppressWarnings("WeakerAccess")
	public void setReconnectInterval(int reconnectInterval) {
		this.reconnectInterval = reconnectInterval;
	}

	public String getDescription() {
		return hostName + ":" + portNumber;
	}

	@SuppressWarnings("unused")
	public void setPingStrategy(PingStrategy pingStrategy, int pingInterval) {
		this.pingStrategy = pingStrategy;
		this.pingInterval = pingInterval;
	}

    @SuppressWarnings({ "SameParameterValue", "WeakerAccess" })
    @Override
    public boolean connect(int connectionTimeout) {
	    log.debug("connecting to {}:{}", hostName, portNumber);

	    isPlannedClose = false;
	    lastConnectionTimeout = connectionTimeout;

	    /*
	    for (EventListener eventListener : eventListeners) {
		    eventListener.onConnecting(wasEverConnected);
	    }
	    */

	    synchronized (eventListeners) {
		    for (Iterator<EventListener> it = eventListeners.iterator(); it.hasNext(); ) {
			    EventListener eventListener = it.next();

			    eventListener.onConnecting(wasEverConnected);
		    }
	    }

	    try {
		    isConnecting = true;

		    socket = new Socket();
		    socket.setSoTimeout(1000);
		    socket.connect(new InetSocketAddress(hostName, portNumber), connectionTimeout);
		    socketOut = new PrintWriter(socket.getOutputStream(), true);
		    socketIn = new BufferedReader(new InputStreamReader(socket.getInputStream()));

		    isFirstConnect = !wasEverConnected;

		    isConnecting = false;
		    wasEverConnected = true;

		    inputThread = new Thread(getInputThreadTask(), "InputThread-" + (inputThreadCount++));
		    inputThread.start();

		    clearPingExpiredTimeout();
		    sendPing();

		    log.debug("connected to {}:{}", hostName, portNumber);

		    /*
		    for (EventListener eventListener : eventListeners) {
			    eventListener.onOpen(isFirstConnect);
		    }
		    */

		    synchronized (eventListeners) {
			    for (Iterator<EventListener> it = eventListeners.iterator(); it.hasNext(); ) {
				    EventListener eventListener = it.next();

				    eventListener.onOpen(isFirstConnect);
			    }
		    }

		    return true;
	    } catch (IOException e) {
	    	/*
		    for (EventListener eventListener : eventListeners) {
			    eventListener.onConnectionFailed(e, wasEverConnected);
		    }
		    */

		    synchronized (eventListeners) {
			    for (Iterator<EventListener> it = eventListeners.iterator(); it.hasNext(); ) {
				    EventListener eventListener = it.next();

				    eventListener.onConnectionFailed(e, wasEverConnected);
			    }
		    }

	    	if (wasEverConnected && reconnectInterval >= 0) {
			    log.debug("connecting to {}:{} failed, reconnecting in {}ms", hostName, portNumber, reconnectTimeout);

			    scheduleReconnect(reconnectInterval);
		    } else {
		    	if (!wasEverConnected) {
				    log.warn("connecting to {}:{} failed", hostName, portNumber);
			    } else {
		    		log.debug("reconnecting to {}:{} failed", hostName, portNumber);
			    }
		    }

		    return false;
	    }
    }

	@SuppressWarnings("unused")
	@Override
	public void close() {
		log.info("closing the socket to {}:{} without automatic reconnecting", hostName, portNumber);

		isPlannedClose = true;

		closeSocket();
	}

    public void addEventListener(EventListener eventListener) {
        eventListeners.add(eventListener);

        // call the onOpen event if the socket is already connected
        if (isConnected()) {
        	eventListener.onOpen(isFirstConnect);
        }
    }

    public boolean sendMessage(String format, Object...arguments) {
	    String message = String.format(format, arguments);

    	if (socketOut == null) {
		    log.warn("requested sending message '{}' to {}:{} but the output stream is not available", message.replace("\n", "\\n"), hostName, portNumber);

    		return false;
	    }

	    log.trace("sending message: '{}' to {}:{}", message.replace("\n", "\\n"), hostName, portNumber);

        socketOut.print(message);

        return !socketOut.checkError();
    }

    public int getMessageCount() {
        return inputQueue.size();
    }

    public String getMessage() {
        String message = inputQueue.peek();

        if (message == null) {
            return null;
        }

        inputQueue.remove();

        return message;
    }

    @Override
    @SuppressWarnings("unused")
    public boolean isConnected() {
	    return socket != null && socket.isConnected() && !socket.isClosed();
    }

    public int getNextMessageId() {
        return messageCounter++;
    }

    private void closeSocket() {
		if (isConnecting) {
			log.debug("close requested while attempting to connect, avoiding reconnect");

			wasEverConnected = false;

			return;
		}

		if (!isConnected()) {
			log.debug("close requested but socket is not connected, ignoring");

			return;
		}

	    clearSendPingTimeout();
	    clearPingExpiredTimeout();
	    clearReconnectTimeout();

	    log.debug("closing socket connection to {}:{}", hostName, portNumber);

		try {
			socket.close();

		} catch (IOException e) {
			log.warn("closing socket to {}:{} failed ({} - {})", hostName, portNumber, e.getClass().getSimpleName(), e.getMessage());

			e.printStackTrace();
		}

		if (inputThread.isAlive() && Thread.currentThread() != inputThread) {
			log.debug("waiting for the input thread to complete");

			try {
				inputThread.join();

				log.debug("input thread has successfully completed");
			} catch (InterruptedException e) {
				log.warn("joining input thread failed ({} - {})", e.getClass().getSimpleName(), e.getMessage());

				e.printStackTrace();
			}
		}

		/*
	    for (EventListener eventListener : ) {
		    eventListener.onClose(isPlannedClose);
	    }
	    */

	    synchronized (eventListeners) {
		    for (Iterator<EventListener> it = eventListeners.iterator(); it.hasNext(); ) {
			    EventListener eventListener = it.next();

			    eventListener.onClose(isPlannedClose);
		    }
	    }

	    log.debug("socket to {}:{} has been closed", hostName, portNumber);

	    if (wasEverConnected && reconnectInterval >= 0 && !isPlannedClose) {
		    scheduleReconnect(reconnectInterval);
	    }
    }

	private void scheduleReconnect(int timeout) {
		if (isPlannedClose) {
			log.warn("scheduling reconnect requested but final close already called, ignoring it");

			return;
		}

		if (timeout == 0) {
			reconnect();
		} else if (timeout > 0) {
			log.debug("scheduling reconnect to {}:{} in {}ms", hostName, portNumber, timeout);

			reconnectTimeout = setTimeout(this::reconnect, timeout);
		}
	}

	@SuppressWarnings("WeakerAccess")
	private void reconnect() {
		if (isPlannedClose) {
			log.warn("reconnect requested but final close already called, ignoring it");

			return;
		}

		if (isConnected()) {
			log.debug("reconnect requested but the connection is already open, closing existing");

			closeSocket();
		}

		log.debug("attempting to reconnect to {}:{}", hostName, portNumber);

		connect(lastConnectionTimeout);
	}

	private Runnable getInputThreadTask() {

		return () -> {
			log.debug("starting input thread");

			while (isConnected()) {
				try {
					log.trace("attempting to read message from {}:{}", hostName, portNumber);

					String message = socketIn.readLine();

					if (message != null) {
						if (pingStrategy != null && pingStrategy.isPingResponse(message)) {
							long currentTime = now();
							long pingLatency = currentTime - requestPingTime;

							log.trace("got ping response for {}:{} '{}' in {}ms", hostName, portNumber, message, pingLatency);

							clearPingExpiredTimeout();

							log.trace("setting ping timeout at {}ms", pingInterval);

							sendPingTimeout = setTimeout(() -> {
								if (!isConnected()) {
									log.debug("ping timeout reached but connection has been lost, skipping it");

									return;
								}

								sendPing();
							}, pingInterval);
						} else {
							log.trace("received message from {}:{}: '{}'", hostName, portNumber, message);

							inputQueue.add(message);

							/*
							for (EventListener eventListener : eventListeners) {
								eventListener.onMessageReceived(message);
							}
							*/

							synchronized (eventListeners) {
								for (Iterator<EventListener> it = eventListeners.iterator(); it.hasNext(); ) {
									EventListener eventListener = it.next();

									eventListener.onMessageReceived(message);
								}
							}
						}
					}
				} catch (ConcurrentModificationException e) {
					log.warn("operation for {}:{} performed concurrent modification", hostName, portNumber, e);

					// TODO find and fix the concurrent modification issue
				} catch (SocketTimeoutException e) {
					log.trace("socket read from {}:{} timed out", hostName, portNumber);
				} catch (Exception e) {
					if (isPlannedClose) {
						log.debug("got planned close socket exception for {}:{} ({} - {})", hostName, portNumber, e.getClass().getSimpleName(), e.getMessage());
					} else {
						log.warn("got exception for {}:{}, stopping input thread ({} - {})", hostName, portNumber, e.getClass().getSimpleName(), e.getMessage(), e);
					}

					break;
				}
			}

			if (isConnected()) {
				log.debug("connection to {}:{} was lost, closing socket", hostName, portNumber);

				closeSocket();
			}

			log.trace("input thread completed for {}:{}", hostName, portNumber);
		};
	}

	private synchronized void clearSendPingTimeout() {
		if (sendPingTimeout == null) {
			return;
		}

		log.trace("clearing send ping timeout for {}:{}", hostName, portNumber);

		sendPingTimeout.interrupt();
		sendPingTimeout = null;
	}

	private synchronized void clearPingExpiredTimeout() {
		if (pingExpiredTimeout == null) {
			return;
		}

		log.trace("clearing ping expired timeout for {}:{}", hostName, portNumber);

		pingExpiredTimeout.interrupt();
		pingExpiredTimeout = null;
	}

	private synchronized void clearReconnectTimeout() {
		if (reconnectTimeout == null) {
			return;
		}

		log.trace("clearing reconnect timeout for {}:{}", hostName, portNumber);

		reconnectTimeout.interrupt();
		reconnectTimeout = null;
	}

	private void sendPing() {
		if (pingStrategy == null) {
			log.info("ping strategy has not been set for {}:{}, not using pinging", hostName, portNumber);

			return;
		}

		String pingMessage = pingStrategy.getPingMessage();

		log.trace("sending ping message to {}:{}: {}", hostName, portNumber, pingMessage);

		requestPingTime = now();
		sendMessage(pingMessage);

		if (pingExpiredTimeout != null) {
			log.warn("setting ping timeout but one already exists, ignoring the request");

			return;
		}

		pingExpiredTimeout = setTimeout(() -> {
			log.warn("ping timed out for {}:{}, connection must have been lost", hostName, portNumber);

			pingExpiredTimeout = null;

			closeSocket();
		}, pingInterval);
	}

	private Thread setTimeout(Runnable runnable, int delay){
		Thread thread = new Thread(() -> {
			try {
				Thread.sleep(delay);

				runnable.run();
			}  catch (Exception e) {
				log.trace("timeout was interrupted ({} - {})", e.getClass().getSimpleName(), e.getMessage());
			}
		}, "Timeout-" + (timeoutThreadCount++));

		thread.start();

		return thread;
	}

	private long now() {
		return Calendar.getInstance().getTimeInMillis();
	}
}