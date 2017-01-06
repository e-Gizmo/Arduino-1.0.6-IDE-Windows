/*
|| @author         Brett Hagman <bhagman@wiring.org.co>
|| @url            http://wiring.org.co/
|| @url            http://roguerobotics.com/
||
|| @description
|| | Rogue Robotics SD Module Library Example.
|| |
|| | This example shows how to detect the current serial bitrate,
|| | to which the RogueSD module is set.
|| | 
|| | Because this example uses NewSoftSerial/SoftwareSerial (by Mikal Hart),
|| | we can only detect up to 115200 bps.
|| | 
|| | If you use a HardwareSerial port, you can detect up to 460800 bps.
|| | 
|| | You can set the baud/bitrate by creating a file
|| | called RMP3.CFG/UMP3.CFG/UMMC.CFG for the rMP3, uMP3,
|| | or uMMC respectively, and add "Dn" on a single line of the file,
|| | where n is one of the bitrate settings from the module's
|| | documentation.
|| |
|| | This Wiring and Arduino Library example works with the following
|| | Rogue Robotics modules:
|| |   - uMMC (SD Card Module)
|| |   - uMP3 (Industrial MP3 Playback Module)
|| |   - rMP3 (Commercial MP3 Playback Module)
|| |
|| | This example uses pins 6 and 7 to connect to "T" and "R" on
|| | the module respectively.  (The rMP3 is connected to pins
|| | 6 and 7 on the shield connector).
|| |
|| | Requires:
|| | uMMC firmware > 102.01
|| | uMP3 firmware > 111.01
|| | rMP3 firmware > 100.00
|| |
|| | See http://www.roguerobotics.com/faq/update_firmware for updating firmware.
|| #
||
|| @license Please see LICENSE.txt for this project.
||
*/

#include <NewSoftSerial.h>
#include <RogueSD.h>

#define ASCII_ESC 0x1b

// This is the serial connection to the Rogue SD module.
// We are using pins 6 and 7 in this example.
const int roguesdRXPin = 6;
const int roguesdTXPin = 7;
NewSoftSerial rogueSerial = NewSoftSerial(roguesdRXPin, roguesdTXPin);

// We connect the above serial object to the RogueSD object here.
RogueSD roguesd = RogueSD(rogueSerial);


// This method will wait for timeout * 10 milliseconds for a character.
int16_t readTimeout(uint16_t timeout)
{
  while (timeout)
  {
    if (rogueSerial.available())
      return (uint8_t) rogueSerial.read();

    timeout--;
    delay(10);
  }

  // We timed out!
  return -1;
}


int32_t rogueSDGetBitrate(void)
{
  LongTable bps = LongTable(115200, 57600, 38400, 19200, 9600, 4800, 2400);

  // Loop variables
  uint8_t i, j;
  // Current bps to test
  int32_t testbps;

  for (i = 0; i < bps.count(); i++)
  {
    testbps = bps[i];

    // Set the serial bitrate for this test.
    rogueSerial.begin(testbps);

    Serial.print(Constant("Trying "));
    Serial.print(testbps, DEC);
    Serial.print(':');

    // Clear any stray/strange characters first.
    rogueSerial.flush();

    // Send an ASCII ESC (0x1b) to the Rogue SD module.
    rogueSerial.print((char)ASCII_ESC);

    // Wait for 100 ms for a '>'
    if (readTimeout(10) == '>')
    {
      Serial.println(Constant("Connected!"));
      return testbps;
    }

    Serial.println('*');
  }

  return -1;
}


void setup()
{
  int32_t currentbps;

  Serial.begin(9600);

  Serial.println(Constant("Getting current Module bitrate..."));

  currentbps = rogueSDGetBitrate();

  if (currentbps > 0)
  {
    rogueSerial.begin(currentbps);

    Serial.print(Constant("Connecting to Rogue SD module @ "));
    Serial.print(currentbps, DEC);
    Serial.println(Constant(" bps."));

    // We need to begin the communications with the SD module.
    if (!roguesd.begin())
    {
      Serial.println(Constant("Connection failed."));
      Serial.print(Constant("Check pins - should be connected to pins "));
      Serial.print(roguesdRXPin, DEC);
      Serial.print(Constant(" and "));
      Serial.println(roguesdTXPin, DEC);
      Serial.println(Constant("Check baud rate settings."));
      // If we failed, let's just leave.
      return;
    }

    Serial.print(Constant("Rogue SD Module type: "));
    switch (roguesd.getModuleType())
    {
      case uMMC:
        Serial.println(Constant("uMMC"));
        break;
      case uMP3:
        Serial.println(Constant("uMP3"));
        break;
      case rMP3:
        Serial.println(Constant("rMP3"));
        break;
    }
    Serial.print(Constant("Rogue SD Module Version: "));
    Serial.println(roguesd.version());

    /* To change the bitrate, you could use the following:
    roguesd.changeSetting('D', 3);
    Serial.println(Constant("Changed to 57600 bps. Please reset."));
    */
  }
  else
  {
    Serial.println(Constant("Couldn't find Rogue SD module (or bitrate is out of range)."));
    Serial.println(Constant("Connection failed."));
    Serial.print(Constant("Check pins - should be connected to pins "));
    Serial.print(roguesdRXPin, DEC);
    Serial.print(Constant(" and "));
    Serial.println(roguesdTXPin, DEC);
    Serial.println(Constant("Check baud rate settings."));
  }
}


void loop()
{
  // Nothing to do in loop() - everything is done in setup().
}

