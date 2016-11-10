package com.stagnationlab.etherio;


import java.util.ArrayList;
import java.util.List;

public class Command {

    public int id;
    public String name;
    public List<String> arguments;

    public Command(int id, String name, Object... arguments) {
        this.id = id;
        this.name = name;
        this.arguments = new ArrayList<>();

        for (Object argument : arguments) {
            this.arguments.add(argument.toString());
        }
    }

    @Override
    public String toString() {
        String command = Integer.toString(id) + ":" + name;

        for (Object argument : arguments) {
            command += ":" + argument.toString();
        }

        return command;
    }

    public static Command parse(String command) throws Exception {
        String[] tokens = command.split(":");

        if (tokens.length < 2) {
            throw new IllegalArgumentException("Expected at least command id and name");
        }

        int id = Integer.parseInt(tokens[0]);
        String name = tokens[1];
        List<String> arguments = new ArrayList<>();

        if (tokens.length >= 3) {
            for (int i = 2; i < tokens.length; i++) {
                arguments.add(tokens[i]);
            }
        }

        return new Command(id, name, arguments.toArray());
    }

}
