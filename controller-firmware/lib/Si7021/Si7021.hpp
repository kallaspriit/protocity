#ifndef SI7021_H
#define SI7021_H

#include "mbed.h"

/** Si7012 Read Temperature Command */
#define READ_TEMP        0xE0 /* Read previous T data from RH measurement command*/
/** Si7012 Read RH Command */
#define READ_RH          0xE5 /* Perform RH (and T) measurement. */

/** Si7012 Read ID */
#define READ_ID1_1       0xFA
#define READ_ID1_2       0x0F
#define READ_ID2_1       0xFC
#define READ_ID2_2       0xC9

/** Si7012 Read Firmware Revision */
#define READ_FWREV_1     0x84
#define READ_FWREV_2     0xB8

/** I2C device address for Si7021 */
#define ADDR    0x80
//#define ADDR    0x40

/** I2C device frequency for Si7021 */
#define FREQ    100000
//#define FREQ    400000
//#define FREQ    10000

/** Device ID value for Si7021 */
#define DEVICE_ID 0x15

class Si7021
{
public:
    Si7021(PinName sda, PinName scl);
    ~Si7021();

    /*
     * Get last measured temperature data
     * return: int32_t = temperature in millidegrees centigrade
     */
    int32_t get_temperature();

    /*
     * Get last measured relative humidity data
     * return: uint32_t = relative humidity value in milli-percent
     */
    uint32_t get_humidity();

    /*
     * Perform measurement.
     * Asynchronous callback can be provided (type void (*)(void)).
     * return: 0 if successful, else one of the defined error codes.
     */
    bool measure();

    /*
     * Check if the sensor is active and responding. This will update the get_active value.
     * Asynchronous callback can be provided (type void (*)(void)).
     * return: 0 if successful, else one of the defined error codes.
     */
    bool check();

private:
    I2C i2c;

    uint8_t  rx_buff[8];
    uint8_t  tx_buff[2];

    uint32_t rhData;
    int32_t  tData;
};

#endif
