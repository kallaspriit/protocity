# ProtoCity
Lego IoT smart city demo.

## Status LEDs
- **LED1** - Breathes if the firmware is operating normally.
- **LED2** - Blinks once if a command is received.
- **LED3** - Blinks fast while connecting to ethernet, blinks slow while waiting for socket connection, on when socket connection is established, off if ethernet connection failed.
- Flash like police sirens if a hard fault condition is detected in the code or by the platform (such as out-of-memory).

## Communication
The device can be interfaced over serial and over ethernet, the protocol is the same.

The basic communication protocol has the following properties:
- Commands end with a newline '\n' character
- Commands have the format of `REQUEST_ID:NAME:arg1:argN`
- Request tokens are separated with a colon ':' character
- For example consider command `1:led:3:ON`
  - Request id is "1"
  - Command name is "led"
  - First parameter, the led index, is "3"
  - Second parameter, the led state, is "ON"
- First token REQUEST_ID is a positive integer
- Second token NAME is a string
- There can be up to 8 additional parameters
- The application responds with `REQUEST_ID:STATUS:arg1:argN`
- The REQUEST_ID matches initial request id
- The STATUS is either "OK" or "ERROR"
- There may be up to 8 additional parameters
- For example the response might be
  - `1:OK`
  - `2:OK:3452` (where 3452 is the number of ram memory remaining etc)
  - `3:ERROR:Invalid parameters provided` (request failed, reason is provided)
- Application send back additional logging information over serial, these always start with a "#" and can be ignored on the receiving side

## Protocol
The following commands are supported (using "1" as example request id)

### Debugging
- `1:memory` - get available ram in bytes
  - responds with available memory left in bytes (for example `1:OK:2625`)
- `1:led:INDEX:MODE` - sets debug LED mode
  - for example call with `1:led:3:BLINK_FAST`
  - responds with OK if successful (for example `1:OK`)
  - parameters
    - **INDEX** should be an integer between 0 and 3
    - **MODE** should be a string value in the set *ON*, *OFF*, *BLINK_SLOW*, *BLINK_FAST*, *BLINK_ONCE*, *BREATHE*

### Ports
- `1:port:PORT_NUMBER:mode:MODE` - sets port mode
  - for example call with `1:port:1:mode:DIGITAL_OUT`
  - responds with OK if successful (for example `1:OK`)
  - responds with ERROR if invalid port or mode is requested
  - parameters
    - **PORT_NUMBER** port number
    - **MODE** should be a string value in the set *DIGITAL_IN*, *DIGITAL_OUT*, *ANALOG_OUT*, *INTERRUPT*, *ANALOG_IN*
- `1:port:PORT_NUMBER:pull:MODE` - sets pull mode
  - for example call with `1:port:1:pull:UP`
  - responds with OK if successful (for example `1:OK`)
  - responds with ERROR if invalid port or mode is requested
  - parameters
    - **PORT_NUMBER** port number
    - **MODE** should be a string value in the set *UP*, *DOWN*, *NONE*
- `1:port:PORT_NUMBER:value:VALUE` - sets port value for *DIGITAL_OUT* and *ANALOG_OUT* modes
  - for example call with `1:port:1:value:HIGH` or `1:port:1:value:0.5`
  - responds with OK if successful (for example `1:OK`)
  - responds with ERROR if invalid port or value is requested
  - parameters
    - **PORT_NUMBER** port number
    - **VALUE** should be either *HIGH* or *LOW* for *DIGITAL_OUT* mode or a float value between 0..1 for *ANALOG_OUT* mode
- `1:port:PORT_NUMBER:read` - reads current *DIGITAL_IN*, *INTERRUPT* or *ANALOG_IN* port value
  - for example call with `1:port:1:read`
  - responds with OK and value if successful (for example `1:OK:HIGH` or `1:OK:0.5`)
  - responds with ERROR if invalid port is requested
  - parameters
    - **PORT_NUMBER** port number
- `1:port:PORT_NUMBER:listen:THRESHOLD:INTERVAL` - sets up *ANALOG_IN* port change listener
  - for example call with `1:port:1:listen:0.1:100` - change events are sent for every 0.1 value change no more often than every 100 milliseconds
  - responds with OK if successful (for example `1:OK`)
  - responds with ERROR if invalid port is requested
  - parameters
    - **THRESHOLD** value threshold 0..1 (optional, defaults to 0.01)
    - **INTERVAL** minimum change reporting interval in milliseconds (optional, defaults to 0)
- `1:port:PORT_NUMBER:listen:off` - stops *ANALOG_IN* port change listener
  - for example call with `1:port:1:listen:off`
  - responds with OK if successful (for example `1:OK`)
  - responds with ERROR if invalid port is requested

## Changelog
**28.11.2016**
- Implemented heartbeat and closing socket connection if sending a message fails.

**21.11.2016**
- Improved listen command supporting variable number of arguments.

**07.11.2016**
- Implemented registering and stopping analog port change listeners.

**04.11.2016**
- Implemented analog port change listener and reporting.

**03.11.2016**
- Implemented analog input port.
- Implemented input port pullup/pulldown functionality.
- Refactored port controller naming.
- Implemented digital port interrupt events.
- Implemented digital port value reading.

**02.11.2016**
- Implemented digital port value command for DIGITAL_OUT and ANALOG_OUT modes.
- Implemented digital port mode action.
- Improved digital port controller logic.
- Implemented test setup and loop.

**01.11.2016**
- Documented communication logic and protocol.
- Implemented command response data logic.

**31.10.2016**
- Improved command response logic.
- Implemented request source handling.
- Implemented command request id parsing.
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
