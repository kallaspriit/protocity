package com.stagnationlab.c8y.driver.fragments;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

@SuppressWarnings("unused")
public class ParkingController {

	@SuppressWarnings("WeakerAccess")
	public static class SlotState {
		private int index;
		private boolean isOccupied;
		private String occupantName;
		private long occupiedTimestamp;
		private long freedTimestamp;

		public SlotState(int index, boolean isOccupied, String occupantName, long occupiedTimestamp, long freedTimestamp) {
			this.index = index;
			this.isOccupied = isOccupied;
			this.occupantName = occupantName;
			this.occupiedTimestamp = occupiedTimestamp;
			this.freedTimestamp = freedTimestamp;
		}

		public SlotState(int index) {
			this(index, false, "", 0, (new Date()).getTime());
		}

		public SlotState() {
			this(0);
		}

		public int getIndex() {
			return index;
		}

		public void setIndex(int index) {
			this.index = index;
		}

		public boolean getIsOccupied() {
			return isOccupied;
		}

		public void setIsOccupied(boolean isOccupied) {
			this.isOccupied = isOccupied;
		}

		public String getOccupantName() {
			return occupantName;
		}

		public void setOccupantName(String occupantName) {
			this.occupantName = occupantName;
		}

		public long getOccupiedTimestamp() {
			return occupiedTimestamp;
		}

		public void setOccupiedTimestamp(long occupiedTimestamp) {
			this.occupiedTimestamp = occupiedTimestamp;
		}

		public long getFreedTimestamp() {
			return freedTimestamp;
		}

		public void setFreedTimestamp(long freedTimestamp) {
			this.freedTimestamp = freedTimestamp;
		}

		public void occupy(String occupantName) {
			setIsOccupied(true);
			setOccupantName(occupantName);
			setOccupiedTimestamp((new Date()).getTime());
		}

		public void free() {
			setIsOccupied(false);
			setFreedTimestamp((new Date()).getTime());
		}
	}

	private List<SlotState> slots = new ArrayList<>();

	public List<SlotState> getSlots() {
		return slots;
	}

	public void setSlots(List<SlotState> slots) {
		this.slots = slots;
	}

	public SlotState slotByIndex(int index) {
		return slots.stream().filter(slot -> slot.getIndex() == index).findFirst().orElse(null);
	}

	public void addSlot(SlotState slot) {
		slots.add(slot);
	}
}
