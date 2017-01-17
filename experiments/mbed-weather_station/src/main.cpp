#include "mbed.h"
#include "TMP102.h"
#include <MPL3115A2.hpp>
#include "uLCD_4DGL.h"
#include <TSL2561.hpp>
#include "Si7021.h"

//InterruptIn alarm(p8);
//DigitalOut led(LED1);
//DigitalOut led2(LED2);
DigitalOut flash(LED4);
DigitalOut kollane(p25);
DigitalOut punane(p26);

//InterruptIn sound(p14);
AnalogIn sound_env(p15);

Serial serial(USBTX, USBRX);

Si7021 humidity(p9, p10);
TMP102 temperature(p9, p10, 0x90);
MPL3115A2 pressure(p9, p10, 0x60 << 1);
uLCD_4DGL uLCD(p37,p31,p11);
TSL2561 luminosity(p9, p10, TSL2561_ADDR_FLOAT);



int main() {
    serial.baud(115200);
    wait(1);

    pressure.Oversample_Ratio(OVERSAMPLE_RATIO_32);
    //pressure.Altimeter_Mode();
    pressure.Barometric_Mode();

    humidity.check();

    //luminosity.setGain(TSL2561_GAIN_0X);
	  //luminosity.setTiming(TSL2561_INTEGRATIONTIME_402MS);

    uLCD.display_control(LANDSCAPE_R);
    uLCD.color(WHITE);
    uLCD.locate(5,1);
    uLCD.printf("ILMAJAAM\n\n");

    while(1) {
        flash = !flash;
        wait(0.3);

        humidity.check();
        humidity.measure();

        float sound_value = sound_env.read()*100.0f;
        float pressure_value = pressure.getPressure()/100.0f;
        int luminosity_value = luminosity.getLuminosity(TSL2561_VISIBLE);
        float temperature_value = temperature.read();
        float humidity_value = humidity.get_humidity();

        //prints into console
        printf("Temperature from TMP102: %f\n", temperature_value);
        printf("Pressure: %f hPa\n", pressure_value);
        //printf("Pressure: %f mmHg\n", pressure_value*0.00750061683);
        //printf("Altitude: %f\n", pressure.getAltimeter());
        printf("Lum: %i lux\n", luminosity_value);
        printf("Gain db: %3.1f\n", sound_value);
        printf("Humidity: %2f\n", humidity_value);

        //prints to the LCD
        uLCD.locate(1,3);
        uLCD.printf("Temp: %.2f °C\n", temperature_value);
        uLCD.printf("\n");
        uLCD.locate(1,5);
        uLCD.printf("Rohk: %.1f hPa\n", pressure_value);
        uLCD.printf("\n");
        uLCD.locate(1,7);
        //uLCD.printf("%.2f mmHg\n", pressure.getPressure()*0.00750061683);
        uLCD.printf("Valgus: %i lux\n", luminosity_value);
        uLCD.printf("\n");
        uLCD.locate(1,9);
        //uLCD.printf("Temp: %.2f °C\n", pressure.getTemperature());
        uLCD.printf("Niiskus: %3.1f %% \n", humidity_value/1000);
        uLCD.printf("\n");
        uLCD.locate(1,11);
        //uLCD.printf("Lum: %i lux\n", luminosity.getLuminosity(TSL2561_VISIBLE));
        uLCD.printf("Heli dB: %3.1f\n", sound_value);
        uLCD.printf("\n");
        uLCD.locate(5,13);


        //microfone
        if(sound_value <= 15){
          //uLCD.printf("Vaikne.\n");
          kollane = 0;
          punane = 0;
        }
        else if( (sound_value > 15) && ( sound_value <= 40) ){
          //uLCD.printf("Keskmine.\n");
          punane = 0;
          kollane = 1;
        }
        else if(sound_value > 40){
          //uLCD.printf("Vali.\n");
          kollane = 0;
          punane = 1;
        }


    }
}
