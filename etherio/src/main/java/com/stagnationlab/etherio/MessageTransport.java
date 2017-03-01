package com.stagnationlab.etherio;

@SuppressWarnings({"WeakerAccess", "unused"})
public interface MessageTransport {

    interface MessageListener {
        void onSocketConnecting(boolean isReconnecting);
        void onSocketOpen();
        void onSocketClose();
        void onSocketMessageReceived(String message);
        void onSocketConnectionFailed(Exception e);
    }

    void addMessageListener(MessageListener messageListener);

    boolean sendMessage(String format, Object...arguments);

    int getMessageCount();

    String getMessage();

    int getNextMessageId();

}
