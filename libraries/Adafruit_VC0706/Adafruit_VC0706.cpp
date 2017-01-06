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

 This is Maple port of library. Porting was done by Oskari Rauta.

 ****************************************************/


#include "Adafruit_VC0706.h"
#include <string.h>

// Initialization code used by all constructor types
void Adafruit_VC0706::common_init(void) {
  hwSerial  = NULL;
  frameptr  = 0;
  bufferLen = 0;
  serialNum = 0;
}

// Constructor when using HardwareSerial
Adafruit_VC0706::Adafruit_VC0706(HardwareSerial *ser) {
  common_init();  // Set everything to common state, then...
  hwSerial = ser; // ...override hwSerial with value passed.
}

boolean Adafruit_VC0706::begin(uint16 baud) {
  hwSerial->begin(baud);
  return reset();
}

boolean Adafruit_VC0706::reset() {
  uint8 args[] = {0x0};

  return runCommand(VC0706_RESET, args, 1, 5);
}

boolean Adafruit_VC0706::motionDetected() {
  if (readResponse(4, 200) != 4) {
    return false;
  }
  if (! verifyResponse(VC0706_COMM_MOTION_DETECTED))
    return false;

  return true;
}


uint8 Adafruit_VC0706::setMotionStatus(uint8 x, uint8 d1, uint8 d2) {
  uint8 args[] = {0x03, x, d1, d2};

  return runCommand(VC0706_MOTION_CTRL, args, sizeof(args), 5);
}


uint8 Adafruit_VC0706::getMotionStatus(uint8 x) {
  uint8 args[] = {0x01, x};

  return runCommand(VC0706_MOTION_STATUS, args, sizeof(args), 5);
}


boolean Adafruit_VC0706::setMotionDetect(boolean flag) {
  if (! setMotionStatus(VC0706_MOTIONCONTROL, 
			VC0706_UARTMOTION, VC0706_ACTIVATEMOTION))
    return false;

  uint8 args[] = {0x01, flag};
  
  runCommand(VC0706_COMM_MOTION_CTRL, args, sizeof(args), 5);
}



boolean Adafruit_VC0706::getMotionDetect(void) {
  uint8 args[] = {0x0};

  if (! runCommand(VC0706_COMM_MOTION_STATUS, args, 1, 6))
    return false;

  return camerabuff[5];
}

uint8 Adafruit_VC0706::getImageSize() {
  uint8 args[] = {0x4, 0x4, 0x1, 0x00, 0x19};
  if (! runCommand(VC0706_READ_DATA, args, sizeof(args), 6))
    return -1;

  return camerabuff[5];
}

boolean Adafruit_VC0706::setImageSize(uint8 x) {
  uint8 args[] = {0x05, 0x04, 0x01, 0x00, 0x19, x};

  return runCommand(VC0706_WRITE_DATA, args, sizeof(args), 5);
}

/****************** downsize image control */

uint8 Adafruit_VC0706::getDownsize(void) {
  uint8 args[] = {0x0};
  if (! runCommand(VC0706_DOWNSIZE_STATUS, args, 1, 6)) 
    return -1;

   return camerabuff[5];
}

boolean Adafruit_VC0706::setDownsize(uint8 newsize) {
  uint8 args[] = {0x01, newsize};

  return runCommand(VC0706_DOWNSIZE_CTRL, args, 2, 5);
}

/***************** other high level commands */

char * Adafruit_VC0706::getVersion(void) {
  uint8 args[] = {0x01};
  
  sendCommand(VC0706_GEN_VERSION, args, 1);
  // get reply
  if (!readResponse(CAMERABUFFSIZ, 200))
    return 0;
  camerabuff[bufferLen] = 0;  // end it!
  return (char *)camerabuff;  // return it!
}


/****************** high level photo comamnds */

void Adafruit_VC0706::OSD(uint8 x, uint8 y, char *str) {
  if (strlen(str) > 14) { str[13] = 0; }

  uint8 args[17] = {strlen(str), strlen(str)-1, (y & 0xF) | ((x & 0x3) << 4)};

  for (uint8 i=0; i<strlen(str); i++) {
    char c = str[i];
    if ((c >= '0') && (c <= '9')) {
      str[i] -= '0';
    } else if ((c >= 'A') && (c <= 'Z')) {
      str[i] -= 'A';
      str[i] += 10;
    } else if ((c >= 'a') && (c <= 'z')) {
      str[i] -= 'a';
      str[i] += 36;
    }

    args[3+i] = str[i];
  }

   runCommand(VC0706_OSD_ADD_CHAR, args, strlen(str)+3, 5);
   printBuff();
}

boolean Adafruit_VC0706::setCompression(uint8 c) {
  uint8 args[] = {0x5, 0x1, 0x1, 0x12, 0x04, c};
  return runCommand(VC0706_WRITE_DATA, args, sizeof(args), 5);
}

uint8 Adafruit_VC0706::getCompression(void) {
  uint8 args[] = {0x4, 0x1, 0x1, 0x12, 0x04};
  runCommand(VC0706_READ_DATA, args, sizeof(args), 6);
  printBuff();
  return camerabuff[5];
}

