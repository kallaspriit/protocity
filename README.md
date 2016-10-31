# ProtoCity
Lego IoT smart city demo.

## Status LEDs
- **LED1** - Breathes if the firmware is operating normally.
- **LED2** - Blinks once if a command is received.
- **LED3** - Blinks fast while connecting to ethernet, blinks slow while waiting for socket connection, on when socket connection is established, off if ethernet connection failed.
- Flash like police sirens if a hard fault condition is detected in the code or by the platform (such as out-of-memory).

## Changelog
**31.10.2016**
- Added changelog and todo list.

**28.10.2016**
- Improved command handling and response, fixed compilation warning.
- Improved command handling.
- Removed json library and implemented custom command parser.
- Implemented basis for command callbacks.
- Implemented dynamic command handling.

**27.10.2016**
- Refactored the code to use as little dynamic memory allocation as possible.
- Improved memory handling.

**26.10.2016**
- Implemented basic ethernet connectivity.
- Improved debugging.
- Implemented basic TCP socket server support.
- Implemented socket server message listener logic.
- Implemented socket server events and debug led behaviour.
- Updated readme about status LEDs.

**25.10.2016**
- Implemented flashing debug LED when a command is received.
- Implemented debug class with breathing LED.
- Refactored to object-oriented codebase.
- Refactored picojson to be a built as a library.
- Initial project setup.

## TODO
- ✔ Implement basic ethernet connectivity.
- ✘ Implement id-based request-response model.
- ✘ Implement response through response object.
