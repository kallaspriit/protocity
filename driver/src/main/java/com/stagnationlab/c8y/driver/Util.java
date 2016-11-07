package com.stagnationlab.c8y.driver;


public class Util {

    public static double map(double value, double inMin, double inMax, double outMin, double outMax) {
        return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
    }

    public static double convertBytesToMb(long bytes) {
        return bytes / 1048576;
    }

}
