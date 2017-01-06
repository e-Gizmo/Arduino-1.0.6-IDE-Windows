#include "Arduino.h"
#include "HBridge.h"
int divisor = 1024;
int wd_pin;
int dir_pin;
int pwm_pin;
HBridge::HBridge(int wd_pin, int pwm_pin, int dir_pin)
{
_wd_pin = wd_pin;
_dir_pin = dir_pin;
_pwm_pin = pwm_pin;
  pinMode(_wd_pin, OUTPUT);
  pinMode(_pwm_pin, OUTPUT);
  pinMode(_dir_pin, OUTPUT);

analogWrite(_wd_pin,10);

}

void HBridge::begin()
{
 byte mode;
  if(wd_pin == 5 || wd_pin == 6 || wd_pin == 9 || wd_pin == 10) {
    switch(divisor) {
    case 1: 
      mode = 0x01; 
      break;
    case 8: 
      mode = 0x02; 
      break;
    case 64: 
      mode = 0x03; 
      break;
    case 256: 
      mode = 0x04; 
      break;
    case 1024: 
      mode = 0x05; 
      break;
    default: 
      return;
    }
    if(wd_pin == 5 || wd_pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } 
    else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } 
  else if(wd_pin == 3 || wd_pin == 11) {
    switch(divisor) {
    case 1: 
      mode = 0x01; 
      break;
    case 8: 
      mode = 0x02; 
      break;
    case 32: 
      mode = 0x03; 
      break;
    case 64: 
      mode = 0x04; 
      break;
    case 128: 
      mode = 0x05; 
      break;
    case 256: 
      mode = 0x06; 
      break;
    case 1024: 
      mode = 0x7; 
      break;
    default: 
      return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

void HBridge::Forward(int speed_fwd)
{
    analogWrite(_pwm_pin,speed_fwd);
  digitalWrite(_dir_pin,HIGH);
}

void HBridge::Backward(int speed_bwd)
{
    analogWrite(_pwm_pin,speed_bwd);
  digitalWrite(_dir_pin,LOW);
}

void HBridge::Stop()
{
analogWrite(_pwm_pin,0);
}
