# ProtoCity

**Prototype lego smart city implementation on Cumulocity.**

## Development environment
- setup cumulocity agent (C:/cumulocity-agent etc)
- create environment variable called CUMULOCITY_AGENT_HOME that points to cumulocity agent root directory (C:/cumulocity-agent etc)

## Projects and building
- `cumulocity-driver` - Java Cumulocity gateway driver
  - `gradle build` - builds the driver .jar file in builds/libs/cumulocity-driver-1.0.0.jar
  - `gradle deploy` - builds the driver jar file and copies it as well as its dependencies to the cumulocity agent libs folder (might require changing the gradle config)
- `etherio` - Java library for communicating with the firmware
  - `gradle build` - builds the library .jar file in builds/libs/etherio-1.0.0.jar, copy it to driver "libs" folder and cumulocity agent "lib" folder
  - `gradle deploy` - builds the library .jar file in builds/libs/etherio-1.0.0.jar and copies it to the driver ../cumulocity-driver/libs folder
- `controller-firmware` - C++ library for the mbed platform for controlling various IO and sensors over ethernet
  - Build and deploy using platformio
- `experiments/etherio-test` - Java project for testing the EtherIO library
  - `gradle jar` - builds the test
  - `run` - runs the test
- `socket-terminal` - Java project for talking to an EtherIO device over ethernet
  - `gradle jar` - builds the terminal
  - `run` - runs the terminal
- `lego` - Lego city physical custom components
