/*
  MPL115A2.cpp - MPL115A2 library
  Created on:	23 Apr 2014
		  by:	Jose
*/

#if(ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <I2C.h>
#include "MPL115A2.h"


MPL115A2::MPL115A2(){

}


///////////////////////////////// Public Methods ////////////////////////////////////////


void MPL115A2::begin(){
	int16_t a0coef, b1coef, b2coef, c12coef;
  
	I2c.begin();                                            // Start I2C protocol.
	I2c.write(MPL115A2_devAd, MPL115A2_read, 0x00);               // Send Read command to MPL115A2.
	I2c.read(MPL115A2_devAd, a0_MSB, 8);                          // Reads the calibration coefficients stored in register 0x04 up to 0x0B.      
	a0coef = (I2c.receive() << 8) | I2c.receive();          // Collects [a0_MSB a0_LSB], 16-bits.
	b1coef = (I2c.receive() << 8) | I2c.receive();          // Collects [b1_MSB b1_LSB], 16-bits.
	b2coef = (I2c.receive() << 8) | I2c.receive();          // Collects [b2_MSB b2_LSB], 16-bits.
	c12coef = ((I2c.receive() << 8) | I2c.receive()) >> 2;  // Collects [c12_MSB c12_LSB], 14-bits. 
	
	a0 = (float)a0coef / 8;          // /(2^3) Convert it to 10-bit decimal coefficient values.
	b1 = (float)b1coef / 8192;       // /(2^13)Convert it to 10-bit decimal coefficient values.
	b2 = (float)b2coef / 16384;      // /(2^14)Convert it to 10-bit decimal coefficient values.
	c12 = (float)c12coef / 4194304;  // /(2^22)Convert it to 10-bit decimal coefficient values.	
	
/* 	Serial.println("+***************************************************************+");  
	Serial.println("              MPL115A2 I2C DIGITAL BAROMETER Test");
	Serial.println("  These are the factory calibration coefficients of the device:");
	Serial.print("  a0 = "); Serial.print(a0coef);
	Serial.print("  b1 = "); Serial.print(b1coef);
	Serial.print("  b2 = "); Serial.print(b2coef);
	Serial.print("  c12 = "); Serial.println(c12coef);
	Serial.println("+***************************************************************+"); */

}

void MPL115A2::readBoth(float *Pressure, float *Temperature){
	uint16_t Tadc, Padc;
	float Pcomp;
	
	// **************Start Conversion********************
	I2c.write(MPL115A2_devAd, MPL115A2_write, 0x00); // Send Write command.
	I2c.write(MPL115A2_devAd, CONVERT, 0x00);        // Activate conversion.
	delay(5); // Give time for conversion, 5ms.
	// **************Finished Conversion********************

	I2c.write(MPL115A2_devAd, MPL115A2_read, 0x00); // Send Read command.
	I2c.read(MPL115A2_devAd, 0x00, 4); // Start reading stored value of pressure and temperature in register 0x00 up to 0x03.       
	Padc = (I2c.receive() << 8); // Padc_MSB (in 0x00)
	Padc |= I2c.receive();       // Padc_LSB (in 0x01), combines the 2 bytes to form 16-bits length value as per data-sheet.
	Tadc = (I2c.receive() << 8); // Tadc_MSB (in 0x02)
	Tadc |= I2c.receive();       // Tadc_LSB (in 0x03), combines the 2 bytes to form 16-bits length value as per data-sheet.
	Padc = Padc >> 6;            // 6-bits shift-right to achieve 10-bit length as per data-sheet.
	Tadc = Tadc >> 6;            // 6-bits shift-right to achieve 10-bit length as per data-sheet.

	Pcomp = a0+(b1+c12*Tadc)*Padc+(b2*Tadc);         // Pressure compensation based on equation stated in the data-sheet.
	*Pressure = (Pcomp*((115.0-50.0)/1023.0))+50.0;   // kPa, Conversion from ADC count to Decimal.
	*Temperature = 25.0+(((float)Tadc-498.0)/-5.35);  // Celsius, 498 ADC counts = 25°C, -5.35 counts per °C

}

float MPL115A2::readTemperature() {
  float     retpressure, rettemp;
  readBoth(&retpressure, &rettemp);
  return rettemp;
}

float MPL115A2::readPressure() {
  float     retpressure, rettemp;
  readBoth(&retpressure, &rettemp);
  return retpressure;
}

