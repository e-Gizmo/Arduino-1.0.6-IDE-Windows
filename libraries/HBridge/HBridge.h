#ifndef HBridge_h
#define HBridge_h

#include "Arduino.h"

class HBridge
{
  public:
    HBridge(int wd_pin, int pwm_pin, int dir_pin);
    void Forward(int speed_fwd);
    void Backward(int speed_bwd);
   void Stop();
    void begin();

  private:
    int _wd_pin;
    int _pwm_pin;
    int _dir_pin;
    int _speed_fwd;
    int _speed_bwd;
};

#endif