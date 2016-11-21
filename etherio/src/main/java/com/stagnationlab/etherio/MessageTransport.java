package com.stagnationlab.etherio;

interface MessageTransport {

    interface MessageListener {
        void onSocketMessageReceived(String message);
    }

    void addMessageListener(MessageListener messageListener);

    void sendMessage(String format, Object...arguments);

    int getMessageCount();

    String getMessage();

    int getNextMessageId();

}
