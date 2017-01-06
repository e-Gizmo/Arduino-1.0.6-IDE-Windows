#include <Adafruit_VC0706.h>


// If using camera with outdoor casing, connect green to TX3 and white to RX3, if you are using
// serial comera without outdoor casing, white to TX3 and green to RX3.

// In Maple & iteadMaple
// TX3 is pin 29 and RX3 is pin 30

// For correct pins, check http://leaflabs.com/devices/

// WARNING:
// I tried to connect to non 5v tolerant Serial1 and wasn't able to make this work..
// I suggest using only 5v tolerant serials. (Serial2 and Serial3)

// This example does not do anything with picture data, it does not save image to SD.
// It uses camera's motion sensor and take's a photo to camera and reports it's filesize.

Adafruit_VC0706 cam = Adafruit_VC0706(&Serial3); // <-- Define used hwSerial

#define delayTime 2000

uint32 lastPhoto;
boolean motionD;

void setup() {

  lastPhoto = millis();
  motionD = false;
  
  delay(2500);
  
  SerialUSB.println("VC0706 Camera test");
    
  // Try to locate the camera
  if (cam.begin()) {
    SerialUSB.println("Camera Found:");
  } else {
    SerialUSB.println("No camera found?");
    return;
  }
   
   delay(200);
    
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
    SerialUSB.print("Failed to get version");
  } else {
    SerialUSB.println("-----------------");
    SerialUSB.print(reply);
    SerialUSB.println("-----------------");
  }

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!

  //cam.setImageSize(VC0706_640x480);        // biggest
  cam.setImageSize(VC0706_320x240);        // medium
  //cam.setImageSize(VC0706_160x120);          // small
  
//  cam.setImageSize(imgSize);
/*  
  if (!cam.reset()) // My camera showd correct resolution when querying size, but file was with previously selected resolution.. Until I
    SerialUSB.println("Reset error"); // made it reset after setting imgSize.. 
    
  cam.getVersion(); // if version is not queried, imgSize and other things cannot be queried..
*/

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8 imgSize = cam.getImageSize();
  SerialUSB.print("Image size: ");
  if (imgSize == VC0706_640x480) SerialUSB.print("640x480 (");
  if (imgSize == VC0706_320x240) SerialUSB.print("320x240 (");
  if (imgSize == VC0706_160x120) SerialUSB.print("160x120 (");
  SerialUSB.print(imgSize); SerialUSB.println(")");


  //  Motion detection system can alert you when the camera 'sees' motion!
  cam.setMotionDetect(true);           // turn it on
  //cam.setMotionDetect(false);        // turn it off   (default)

  // You can also verify whether motion detection is active!
  SerialUSB.print("Motion detection is ");
  if (cam.getMotionDetect()) 
    SerialUSB.println("ON");
  else 
    SerialUSB.println("OFF");
}




void loop() {
  
 if (cam.motionDetected())
   motionD = true;
  
 if ((motionD) && ( millis() > (lastPhoto + delayTime))) {
   SerialUSB.println("Motion!");   
   cam.setMotionDetect(false);
   
  if (! cam.takePicture()) 
    SerialUSB.println("Failed to snap!");
  else {
    SerialUSB.print("Picture taken! ");
    
    uint16 jpglen = cam.frameLength();
    SerialUSB.print(jpglen, DEC);
    SerialUSB.println(" byte image");
  }
  
  cam.resumeVideo();
  cam.setMotionDetect(true);
  lastPhoto = millis();
  motionD = false;
 }
}

