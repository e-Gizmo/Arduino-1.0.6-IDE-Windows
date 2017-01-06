/*
HMC5883L_Example.ino - Example sketch for integration with an HMC5883L triple axis magnetometer.
 Copyright (C) 2013 BluLemonLabs (bluelemonlabs.blogspot.com)
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the version 3 GNU General Public License as
 published by the Free Software Foundation.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

// Reference the I2C Library
#include <Wire.h>
// Reference the HMC5883L Compass Library
#include <HMC5883L.h>

// Store our compass as an object.
HMC5883L compass;

// Record any errors that may occur in the compass.
int error = 0;

// Out setup routine, here we will configure the microcontroller and compass.
void setup()
{
  // Initialize the serial port.
  Serial.begin(9600);

  Serial.println("Starting the I2C interface.");
  Wire.begin(); // Start the I2C interface.

  Serial.println("Constructing new HMC5883L");
  compass = HMC5883L(); // Construct a new HMC5883 compass.
  //The implementation of the class is provided in the library
  
  // Now we have an istance of the class!
  //Let's initializate it...
  
  Serial.println("Setting scale to +/- 1.3 Ga");
  error = compass.SetScale(1.3); // Set the scale of the compass to 1.3Ga
  if(error != 0){ // If there is an error, print it out. 
    Serial.println(compass.GetErrorText(error));
    error =0;
  }

  Serial.println("Setting measurement mode to continous.");
  error = compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement mode to Continuous
  if(error != 0) {// If there is an error, print it out.
    Serial.println(compass.GetErrorText(error)); //Todo: Error handling for this method in .h and .cpp
    error=0;
  }
}

// Our main program loop. 
void loop()
{
  // Retrieve the raw values from the magnetometer (not scaled).
  MagnetometerRaw raw = compass.ReadRawAxis();
  // Retrieve the scaled values from the magnetometer (scaled to the configured scale).
  MagnetometerScaled scaled = compass.ReadScaledAxis();

  // Values are accessed like so:
  int MilliGauss_OnThe_XAxis = scaled.XAxis;// (or YAxis, or ZAxis)

  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  // Atan2() automatically check the correct formula taking care of the quadrant you are in
  float heading = atan2(scaled.YAxis, scaled.XAxis);

  // Once you have your heading, you must then add your 'Declination Angle',
  // which is the 'Error' of the magnetic field in your location. Mine is 0.0404 
  // Find yours here: http://www.magnetic-declination.com/
  
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 0.0404;
  heading += declinationAngle;

  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;

  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;

  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 

  // Output the data via the serial port.
  Output(raw, scaled, heading, headingDegrees);

  // By default the HMC5883L reads the data 15 time per second (15Hz)
  // However since we have a long serial out (104ms at 9600) we will let
  // it run at its natural speed.
  // delay(66);
}

// Output the data down the serial port.
void Output(MagnetometerRaw raw, MagnetometerScaled scaled, float heading, float headingDegrees)
{
  Serial.print("Raw:\t");
  Serial.print(raw.XAxis);
  Serial.print("   ");   
  Serial.print(raw.YAxis);
  Serial.print("   ");   
  Serial.print(raw.ZAxis);
  Serial.print("   \tScaled:\t");

  Serial.print(scaled.XAxis);
  Serial.print("   ");   
  Serial.print(scaled.YAxis);
  Serial.print("   ");   
  Serial.print(scaled.ZAxis);

  Serial.print("   \tHeading:\t");
  Serial.print(heading);
  Serial.print(" Radians   \t");
  Serial.print(headingDegrees);
  Serial.println(" Degrees   \t");
}
