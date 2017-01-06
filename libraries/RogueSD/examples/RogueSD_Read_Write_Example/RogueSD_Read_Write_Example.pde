/*
|| @author         Brett Hagman <bhagman@wiring.org.co>
|| @url            http://wiring.org.co/
|| @url            http://roguerobotics.com/
||
|| @description
|| | Rogue Robotics SD Module Library Example.
|| |
|| | Demonstrates writing/reading to/from the card.
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
  /*
  || Local Variables
  */
  // A "handle" to identify our file.
  int8_t filehandle;
  // The filename of the file we will write to and read from.
  const char filename[] = "/appendtest.txt";
  // A string (char array) to get data from the file.
  char fileData[60];


  // To protect the card from the multiple resets during power up via USB,
  // let's wait a bit to make sure we are ready to write.
  delay(1000);

  Serial.begin(9600);

  rogueSerial.begin(9600);

  // Prepare the random() function to give us better random values.
  randomSeed(analogRead(0));

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


  // Now, on with our example.

  // Open a file
  Serial.println(Constant("Opening file to append some data..."));
  filehandle = roguesd.open(filename, OPEN_APPEND);

  if (filehandle > 0)
  {
    Serial.print(Constant("Using handle "));
    Serial.println(filehandle);

    // Prepare the module to write data to the file
    roguesd.writelnStart(filehandle);

    // you can use any of the standard print methods to send data to the file.
    roguesd.print(Constant("You'll see this many times in this file. RAND: "));
    roguesd.print(random(10000));
    roguesd.writelnFinish();

    Serial.println(Constant("Append complete."));

    roguesd.close(filehandle);
    Serial.println(Constant("File closed."));

    // now let's read the file
    filehandle = roguesd.open(filename, OPEN_READ);

    Serial.println(Constant("Opening file for read..."));

    if (filehandle > 0)
    {
      Serial.print(Constant("Using handle "));
      Serial.println(filehandle);

      Serial.println(Constant("-------------"));

      // this reads 60 characters at a time (if available, per line)
      while (roguesd.readln(filehandle, 60, fileData) > 0)
      {
        Serial.println(fileData);

        delay(200);  // pause 200 milliseconds between lines
      }

      Serial.println(Constant("-------------"));

      roguesd.close(filehandle);
      Serial.println(Constant("File closed."));
    }
    else
    {
      Serial.println(Constant("An error occurred while opening the file to read:"));
      Serial.print(Constant("Error code E"));
      if (roguesd.LastErrorCode < 16)
        Serial.print('0');
      Serial.println(roguesd.LastErrorCode, HEX);
    }
  }
  else
  {
    Serial.println(Constant("An error occurred while opening the file to write:"));

    if (roguesd.LastErrorCode == ERROR_CARD_NOT_INSERTED)
    {
      Serial.println(Constant("No card inserted."));
    }
    else
    {
      Serial.print(Constant("Error code E"));
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

