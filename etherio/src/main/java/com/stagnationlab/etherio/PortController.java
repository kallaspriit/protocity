package com.stagnationlab.etherio;

import java.util.*;
import java.util.concurrent.CompletableFuture;

@SuppressWarnings({"WeakerAccess", "unused"})
public class PortController implements Commander.SpecialCommandListener {

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
        default void onPortDigitalValueChange(int id, DigitalValue value) {}
        default void onPortAnalogValueChange(int id, float value) {}
        default void onPortValueRise(int id) {}
        default void onPortValueFall(int id) {}
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
    private static final String EVENT_INTERRUPT = "INTERRUPT";
    private static final String EVENT_ANALOG = "ANALOG";

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

    @Override
    public void handleSpecialCommand(Command command) {
        String[] handledCommands = {
                EVENT_INTERRUPT,
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
                case EVENT_INTERRUPT:
                    listener.onPortDigitalValueChange(id, DigitalValue.valueOf(command.getString(1)));
                    break;

                case EVENT_ANALOG:
                    listener.onPortAnalogValueChange(id, command.getFloat(1));
                    break;
            }
        }
    }
}
