package com.stagnationlab.c8y.driver.fragments.sensors;


@SuppressWarnings("unused")
public class MonitoringSensor {

	public enum State {
		UNINITIALIZED,
		CONNECTING,
		RECONNECTING,
		CONNECTED,
		DISCONNECTED,
		CONNECTION_FAILED
	}

	private String name = "";
	private String host = "";
	private int port = 0;
	private String version = "";
	private State state = State.UNINITIALIZED;

	public MonitoringSensor(String name, String host, int port) {
		this.name = name;
		this.host = host;
		this.port = port;
	}

	public MonitoringSensor() {
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getHost() {
		return host;
	}

	public void setHost(String host) {
		this.host = host;
	}

	public int getPort() {
		return port;
	}

	public void setPort(int port) {
		this.port = port;
	}

	public State getState() {
		return state;
	}

	public void setState(State state) {
		this.state = state;
	}

	public String getVersion() {
		return version;
	}

	public void setVersion(String version) {
		this.version = version;
	}

}
