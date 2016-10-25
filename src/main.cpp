#include "mbed.h"
#include "rtos.h"

#include <string>
using namespace std;

#include <picojson.h>

// serials
Serial serial(USBTX, USBRX);

// leds
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

// threads
Thread blinkThread;
Thread serialRxNotifierThread;
Thread serialTxNotifierThread;

// runtime variables
string command = "";

// signals
const int32_t SIGNAL_SERIAL_RX = 1;
const int32_t SIGNAL_SERIAL_TX = 2;

// configuration
const int SERIAL_LED_BLINK_DURATION_MS = 50;

void runBlink() {
    while (true) {
        led2 = !led2;
		Thread::wait(500);

		// serial.printf("> updated led2 state: %d\n", led1.read());
    }
}

void runSerialRxNotifier() {
	while (true) {
		Thread::signal_wait(SIGNAL_SERIAL_RX);

		led3 = 1;
		Thread::wait(SERIAL_LED_BLINK_DURATION_MS);
		led3 = 0;
	}
}

void runSerialTxNotifier() {
	while (true) {
		Thread::signal_wait(SIGNAL_SERIAL_TX);

		led4 = 1;
		Thread::wait(SERIAL_LED_BLINK_DURATION_MS);
		led4 = 0;
	}
}

bool isJsonCommand(string command) {
	return command.size() >= 2 && command[0] == '{' && command[command.size() - 1] == '}';
}

void handleJsonCommand(string command) {
	serial.printf("> got JSON command: '%s'\n", command.c_str());
}

void handleStringCommand(string command) {
	serial.printf("> got string command: '%s'\n", command.c_str());
}

void handleCommand(string command) {
	if (isJsonCommand(command)) {
		handleJsonCommand(command);
	} else {
		handleStringCommand(command);
	}
}

void handleSerialRx() {
	serialRxNotifierThread.signal_set(SIGNAL_SERIAL_RX);

	char receivedChar = serial.getc();

	if (receivedChar == '\n') {
		handleCommand(command);
		command = "";
	} else {
		command += receivedChar;
	}
}

void handleSerialTx() {
	serialTxNotifierThread.signal_set(SIGNAL_SERIAL_TX);
}

int main() {
	// configure serial
	serial.baud(115200);
	serial.attach(&handleSerialRx, Serial::RxIrq);
	serial.attach(&handleSerialTx, Serial::TxIrq);

	// start threads
	blinkThread.start(&runBlink);
	serialRxNotifierThread.start(&runSerialRxNotifier);
	serialTxNotifierThread.start(&runSerialTxNotifier);

    while (true) {
        led1 = !led1;
		Thread::wait(2000);

		// serial.printf("> updated led1 state: %d\n", led1.read());
    }
}

/*
void parse() {
    picojson::value v;
    const char *json = "{\"string\": \"it works\", \"number\": 3.14, \"integer\":5}";

    string err = picojson::parse(v, json, json + strlen(json));
    printf("res error? %s\r\n", err.c_str());
    printf("string =%s\r\n" ,  v.get("string").get<string>().c_str());
    printf("number =%f\r\n" ,  v.get("number").get<double>());
    printf("integer =%d\r\n" ,  (int)v.get("integer").get<double>());
}

void serialize() {
    picojson::object v;
    picojson::object inner;
    string val = "tt";

    v["aa"] = picojson::value(val);
    v["bb"] = picojson::value(1.66);
    inner["test"] =  picojson::value(true);
    inner["integer"] =  picojson::value(1.0);
    v["inner"] =  picojson::value(inner);

    string str = picojson::value(v).serialize();
    printf("serialized content = %s\r\n" ,  str.c_str());
}

void advanced() {
    picojson::value v;
    const char *jsonsoure =
        "{\"menu\": {"
        "\"id\": \"f\","
        "\"popup\": {"
        "  \"menuitem\": ["
        "    {\"v\": \"0\"},"
        "    {\"v\": \"1\"},"
        "    {\"v\": \"2\"}"
        "   ]"
        "  }"
        "}"
        "}";
    char * json = (char*) malloc(strlen(jsonsoure)+1);
    strcpy(json, jsonsoure);
    string err = picojson::parse(v, json, json + strlen(json));
    printf("res error? %s\r\n", err.c_str());

    printf("id =%s\r\n", v.get("menu").get("id").get<string>().c_str());

    picojson::array list = v.get("menu").get("popup").get("menuitem").get<picojson::array>();

    for (picojson::array::iterator iter = list.begin(); iter != list.end(); ++iter) {
        printf("menu item value =%s\r\n", (*iter).get("v").get<string>().c_str());
    }
}

int main() {
    printf("Starting PicoJSONSample "__TIME__"\r\n");
    printf(">>> parsing \r\n");
    parse();
    printf(">>> serializing \r\n");
    serialize();
    printf(">>> advanced parsing \r\n");
    advanced();
    printf("Ending PicoJSONSample " __TIME__ "\r\n");
}
*/
