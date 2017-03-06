package com.stagnationlab.c8y.driver.fragments;

@SuppressWarnings("unused")
public class Controller {

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

	public Controller(String name, String host, int port, String version, State state) {
		this.name = name;
		this.host = host;
		this.port = port;
		this.version = version;
		this.state = state;
	}

	public Controller() {
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

	public String getVersion() {
		return version;
	}

	public void setVersion(String version) {
		this.version = version;
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
}
