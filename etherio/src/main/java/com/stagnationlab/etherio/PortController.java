package com.stagnationlab.etherio;

import java.util.*;
import java.util.concurrent.CompletableFuture;

@SuppressWarnings({"WeakerAccess", "unused"})
public class PortController implements Commander.SpecialCommandListener {

    public enum PortMode {
        UNUSED,
        INVALID,
        DIGITAL_OUT,
        DIGITAL_IN,
        ANALOG_OUT,
        ANALOG_IN,
        INTERRUPT
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
        default void onPortDigitalValueChange(int id, DigitalValue value) {}
        default void onPortAnalogValueChange(int id, float value) {}
        default void onPortValueRise(int id) {}
        default void onPortValueFall(int id) {}
    }

    private enum Action {
        MODE,
        PULL,
        VALUE,
        READ,
        LISTEN;

        @Override
        public String toString() {
            return name().toLowerCase();
        }
    }

    private static final String COMMAND_PORT = "port";
    private static final String VALUE_OFF = "off";
    private static final String EVENT_INTERRUPT_CHANGE = "INTERRUPT_CHANGE";
    private static final String EVENT_INTERRUPT_RISE = "INTERRUPT_RISE";
    private static final String EVENT_INTERRUPT_FALL = "INTERRUPT_FALL";
    private static final String EVENT_ANALOG = "ANALOG_IN";

    private Commander commander;
    private final List<PortEventListener> portEventListeners;

    private final int id;
    private PortMode portMode = PortMode.UNUSED;
    private PullMode pullMode = PullMode.NONE;

    public PortController(int id, Commander commander) {
        this.id = id;
        this.commander = commander;
        this.portEventListeners = new ArrayList<>();

        commander.addSpecialCommandListener(this);
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

    public CompletableFuture<Commander.CommandResponse> setPortMode(PortMode portMode, PortEventListener listener) {
        CompletableFuture<Commander.CommandResponse> result = sendPortCommand(
                Action.MODE,
                portMode.name()
        );

        addEventListener(listener);

        return result;
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

    public CompletableFuture<Commander.CommandResponse> getDigitalValue() {
        return sendPortCommand(
                Action.READ
        );
    }

    public CompletableFuture<Commander.CommandResponse> setDigitalValue(DigitalValue value) {
        return sendPortCommand(
                Action.VALUE,
                value.name()
        );
    }

    public CompletableFuture<Commander.CommandResponse> getAnalogValue() {
        return sendPortCommand(
                Action.READ
        );
    }

    public CompletableFuture<Commander.CommandResponse> setAnalogValue(float dutyCycle) {
        return sendPortCommand(
                Action.VALUE,
                dutyCycle
        );
    }

    public CompletableFuture<Commander.CommandResponse> listenAnalogValueChange(float changeThreshold, int intervalMs) {
        return sendPortCommand(
                Action.LISTEN,
                changeThreshold,
                intervalMs
        );
    }

    public CompletableFuture<Commander.CommandResponse> listenAnalogValueChange(float changeThreshold, int intervalMs, PortEventListener listener) {
        CompletableFuture<Commander.CommandResponse> result = sendPortCommand(
                Action.LISTEN,
                changeThreshold,
                intervalMs
        );

        addEventListener(listener);

        return result;
    }

    public CompletableFuture<Commander.CommandResponse> stopAnalogValueListener() {
        return sendPortCommand(
                Action.LISTEN,
                VALUE_OFF
        );
    }

    public CompletableFuture<Commander.CommandResponse> sendPortCommand(Object... arguments) {
        List<Object> argumentList = new LinkedList<>(Arrays.asList(arguments));
        argumentList.add(0, id);

        return commander.sendCommand(COMMAND_PORT, argumentList.toArray());
    }

    @Override
    public void handleSpecialCommand(Command command) {
        String[] handledCommands = {
                EVENT_INTERRUPT_CHANGE,
                EVENT_INTERRUPT_RISE,
                EVENT_INTERRUPT_FALL,
                EVENT_ANALOG
        };

        boolean isHandledCommand = Arrays.asList(handledCommands).contains(command.name);

        // only handle predefined list of special commands
        if (!isHandledCommand) {
            return;
        }

        int portId = command.getInt(0);

        // only handle special commands of given port
        if (portId != id) {
            return;
        }

        // notify the event listeners
        for (PortEventListener listener : portEventListeners) {
            switch (command.name) {
                case EVENT_INTERRUPT_CHANGE:
                    listener.onPortDigitalValueChange(id, DigitalValue.valueOf(command.getString(1)));
                    break;

                case EVENT_INTERRUPT_RISE:
                    listener.onPortValueRise(id);
                    break;

                case EVENT_INTERRUPT_FALL:
                    listener.onPortValueFall(id);
                    break;

                case EVENT_ANALOG:
                    listener.onPortAnalogValueChange(id, command.getFloat(1));
                    break;
            }
        }
    }
}
