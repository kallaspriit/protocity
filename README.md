# ProtoCity

**Prototype lego smart city implementation on Cumulocity.**

## Projects and building
- `driver` - Java Cumulocity gateway driver
  - `gradle jar` - builds the driver .jar file in builds/libs/cumulocity-driver-1.0.0.jar
  - `gradle deploy` - builds the driver jar file and copies it to the cumulocity agent libs folder (might require changing the gradle config)
- `etherio` - Java library for communicating with the firmware
  - `gradle jar` - builds the library .jar file in builds/libs/etherio-1.0.0.jar, copy it to driver "libs" folder and cumulocity agent "lib" foldeer
- `firmware` - C++ library for the mbed platform for controlling various IO and sensors over ethernet
  - Build and deploy using platformio
- `etherio-test` - Java project for testing the EtherIO library
  - `gradle jar` - builds the test
  - `run` - runs the test
- `socket-terminal` - Java project for talking to an EtherIO device over ethernet
  - `gradle jar` - builds the terminal
  - `run` - runs the terminal
- `lego` - Lego city physical custom components
