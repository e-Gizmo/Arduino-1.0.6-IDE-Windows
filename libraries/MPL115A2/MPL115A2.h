/*
  MPL115A2.h   - MPL115A2 library
*/
#ifndef MPL115A2_h
#define MPL115A2_h

#if(ARDUINO >= 100)
	#include <Arduino.h>
#else
	#include <WProgram.h>
#endif

#include <I2C.h>

#define MPL115A2_devAd 0x60
#define MPL115A2_write 0xC0 
#define MPL115A2_read 0xC1
#define CONVERT 0x12
#define Padc_MSB 0x00
#define a0_MSB 0x04

class MPL115A2
{
  public:
    MPL115A2();
    void begin();
	void readBoth(float *Pressure, float *Temperature);
	float readPressure();
	float readTemperature();

  private:
	float a0;
	float b1;
	float b2;
	float c12;
};
#endif

