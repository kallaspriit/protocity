#include "Si7021.hpp"

Si7021::Si7021(PinName sda, PinName scl):i2c(sda, scl)
{
    i2c.frequency(FREQ);
}

Si7021::~Si7021()
{

}

int32_t Si7021::get_temperature()
{
    return tData;
}

uint32_t Si7021::get_humidity()
{
    return rhData;
}

bool Si7021::measure()
{
    tx_buff[0] = READ_RH;

    if (i2c.write(ADDR, (char*)tx_buff, 1) != 0) {
      // printf("# Si7021: failed to write humidity measurement command\n");

      return false;
    }

    wait(0.5f);

    if (i2c.read(ADDR, (char*)rx_buff, 2) != 0) {
        // printf("# Si7021: failed to read humidity measurement response\n");

        return false;
    }

    rhData = ((uint32_t)rx_buff[0] << 8) + (rx_buff[1] & 0xFC);
    rhData = (((rhData) * 15625L) >> 13) - 6000;

    /*
    tx_buff[0] = READ_TEMP;

    if (i2c.write(ADDR, (char*)tx_buff, 1) != 0) {
        printf("# Si7021: failed to write temperature measurement command\n");

        return false;
    }

    if (i2c.read(ADDR, (char*)rx_buff, 2) != 0) {
        printf("# Si7021: failed to read temperature measurement response\n");

        return false;
    }

    tData = ((uint32_t)rx_buff[0] << 8) + (rx_buff[1] & 0xFC);
    tData = (((tData) * 21965L) >> 13) - 46850;
    */

    return true;
}

bool Si7021::check()
{
    tx_buff[0] = READ_ID2_1;
    tx_buff[1] = READ_ID2_2;

    if (i2c.write(ADDR, (char*)tx_buff, 2) != 0) {
        // printf("# Si7021: failed to write check command\n");

        return false;
    }

    if(i2c.read(ADDR, (char*)rx_buff, 8) != 0) {
        // printf("# Si7021: failed to read check command response\n");

        return false;
    }

    if(rx_buff[0] != DEVICE_ID) {
        // printf("# Si7021: invalid device id %d received (expected %d)\n", rx_buff[0], DEVICE_ID);

        return false;
    }

    // printf("# Si7021: check complete, found device 0x%x\n", rx_buff[0]);

    return true;
}
