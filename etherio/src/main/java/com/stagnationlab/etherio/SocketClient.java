package com.stagnationlab.etherio;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.*;

import lombok.extern.slf4j.Slf4j;

@Slf4j
public class SocketClient implements MessageTransport {

	public interface PingStrategy {
		String getPingMessage();
		boolean isPingResponse(String message);
	}

	// configuration
    private String hostName = "127.0.0.1";
    private int portNumber = 8080;
	private int reconnectTimeout = 5000;
	private int pingInterval = 1000;

	// runtime info
    private Socket socket;
	private BufferedReader socketIn;
    private PrintWriter socketOut;
    private Thread inputThread;
    private Thread pingThread;
	private PingStrategy pingStrategy = null;
    private final Queue<String> inputQueue = new LinkedList<>();
	//private final Queue<Integer> latencies = new LinkedList<>();
    private final List<EventListener> eventListeners = new ArrayList<>();
    private int messageCounter = 1;
    private boolean wasEverConnected = false;
	private volatile boolean isConnecting = false;
	private volatile boolean isConnected = false;
	private volatile boolean isExpectingPingResponse = false;
	private volatile boolean isReconnectScheduled = false;

    private int lastConnectionTimeout = 0;

	@SuppressWarnings("unused")
	public SocketClient(String hostName, int portNumber, int reconnectTimeout) {
		setRemoteHost(hostName, portNumber);
		setReconnectTimeout(reconnectTimeout);
	}

    @SuppressWarnings("WeakerAccess")
    public void setRemoteHost(String hostName, int portNumber) {
        this.hostName = hostName;
        this.portNumber = portNumber;
    }

	@SuppressWarnings("WeakerAccess")
	public void setReconnectTimeout(int reconnectTimeout) {
		this.reconnectTimeout = reconnectTimeout;
	}

	@SuppressWarnings("unused")
	public void setPingStrategy(PingStrategy pingStrategy, int pingInterval) {
		this.pingStrategy = pingStrategy;
		this.pingInterval = pingInterval;
	}

    @SuppressWarnings({ "SameParameterValue", "WeakerAccess" })
    public boolean connect(int connectionTimeout) {
	    log.debug("connecting to {}:{}", hostName, portNumber);

	    lastConnectionTimeout = connectionTimeout;

	    for (EventListener eventListener : eventListeners) {
		    eventListener.onConnecting(wasEverConnected);
	    }

	    try {
		    isConnecting = true;

		    socket = new Socket();
		    socket.connect(new InetSocketAddress(hostName, portNumber), connectionTimeout);
		    socketOut = new PrintWriter(socket.getOutputStream(), true);
		    socketIn = new BufferedReader(new InputStreamReader(socket.getInputStream()));

		    inputThread = new Thread(getInputThreadTask(), "InputThread");
		    inputThread.start();

		    if (pingStrategy != null) {
			    pingThread = new Thread(getPingThreadTask(), "PingThread");
			    pingThread.start();
		    }

		    isConnecting = false;
		    isConnected = true;
		    wasEverConnected = true;

		    log.debug("connected to {}:{}", hostName, portNumber);

		    for (EventListener eventListener : eventListeners) {
			    eventListener.onOpen();
		    }

		    return true;
	    } catch (IOException e) {
		    for (EventListener eventListener : eventListeners) {
			    eventListener.onConnectionFailed(e);
		    }

	    	if (wasEverConnected && reconnectTimeout >= 0) {
			    log.debug("connecting to {}:{} failed, reconnecting in {}ms", hostName, portNumber, reconnectTimeout);

			    scheduleReconnect(reconnectTimeout);
		    } else {
			    log.warn("connecting to {}:{} failed", hostName, portNumber);
		    }

		    return false;
	    }
    }

    public void addEventListener(EventListener eventListener) {
        eventListeners.add(eventListener);

        // call the onOpen event if the socket is already connected
        if (isConnected()) {
        	eventListener.onOpen();
        }
    }

    public boolean sendMessage(String format, Object...arguments) {
	    String message = String.format(format, arguments);

    	if (socketOut == null) {
		    log.warn("requested sending message '{}' to {}:{} but the output stream is not available", message.replace("\n", "\\n"), hostName, portNumber);

    		return false;
	    }

	    log.trace("sending message: '{}'", message.replace("\n", "\\n"));

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
		return isConnected;
    }

    public int getNextMessageId() {
        return messageCounter++;
    }

