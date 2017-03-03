package com.stagnationlab.etherio;

@SuppressWarnings({"WeakerAccess", "unused"})
public interface MessageTransport {

    interface EventListener {
        default void onConnecting(boolean wasReconnected) {}
	    default void onOpen(boolean isReconnected) {}
	    default void onClose() {}
	    default void onMessageReceived(String message) {}
	    default void onConnectionFailed(Exception e, boolean wasEverOpened) {}
    }

    void addEventListener(EventListener eventListener);

    boolean sendMessage(String format, Object...arguments);

    int getMessageCount();

    String getMessage();

    int getNextMessageId();

    boolean isConnected();

}
