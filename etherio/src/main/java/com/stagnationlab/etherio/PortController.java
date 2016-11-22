package com.stagnationlab.etherio;

import java.util.*;
import java.util.concurrent.CompletableFuture;

@SuppressWarnings({"WeakerAccess", "unused"})
public class PortController {

    public enum PortMode {
        UNUSED,
        INVALID,
        OUTPUT,
        INPUT,
        INTERRUPT,
        PWM,
        ANALOG
    }

    public enum PullMode {
        NONE,
        UP,
        DOWN
    }

    public enum DigitalValue {
        LOW,
        HIGH
    }

    public interface PortEventListener {
        void onPortDigitalValueChange(int id, DigitalValue value);
        void onPortAnalogValueChange(int id, float value);
        void onPortValueRise(int id);
        void onPortValueFall(int id);
    }

    private enum Action {
        MODE,
        PULL,
        VALUE;

        @Override
        public String toString() {
            return name().toLowerCase();
        }
    }

    private static final String COMMAND_PORT = "port";

    private Commander commander;
    private final List<PortEventListener> portEventListeners;

    private final int id;
    private PortMode portMode = PortMode.UNUSED;
    private PullMode pullMode = PullMode.NONE;

    public PortController(int id, Commander commander) {
        this.id = id;
        this.commander = commander;
        this.portEventListeners = new ArrayList<>();
    }

    public void addEventListener(PortEventListener listener) {
        portEventListeners.add(listener);
    }

    public int getId() {
        return id;
    }

    public PortMode getPortMode() {
        return portMode;
    }

    public CompletableFuture<Commander.CommandResponse> setPortMode(PortMode portMode) {
        return sendPortCommand(
                Action.MODE,
                portMode.name()
        );
    }

    public PullMode getPullMode() {
        return pullMode;
    }

    public CompletableFuture<Commander.CommandResponse> setPullMode(PullMode pullMode) {
        return sendPortCommand(
                Action.PULL,
                pullMode.name()
        );
    }

    public CompletableFuture<Commander.CommandResponse> setValue(DigitalValue value) {
        return sendPortCommand(
                Action.VALUE,
                value.name()
        );
    }

    public CompletableFuture<Commander.CommandResponse> setPwmDutyCycle(float dutyCycle) {
        return sendPortCommand(
                Action.VALUE,
                dutyCycle
        );
    }



    public CompletableFuture<Commander.CommandResponse> sendPortCommand(Object... arguments) {
        List<Object> argumentList = new LinkedList<>(Arrays.asList(arguments));
        argumentList.add(0, id);

        return commander.sendCommand(COMMAND_PORT, argumentList.toArray());
    }
}
