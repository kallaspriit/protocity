package com.stagnationlab.etherio;


import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

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

    public List<String> getArguments(int startIndex) {
        return arguments.subList(startIndex, arguments.size());
    }

    public List<String> getArguments() {
        return arguments;
    }

    @Override
    public String toString() {
        String command = Integer.toString(id) + ":" + name;

        for (Object argument : arguments) {
            command += ":" + argument.toString();
        }

        return command;
    }

    public static Command parse(String command) throws IllegalArgumentException {
        // split on : ignoring escaped \: separators
        List<String> tokens = Arrays.stream(command.split("(?<!\\\\):"))
		        .map(token -> token.replaceAll("\\\\:", ":")).collect(Collectors.toList());

        if (tokens.size() < 2) {
            throw new IllegalArgumentException("Expected at least command id and name");
        }

        int id = Integer.parseInt(tokens.get(0));
        String name = tokens.get(1);
        List<String> arguments = new ArrayList<>();

        if (tokens.size() >= 3) {
            arguments.addAll(tokens.subList(2, tokens.size()));
        }

        return new Command(id, name, arguments.toArray());
    }

}
