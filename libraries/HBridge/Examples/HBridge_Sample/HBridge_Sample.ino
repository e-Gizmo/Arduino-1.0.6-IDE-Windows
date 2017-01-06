// Library by Dan, e-Gizmo Mechatronix Central
// The library is made to simplify HBridge control
// and pwm frequency setting

/*
Available commands:
 HBridge variable(wd_pin,pwm_pin,dir_pin);  // Assigns pin
 variable.begin();                          // Sets wd pin
 variable.Forward(speed);                   // Forward
 variable.Backward(speed);                  // Backward
 variable.Stop();                           // Stop
 */

#include <HBridge.h>

HBridge motors(11,6,8);

void setup()
{
  motors.begin();
}

void loop()
{
  motors.Forward(255);
  delay(1000);
  motors.Backward(255);
  delay(1000);
  motors.Stop();
  delay(1000);
}


