#include "Si7021.h"

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
    printf("measure - READ_RH\n");

    if(i2c.write(ADDR, (char*)tx_buff, 1) != 0) {
      printf("fail\n");
      return 0;
    }
    printf("write tx_buff\n");
    if(i2c.read(ADDR, (char*)rx_buff, 2) != 0) return 0;
    printf("read rx_buff\n");

    rhData = ((uint32_t)rx_buff[0] << 8) + (rx_buff[1] & 0xFC);
    rhData = (((rhData) * 15625L) >> 13) - 6000;
    printf("rhdata: %lu \n", rhData);

    tx_buff[0] = READ_TEMP;
    if(i2c.write(ADDR, (char*)tx_buff, 1) != 0) return 0;
    if(i2c.read(ADDR, (char*)rx_buff, 2) != 0) return 0;
    printf("temp tx ja rx done\n");
    tData = ((uint32_t)rx_buff[0] << 8) + (rx_buff[1] & 0xFC);
    tData = (((tData) * 21965L) >> 13) - 46850;
    printf("tData: %lu\n", tData);

    return 1;
}

bool Si7021::check()
{
    tx_buff[0] = READ_ID2_1;
    tx_buff[1] = READ_ID2_2;
    printf("check\n");
    if(i2c.write(ADDR, (char*)tx_buff, 2) != 0) return 0;
    if(i2c.read(ADDR, (char*)rx_buff, 8) != 0) return 0;
    printf("korras\n");

    if(rx_buff[0] == DEVICE_ID)
        return true;
    else return 0;
}
