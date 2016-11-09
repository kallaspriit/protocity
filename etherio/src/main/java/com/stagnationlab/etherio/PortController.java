package com.stagnationlab.etherio;


public class PortController implements MessageTransport.MessageListener {

    MessageTransport messageTransport;

    public PortController(MessageTransport messageTransport) {
        this.messageTransport = messageTransport;

        messageTransport.addMessageListener(this);
    }

    public void x() {
        sendCommand("0:memory");
    }

    void sendCommand(String command) {
        System.out.printf("< %s%n", command);

        messageTransport.sendMessage("%s\n", command);
    }

    @Override
    public void onSocketMessageReceived(String message) {
        System.out.printf("> %s%n", message);
    }
}
