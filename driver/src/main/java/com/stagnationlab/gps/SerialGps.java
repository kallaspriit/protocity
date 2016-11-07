package com.stagnationlab.gps;

import com.fazecast.jSerialComm.SerialPort;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

@SuppressWarnings({"unused", "WeakerAccess"})
public class SerialGps {

    public interface StateListener {
        void onGpsStateUpdated(NMEA.GpsState state);
    }

    private static final Logger log = LoggerFactory.getLogger(SerialGps.class);

    private String portName;
    private int baudRate;
    private boolean isRunning = false;
    private List<StateListener> stateListeners = new ArrayList<>();

    public SerialGps(String portName, int baudRate) {
        this.portName = portName;
        this.baudRate = baudRate;
    }

    public SerialGps(String portName) {
        this(portName, 4800);
    }

    public void addStateListener(StateListener stateListener) {
        stateListeners.add(stateListener);
    }

    public void start() throws Exception {
        NMEA nmea = new NMEA();

        SerialPort[] serialPorts = SerialPort.getCommPorts();
        SerialPort gpsPort = null;

        for (SerialPort serialPort : serialPorts) {
            log.info("found serial port: " + serialPort.getDescriptivePortName());

            if (
                    serialPort.getDescriptivePortName().toLowerCase().contains(portName)
                    || serialPort.getDescriptivePortName().toLowerCase().contains("serial")
            ) {
                gpsPort = serialPort;
            }
        }

        if (gpsPort == null) {
            throw new Exception("failed to find gps serial port");
        }

        log.info("using gps serial port: " + gpsPort.getDescriptivePortName());

        gpsPort.setBaudRate(baudRate);
        gpsPort.openPort();
        InputStream inStream = gpsPort.getInputStream();

        if (inStream == null) {
            throw new Exception("opening port " + gpsPort.getDescriptivePortName() + " failed");
        }

        Thread thread = new Thread(() -> {
            String line = "";

            isRunning = true;

            while (isRunning) {
                try {
                    if (inStream.available() > 0) {
                        char b = (char) inStream.read();

                        if (b == '\n') {
                            NMEA.GpsState gpsState = nmea.getUpdatedStatus(line);

                            updateState(gpsState);

                            line = "";
                        } else {
                            line += b;
                        }
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });

        thread.start();
    }

    public void stop() throws InterruptedException {
        isRunning = false;
    }

    private void updateState(NMEA.GpsState gpsState) {
        stateListeners.forEach(stateListener -> stateListener.onGpsStateUpdated(gpsState));
    }

}
