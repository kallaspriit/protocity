#include "Application.hpp"

Application application = Application(8080);

void setup() {
    application.setup();
}

void loop() {
    application.loop();
}
