# Cumulocity Java Driver
**Basic gradle-based Cumulocity IoT platform Java driver.**

Connects to the Cumulocity platform and provides a simple operation.

## Tasks
- ```gradle clean build``` - build the application
- ```gradle jar``` - generates the driver jar in ```build/lib```
- ```gradle deploy``` - generates the driver jar in ```build/lib``` and copies it to the agent lib directory

## Testing through API
**`/devicecontrol/operations/`**
```json
{
  "deviceId" : "11467740",
  "com_stagnationlab_c8y_driver_operations_SetChannelValue": {
      "channel": 0,
      "value": 0.1
  },
  "description": "Update channel value"
}
```