/*
|| @author         Brett Hagman <bhagman@wiring.org.co>
|| @url            http://wiring.org.co/
|| @url            http://roguerobotics.com/
||
|| @description
|| | Rogue Robotics SD Module Library Example.
|| |
|| | Lists all files on the card.
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

// This is the path from which we want to list all files and folders.
const char fileListingPath[] = "/";
// This is the file mask (i.e. pattern) we want to list.
// e.g. "*.mp3"
const char filePattern[] = "*";


void setup()
{
  // This string (array of chars) is used to store the filenames
  // we get from readDir() below.
  char filename[64];
  // Number of files and folders.
  int32_t filecount;

  // Just a simple wait to allow for any unforseen fiddlybits.
  delay(1000);

  Serial.begin(9600);

  // Prepare the serial port connected to the Rogue SD module.
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


  // Now, on with our example.

  // Check if a card is inserted.
  if (roguesd.status())
  {
    // List all files on the card.

    // fileCount() takes 2 arguments:
    // 1. source path
    // 2. file mask (e.g. list only MP3 files: "*.mp3")
    // e.g. roguesd.fileCount("/", "*.mp3");
    filecount = roguesd.fileCount(fileListingPath, filePattern);

    if (filecount >= 0)
    {
      Serial.print(Constant("File count: "));
      Serial.println(filecount, DEC);

      if (filecount > 0)
      {
        Serial.println(Constant("--- [F]iles and Fol[D]ers ---"));

        // openDir() opens the directory for reading.
        // The argument is the path.
        roguesd.openDir(fileListingPath);

        // readDir() gets the next directory entry that matches the mask.
        // It takes 2 arguments:
        // 1. a string (char array) to store the name
        //    - make sure that it's big enough to store the largest name
        //      in the directory.
        // 2. file mask (same as in fileCount())

        // type, which is returned from readDir(), will tell us if
        // it is a TYPE_FOLDER or a TYPE_FILE.
        int type;

        while ((type = roguesd.readDir(filename, filePattern)) > 0)
        {
          if (type == TYPE_FOLDER)  // if it's a folder/directory
            Serial.print(Constant("[D] "));
          else
            Serial.print(Constant("[F] "));
          Serial.println(filename);
        }

        Serial.println(Constant("-----------------------------"));
      }
    }
    else
    {
      // fileCount() failed, because card was ejected, or something else.
      Serial.print(Constant("fileCount failed: Error code E"));
      if (roguesd.LastErrorCode < 16)
        Serial.print('0');
      Serial.println(roguesd.LastErrorCode, HEX);
    }
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
