import java.io.BufferedReader;
import java.io.IOException;

import com.stagnationlab.c8y.driver.services.TextToSpeech;

public class Main {

    public static void main(String[] args) throws Exception {
        (new Main()).run();
    }

    private void run() throws Exception {
        System.out.printf("testing text to speech%n");

	    TextToSpeech.INSTANCE.speak("main thread first message");
	    //TextToSpeech.INSTANCE.speak("main thread second message", true);


		Thread.sleep(2000);
		//Thread.sleep(10);
		TextToSpeech.INSTANCE.speak("main thread second message");

        new Thread(() -> {
	        TextToSpeech.INSTANCE.speak("second thread first message", true);
	        TextToSpeech.INSTANCE.speak("second thread second message");
        }).start();

	    //TextToSpeech.INSTANCE.speak("main thread second message");

        /*
        TextToSpeech.INSTANCE.speak("second message that will be interrupted by the third message");
        Thread.sleep(4000);
	    TextToSpeech.INSTANCE.speak("this message overrides previous messages", true);
	    */

	    System.out.printf("continue program%n");
    }

    private static String askFor(String question, String defaultValue, BufferedReader consoleIn) throws IOException {
        System.out.printf("@ %s (%s): ", question, defaultValue);

        String userInput = consoleIn.readLine();

        if (userInput.length() == 0) {
            return defaultValue;
        }

        return userInput;
    }
}
