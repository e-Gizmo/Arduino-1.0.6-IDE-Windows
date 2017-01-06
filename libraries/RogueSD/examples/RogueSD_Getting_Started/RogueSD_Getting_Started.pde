/*
|| @author         Brett Hagman <bhagman@wiring.org.co>
|| @url            http://wiring.org.co/
|| @url            http://roguerobotics.com/
||
|| @description
|| | Rogue Robotics SD Module Library Example.
|| |
|| | Demonstrates connecting to the module, and getting some
|| | basic information about the inserted card.
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

// This is the serial connection to the Rogue SD module.
// We are using pins 6 and 7 in this example.
const int roguesdRXPin = 6;
const int roguesdTXPin = 7;
NewSoftSerial rogueSerial = NewSoftSerial(roguesdRXPin, roguesdTXPin);

// We connect the above serial object to the RogueSD object here.
RogueSD roguesd = RogueSD(rogueSerial);


void setup()
{
  Serial.begin(9600);

  rogueSerial.begin(9600);

  Serial.println(Constant("Connecting to Rogue SD Module..."));

  // We need to begin the communications with the SD module.
  if (!roguesd.begin())
  {
    Serial.println(Constant("Connection failed."));
    Serial.print(Constant("Check pins - should be connected to pins "));
    Serial.print(roguesdRXPin, DEC);
    Serial.print(Constant(" and "));
    Serial.println(roguesdTXPin, DEC);
    Serial.println(Constant("Check baud rate settings."));
    Serial.println(Constant("Try the RogueSD_Autobaud_Detect example."));
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

  // Check if a card is inserted and print out some information.
  if (roguesd.status())
  {
    Serial.println();
    Serial.print(Constant("Card Size: "));
    Serial.print(roguesd.cardSize());
    Serial.println(Constant(" KiB"));
    Serial.print(Constant("Free Space: "));
    Serial.print(roguesd.freeSpace());
    Serial.println(Constant(" KiB"));
  }
  else
  {
    Serial.print(Constant("The fun stops here: "));
    if (roguesd.LastErrorCode == ERROR_CARD_NOT_INSERTED)
    {
      Serial.println(Constant("No card inserted."));
    }
    else
    {
      Serial.println(Constant("Error code E"));
      if (roguesd.LastErrorCode < 16)
        Serial.print('0');
      Serial.println(roguesd.LastErrorCode, HEX);
    }
  }
}


void loop()
{
  // Nothing to do in loop() - everything is done in setup().
}
