/*************************************************** 
  This is a library for the Adafruit TTL JPEG Camera (VC0706 chipset)

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/397

  These displays use Serial to communicate, 2 pins are required to interface

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution

 ****************************************************

  This is maple port of library. Possibly might support Arduino Due
  as well if you change #include "WProgram.h" to #include "Arduino.h"
  in this file.

  Ported by Oskari Rauta.

 ****************************************************/


#include "WProgram.h"

#define VC0706_RESET  0x26
#define VC0706_GEN_VERSION 0x11
#define VC0706_READ_FBUF 0x32
#define VC0706_GET_FBUF_LEN 0x34
#define VC0706_FBUF_CTRL 0x36
#define VC0706_DOWNSIZE_CTRL 0x54
#define VC0706_DOWNSIZE_STATUS 0x55
#define VC0706_READ_DATA 0x30
#define VC0706_WRITE_DATA 0x31
#define VC0706_COMM_MOTION_CTRL 0x37
#define VC0706_COMM_MOTION_STATUS 0x38
#define VC0706_COMM_MOTION_DETECTED 0x39
#define VC0706_MOTION_CTRL 0x42
#define VC0706_MOTION_STATUS 0x43
#define VC0706_TVOUT_CTRL 0x44
#define VC0706_OSD_ADD_CHAR 0x45

#define VC0706_STOPCURRENTFRAME 0x0
#define VC0706_STOPNEXTFRAME 0x1
#define VC0706_RESUMEFRAME 0x3
#define VC0706_STEPFRAME 0x2

#define VC0706_640x480 0x00
#define VC0706_320x240 0x11
#define VC0706_160x120 0x22

#define VC0706_MOTIONCONTROL 0x0
#define VC0706_UARTMOTION 0x01
#define VC0706_ACTIVATEMOTION 0x01

#define VC0706_SET_ZOOM 0x52
#define VC0706_GET_ZOOM 0x53

#define CAMERABUFFSIZ 100
#define CAMERADELAY 10


class Adafruit_VC0706 {
 public:
  Adafruit_VC0706(HardwareSerial *ser); // Constructor when using HardwareSerial
  boolean begin(uint16 baud = 38400);
  boolean reset(void);
  boolean TVon(void);
  boolean TVoff(void);
  boolean takePicture(void);
  uint8 *readPicture(uint8 n);
  boolean resumeVideo(void);
  uint32 frameLength(void);
  char *getVersion(void);
  uint8 available();
  uint8 getDownsize(void);
  boolean setDownsize(uint8);
  uint8 getImageSize();
  boolean setImageSize(uint8);
  boolean getMotionDetect();
  uint8 getMotionStatus(uint8);
  boolean motionDetected();
  boolean setMotionDetect(boolean f);
  boolean setMotionStatus(uint8 x, uint8 d1, uint8 d2);
  boolean cameraFrameBuffCtrl(uint8 command);
  uint8 getCompression();
  boolean setCompression(uint8 c);
  
  boolean getPTZ(uint16 &w, uint16 &h, uint16 &wz, uint16 &hz, uint16 &pan, uint16 &tilt);
  boolean setPTZ(uint16 wz, uint16 hz, uint16 pan, uint16 tilt);

  void OSD(uint8 x, uint8 y, char *s); // isnt supported by the chip :(
  
 private:
  uint8  serialNum;
  uint8  camerabuff[CAMERABUFFSIZ+1];
  uint8  bufferLen;
  uint16 frameptr;
  HardwareSerial *hwSerial;

  void common_init(void);
  boolean runCommand(uint8 cmd, uint8 args[], uint8 argn, uint8 resp, boolean flushflag = true); 
  void sendCommand(uint8 cmd, uint8 args[], uint8 argn); 
  uint8 readResponse(uint8 numbytes, uint8 timeout);
  boolean verifyResponse(uint8 command);
  void printBuff(void);
};

        