	@SuppressWarnings("WeakerAccess")
    public void close() {
		if (isReconnectScheduled) {
			log.debug("close called, cancelling scheduled reconnect");

			isReconnectScheduled = false;
		}

		if (isConnecting) {
			log.debug("close requested while attempting to connect, avoiding reconnect");

			wasEverConnected = false;

			return;
		}

    	if (!isConnected) {
    		return;
	    }

	    log.debug("closing socket connection to {}:{}", hostName, portNumber);

	    isConnected = false;
		isExpectingPingResponse = false;

		try {
			socket.close();
		} catch (IOException e) {
			log.warn("closing socket to {}:{} failed ({} - {})", hostName, portNumber, e.getClass().getSimpleName(), e.getMessage());

			e.printStackTrace();
		}

		// close can be called from the ping thread, avoiding waiting for oneself
		if (pingThread != null && Thread.currentThread() != pingThread) {
			try {
				log.debug("waiting for the ping thread to complete");

				pingThread.join();

				log.debug("ping thread has successfully completed");
			} catch (InterruptedException e) {
				log.warn("joining ping thread failed ({} - {})", e.getClass().getSimpleName(), e.getMessage());

				e.printStackTrace();
			}
		}

		try {
			log.debug("waiting for the input thread to complete");

			inputThread.join();

			log.debug("input thread has successfully completed");
		} catch (InterruptedException e) {
			log.warn("joining input thread failed ({} - {})", e.getClass().getSimpleName(), e.getMessage());

			e.printStackTrace();
		}

	    for (EventListener eventListener : eventListeners) {
		    eventListener.onClose();
	    }

	    log.debug("socket to {}:{} has been closed", hostName, portNumber);
    }

	@SuppressWarnings("WeakerAccess")
	public void scheduleReconnect(int timeout) {
		if (timeout == 0) {
			reconnect();
		} else if (timeout > 0) {
			log.debug("scheduling reconnect to {}:{} in {}ms", hostName, portNumber, timeout);

			isReconnectScheduled = true;

			setTimeout(() -> {
				if (!isReconnectScheduled) {
					log.debug("reconnecting has been cancelled");

					return;
				}

				reconnect();
			}, timeout);
		}
	}

	@SuppressWarnings("WeakerAccess")
	public void reconnect() {
		if (isConnected) {
			log.debug("reconnect requested but the connection is already open, closing existing");

			close();
		}

		log.debug("attempting to reconnect to {}:{}", hostName, portNumber);

		connect(lastConnectionTimeout);
	}

	private Runnable getInputThreadTask() {
		return () -> {
			log.debug("starting input thread");

			boolean wasConnectionLost = false;

			while (isConnected) {
				try {
					String message = socketIn.readLine();

					if (!isConnected) {
						break;
					}

					if (message != null) {
						log.trace("received message: '{}'", message);

						if (pingStrategy != null && pingStrategy.isPingResponse(message)) {
							log.trace("got ping response '{}'", message);

							isExpectingPingResponse = false;
						} else {
							inputQueue.add(message);

							synchronized (this) {
								for (EventListener eventListener : eventListeners) {
									eventListener.onMessageReceived(message);
								}
							}
						}
					}
				} catch (Exception e) {
					// if the socket client was still running during the error, the connection was lost
					if (isConnected) {
						log.warn("reading from {}:{} failed, stopping input loop ({} - {})", hostName, portNumber, e.getClass().getSimpleName(), e.getMessage());

						wasConnectionLost = true;
					}

					break;
				}
			}

			if (wasConnectionLost) {
				log.debug("connection to {}:{} was lost, closing socket", hostName, portNumber);

				close();

				if (wasEverConnected && reconnectTimeout >= 0) {
					log.debug("connection to {}:{} was lost, reconnecting in {}ms", hostName, portNumber, reconnectTimeout);

					scheduleReconnect(reconnectTimeout);
				} else {
					log.debug("connection to {}:{} was lost", hostName, portNumber);
				}
			} else {
				log.debug("input thread completed");
			}
		};
	}

	private Runnable getPingThreadTask() {
		return () -> {
			log.debug("starting ping thread");

			isExpectingPingResponse = false;

			boolean pingResponseNotReceived = false;

			while (isConnected) {
				try {
					Thread.sleep(pingInterval);
				} catch (InterruptedException e) {
					log.debug("ping thread sleep was interrupted");

					return;
				}

				if (Thread.currentThread().isInterrupted()) {
					break;
				}

				if (!isConnected) {
					break;
				}

				if (isExpectingPingResponse) {
					pingResponseNotReceived = true;
					isExpectingPingResponse = false;

					break;
				}

				log.trace("sending ping message");

				String pingMessage = pingStrategy.getPingMessage();

				sendMessage(pingMessage);

				isExpectingPingResponse = true;
			}

			log.debug("ping thread completed");

			if (pingResponseNotReceived) {
				log.warn("ping response from {}:{} was not received in {}ms, connection must have died, reconnecting in {}ms", hostName, portNumber, pingInterval, reconnectTimeout);

				scheduleReconnect(reconnectTimeout);
			}
		};
	}

	private Thread setTimeout(Runnable runnable, int delay){
		Thread thread = new Thread(() -> {
			try {
				Thread.sleep(delay);

				runnable.run();
			}  catch (Exception e) {
				log.debug("timeout was interrupted ({} - {})", e.getClass().getSimpleName(), e.getMessage());
			}
		});

		thread.start();

		return thread;
	}
}