boolean Adafruit_VC0706::setPTZ(uint16 wz, uint16 hz, uint16 pan, uint16 tilt) {
  uint8 args[] = {0x08, wz >> 8, wz, 
		    hz >> 8, wz, 
		    pan>>8, pan, 
		    tilt>>8, tilt};

  return (! runCommand(VC0706_SET_ZOOM, args, sizeof(args), 5));
}


boolean Adafruit_VC0706::getPTZ(uint16 &w, uint16 &h, uint16 &wz, uint16 &hz, uint16 &pan, uint16 &tilt) {
  uint8 args[] = {0x0};
  
  if (! runCommand(VC0706_GET_ZOOM, args, sizeof(args), 16))
    return false;
  printBuff();

  w = camerabuff[5];
  w <<= 8;
  w |= camerabuff[6];

  h = camerabuff[7];
  h <<= 8;
  h |= camerabuff[8];

  wz = camerabuff[9];
  wz <<= 8;
  wz |= camerabuff[10];

  hz = camerabuff[11];
  hz <<= 8;
  hz |= camerabuff[12];

  pan = camerabuff[13];
  pan <<= 8;
  pan |= camerabuff[14];

  tilt = camerabuff[15];
  tilt <<= 8;
  tilt |= camerabuff[16];

  return true;
}


boolean Adafruit_VC0706::takePicture() {
  frameptr = 0;
  return cameraFrameBuffCtrl(VC0706_STOPCURRENTFRAME); 
}

boolean Adafruit_VC0706::resumeVideo() {
  return cameraFrameBuffCtrl(VC0706_RESUMEFRAME); 
}

boolean Adafruit_VC0706::TVon() {
  uint8 args[] = {0x1, 0x1};
  return runCommand(VC0706_TVOUT_CTRL, args, sizeof(args), 5);
}
boolean Adafruit_VC0706::TVoff() {
  uint8 args[] = {0x1, 0x0};
  return runCommand(VC0706_TVOUT_CTRL, args, sizeof(args), 5);
}

boolean Adafruit_VC0706::cameraFrameBuffCtrl(uint8 command) {
  uint8 args[] = {0x1, command};
  return runCommand(VC0706_FBUF_CTRL, args, sizeof(args), 5);
}

uint32 Adafruit_VC0706::frameLength(void) {
  uint8 args[] = {0x01, 0x00};
  if (!runCommand(VC0706_GET_FBUF_LEN, args, sizeof(args), 9))
    return 0;

  uint32 len;
  len = camerabuff[5];
  len <<= 8;
  len |= camerabuff[6];
  len <<= 8;
  len |= camerabuff[7];
  len <<= 8;
  len |= camerabuff[8];

  return len;
}


uint8 Adafruit_VC0706::available(void) {
  return bufferLen;
}


uint8* Adafruit_VC0706::readPicture(uint8 n) {
  uint8 args[] = {0x0C, 0x0, 0x0A, 
                    0, 0, frameptr >> 8, frameptr & 0xFF, 
                    0, 0, 0, n, 
                    CAMERADELAY >> 8, CAMERADELAY & 0xFF};

  if (! runCommand(VC0706_READ_FBUF, args, sizeof(args), 5, false))
    return 0;


  // read into the buffer PACKETLEN!
  if (readResponse(n+5, CAMERADELAY) == 0) 
      return 0;


  frameptr += n;

  return camerabuff;
}

/**************** low level commands */


boolean Adafruit_VC0706::runCommand(uint8 cmd, uint8 *args, uint8 argn, 
			   uint8 resplen, boolean flushflag) {
  // flush out anything in the buffer?
  if (flushflag) {
    readResponse(100, 10); 
  }

  sendCommand(cmd, args, argn);
  if (readResponse(resplen, 200) != resplen) 
    return false;
  if (! verifyResponse(cmd))
    return false;
  return true;
}

void Adafruit_VC0706::sendCommand(uint8 cmd, uint8 args[] = 0, uint8 argn = 0) {
    hwSerial->write((byte)0x56);
    hwSerial->write((byte)serialNum);
    hwSerial->write((byte)cmd);

    for (uint8 i=0; i<argn; i++) {
      hwSerial->write((byte)args[i]);
      //Serial.print(" 0x");
      //Serial.print(args[i], HEX);
    }
}

uint8 Adafruit_VC0706::readResponse(uint8 numbytes, uint8 timeout) {
  uint8 counter = 0;
  bufferLen = 0;
  int avail;
 
  while ((timeout != counter) && (bufferLen != numbytes)){
    avail = hwSerial->available();
    if (avail <= 0) {
      delay(1);
      counter++;
      continue;
    }
    counter = 0;
    // there's a byte!
    camerabuff[bufferLen++] = hwSerial->read();
  }
  return bufferLen;
}

boolean Adafruit_VC0706::verifyResponse(uint8 command) {
  if ((camerabuff[0] != 0x76) ||
      (camerabuff[1] != serialNum) ||
      (camerabuff[2] != command) ||
      (camerabuff[3] != 0x0))
      return false;
  return true;
  
}

void Adafruit_VC0706::printBuff() {
  for (uint8 i = 0; i< bufferLen; i++) {
    SerialUSB.print(" 0x");
    SerialUSB.print(camerabuff[i], HEX); 
  }
  SerialUSB.println();
}

        
