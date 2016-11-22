package com.stagnationlab.etherio;


import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

@SuppressWarnings({"WeakerAccess", "unused"})
public class Command {

    public final int id;
    public final String name;
    private final List<String> arguments;

    public Command(int id, String name, Object... arguments) {
        this.id = id;
        this.name = name;
        this.arguments = new ArrayList<>();

        for (Object argument : arguments) {
            this.arguments.add(argument.toString());
        }
    }

    public String getString(int argumentIndex) {
        if (argumentIndex + 1 > arguments.size()) {
            throw new IllegalArgumentException("Invalid argument index provided");
        }

        return arguments.get(argumentIndex);
    }

    public int getInt(int argumentIndex) {
        return Integer.parseInt(getString(argumentIndex));
    }

    public float getFloat(int argumentIndex) {
        return Float.parseFloat(getString(argumentIndex));
    }

    public double getDouble(int argumentIndex) {
        return Double.parseDouble(getString(argumentIndex));
    }

    @Override
    public String toString() {
        String command = Integer.toString(id) + ":" + name;

        for (Object argument : arguments) {
            command += ":" + argument.toString();
        }

        return command;
    }

    public static Command parse(String command) {
        String[] tokens = command.split(":");

        if (tokens.length < 2) {
            throw new IllegalArgumentException("Expected at least command id and name");
        }

        int id = Integer.parseInt(tokens[0]);
        String name = tokens[1];
        List<String> arguments = new ArrayList<>();

        if (tokens.length >= 3) {
            arguments.addAll(Arrays.asList(tokens).subList(2, tokens.length));
        }

        return new Command(id, name, arguments.toArray());
    }

}
