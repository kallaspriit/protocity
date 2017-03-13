package com.stagnationlab.c8y.driver.services;

public class ReadableSize {

	public enum Unit {
		B,
		KB,
		MB,
		GB,
		TB
	}

	public final double size;
	public final ReadableSize.Unit unit;

	public ReadableSize(double size, ReadableSize.Unit unit) {
		this.size = size;
		this.unit = unit;
	}

	public static ReadableSize fromBytes(long bytes) {
		if (bytes <= 0) {
			return new ReadableSize(0, ReadableSize.Unit.B);
		}

		int digitGroups = (int) (Math.log10(bytes) / Math.log10(1024));
		double size = bytes / Math.pow(1024, digitGroups);

		if (digitGroups > Unit.values().length - 1) {
			return new ReadableSize(bytes, ReadableSize.Unit.B);
		}

		ReadableSize.Unit unit = Unit.values()[digitGroups];

		return new ReadableSize(size, unit);
	}

}
