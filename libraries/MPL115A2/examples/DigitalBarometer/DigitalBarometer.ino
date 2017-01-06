/*******************************************
 MPL115A2 I2C DIGITAL BAROMETER
 - This is a sample code for MPL115A2 Barometer 
   utilizing MPL115A2.h library.
 - The output of this IC are Pressure(in kPa)
   and Temperature(in Celsius).
  
 MPL115A2 Functions:
    readBoth(&P, &T)   - Simultaneously read pressure(P)
                       and temperature(T) from the chip.
    readPressure()     - returns the pressure reading from the chip.
    readTemperature()  - returns the temperature reading from the chip.
    *Both returned values are floating points.
    
 Wiring Connection:
   MPL115A2 BREAKOUT BOARD	GIZDUINO          
           VCC      <------------> VCC                  
           GND      <------------> GND                  
           CS/RESET <------------> A2                   
           SHDN     <------------> A3                   
           DIN/SDA  <------------> A4 <--- 4.7Kohms ---> VCC
           SCLK/SCL <------------> A5 <--- 4.7Kohms ---> VCC  
 
 Created On: 23 Apr 2014
         By: e-Gizmo Mechatronix Central

 *******************************************/
#include <I2C.h>
#include "MPL115A2.h"

#define ResetBar A2
#define SHDNBar A3

MPL115A2 m2;
float Pressure_kPa, Temperature_Celsius, Pressure_kPa2, Temperature_Celsius2;

void setup()
{
  Serial.begin(9600);

  pinMode(ResetBar,OUTPUT);
  pinMode(SHDNBar,OUTPUT);
  delay(1);
  analogWrite(ResetBar, 255);   // Set MPL115A2 RST'(CS in Barometer breakout board) pin HIGH to Enable I2C Communication.
  analogWrite(SHDNBar, 255);    // Set MPL115A2 SHDN'(SHDN in Barometer breakout board) pin HIGH for normal operation.
  
  m2.begin();                   // Starts MPL115A2 Chip.
}

void loop()
{
  Pressure_kPa = m2.readPressure();
  Temperature_Celsius = m2.readTemperature();
  Serial.print("Pressure = "); Serial.print(Pressure_kPa, 2); Serial.println(" kPa");
  Serial.print("Temperature = "); Serial.print(Temperature_Celsius, 2); Serial.println(" C");
  m2.readBoth(&Pressure_kPa2, &Temperature_Celsius2);
  Serial.print("Simultaneously: P = "); Serial.print(Pressure_kPa2, 2); Serial.print("kPa T = ");Serial.print(Temperature_Celsius, 2); Serial.println("C");
  delay(1000);
}

