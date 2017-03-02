package com.stagnationlab.etherio;

@SuppressWarnings({"WeakerAccess", "unused"})
public interface MessageTransport {

    interface EventListener {
        default void onConnecting(boolean isReconnecting) {}
	    default void onOpen() {}
	    default void onClose() {}
	    default void onMessageReceived(String message) {}
	    default void onConnectionFailed(Exception e) {}
    }

    void addEventListener(EventListener eventListener);

    boolean sendMessage(String format, Object...arguments);

    int getMessageCount();

    String getMessage();

    int getNextMessageId();

    boolean isConnected();

}
