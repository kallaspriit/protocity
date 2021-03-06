package com.stagnationlab.c8y.driver.fragments.sensors;

@SuppressWarnings("unused")
public class TagSensor {

	private boolean isRunning;
	private String tagName;
	private String tagUid;
	private boolean isTagActive;

	public TagSensor() {
		reset();
	}

	public void reset() {
		isRunning = false;
		tagName = "";
		isTagActive = false;
	}

	public boolean getIsRunning() {
		return isRunning;
	}

	public void setIsRunning(boolean isRunning) {
		this.isRunning = isRunning;
	}

	public String getTagName() {
		return tagName;
	}

	public void setTagName(String tagName) {
		this.tagName = tagName;
	}

	public String getTagUid() {
		return tagUid;
	}

	public void setTagUid(String tagUid) {
		this.tagUid = tagUid;
	}

	public boolean isTagActive() {
		return isTagActive;
	}

	public void setTagActive(boolean tagActive) {
		isTagActive = tagActive;
	}
}
