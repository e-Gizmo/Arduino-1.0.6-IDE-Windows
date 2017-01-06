/*
||
|| @author         Brett Hagman <bhagman@wiring.org.co>
|| @url            http://wiring.org.co/
|| @url            http://roguerobotics.com/
||
|| @description
|| | Rogue Robotics SD Module Library
|| |
|| | This Wiring and Arduino Library works with the following
|| | Rogue Robotics modules:
|| |   - uMMC (SD Card Module)
|| |   - uMP3 (Industrial MP3 Playback Module)
|| |   - rMP3 (Commercial MP3 Playback Module)
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

#include <stdint.h>
#include <ctype.h>

#if WIRING
 #include <Wiring.h>
#elif ARDUINO >= 100
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

#include "RogueSD.h"

/*
|| Private Constants
*/

#define UMMC_MIN_FW_VERSION_FOR_NEW_COMMANDS  10201
#define UMP3_MIN_FW_VERSION_FOR_NEW_COMMANDS  11101

#define STATS_SIZE                            0
#define STATS_POSITION                        1
#define STATS_REMAINING                       2

#define ASCII_ESC                             0x1b

// Default to 1 second.
#define ROGUESD_DEFAULT_READ_TIMEOUT          100


/*
|| Constructor
*/

RogueSD::RogueSD(Stream &comms)
  : lastErrorCode(0),
    _promptChar(DEFAULT_PROMPT),
    _fwVersion(0),
    _moduleType(uMMC),
    _fwLevel(0)
{
  _comms = &comms;
  _prefix = "";
}



/*
|| Public Methods
*/

int8_t RogueSD::sync(bool blocking)
{
  // procedure:
  // 1. sync (send ESC, clear prompt)
  // 2. get version ("v"), and module type
  // 3. change settings as needed
  // 4. close files

  // 0. empty any data in the serial buffer
  _comms->flush();

  // 1. sync
  print((char)ASCII_ESC);               // send ESC to clear buffer on uMMC
  if (blocking)
  {
    _readBlocked();                     // consume prompt
  }
  else
  {
    if (_readTimeout(ROGUESD_DEFAULT_READ_TIMEOUT) < 0)
    {
      return 0;
    }
  }

  // 2. get version (ignore prompt - just drop it)
  _getVersion();
  if (_moduleType == uMMC)
    _prefix = "";
  else
    _prefix = "FC";

  // 3. change settings as needed
  // OLD: write timeout setting = 10 ms timeout
  // NEW: listing style = old style (0)
  if ((_moduleType == uMMC && _fwVersion < UMMC_MIN_FW_VERSION_FOR_NEW_COMMANDS) ||
      (_moduleType == uMP3 && _fwVersion < UMP3_MIN_FW_VERSION_FOR_NEW_COMMANDS))
  {
    // we need to set the write timeout to allow us to control when a line is finished
    // in writeln.
    changeSetting('1', 1);              // 10 ms timeout
    _fwLevel = 0;
  }
  else
  {
    // we're using the new version
    _fwLevel = 1;
    // Let's make sure the Listing Style setting is set to the old style
    if (getSetting('L') != 0)
    {
      changeSetting('L', 0);
    }

    // get the prompt char
    print('S');
    if (_moduleType != uMMC) print('T');
    print('P');
    print('\r');  // get our prompt (if possible)
    _promptChar = _getNumber(10);
    _readBlocked();                     // consume prompt
  }

  // 4. close files

  closeAll();                           // ensure all handles are closed

  return 1;
}


int32_t RogueSD::cardInfo(uint8_t getSize)
{
  int32_t datum = 0;

  print(_prefix);
  print('Q');
  print('\r');

  while (!_commAvailable());
  if (_commPeek() != 'E')
  {
    datum = _getNumber(10);             // Free space (in KiB)

    _readBlocked();                     // consume '/' or ' '

    if (getSize)
      datum = _getNumber(10);           // Card size
    else
      _getNumber(10);

    _readBlocked();                     // consume prompt
  }
  else
  {
    _getResponse();
    // if we have an error, return -1
    // error code is actually in .lastErrorCode
    datum = -1;
  }

  return datum;
}


int8_t RogueSD::status(int8_t handle)
{
  int8_t resp = 0;

  print(_prefix);
  print('Z');
  if (handle > 0)
    print((char)('0' + handle));
  print('\r');                         // Get status

  resp = _getResponse();

  if (resp)
    _readBlocked();                    // consume prompt if OK

  return resp;
}


int8_t RogueSD::getFreeHandle(void)
{
  uint8_t resp;

  print(_prefix);
  print('F');
  print('\r');

  resp = _readBlocked();

  if (resp != 'E')
    resp -= '0';                        // got our handle
  else
  {
    lastErrorCode = _getNumber(16);
    if (lastErrorCode == ERROR_NO_FREE_FILES)
      resp = 0;
    else
      resp = -1;
  }

  _readBlocked();                      // consume prompt

  return resp;
}


int8_t RogueSD::exists(const char *path)
{
  // There are a few ways to determine whether path is a file.
  // Determining whether path is a directory is a bit more tricky.
  // If you use FC L path, the contents of the directory are returned.
  // If you use FC LC path, you will get the number of files within the directory, but
  // if there is only 1 file within the directory, it can be misleading (and what happens
  // if there are NO files in the directory?).
  // The current best solution is:
  // 1. Use "FC LS path" to "Open Directory" - if no error, then path is a folder.
  // 2. If an error was returned, use "FC LC path" - if no error, then path is a file.

  if (openDir(path))
  {
    return TYPE_FOLDER;
  }
  else
  {
    if (fileCount(path) == 1)
      return TYPE_FILE;
    else
      return 0;
  }
}


fileInfo RogueSD::getFileInfo(int8_t handle)
{
  fileInfo fi;

  print(_prefix);
  print('I');
  print((char)('0' + handle));
  print('\r');

  while (!_commAvailable());
  if (_commPeek() != 'E')
  {
    fi.position = _getNumber(10);       // current file position

    _readBlocked();                     // consume '/' or ' '

    fi.size = _getNumber(10);           // file size

    _readBlocked();                     // consume prompt
  }
  else
  {
    _getResponse();
    // if we have an error, just return 0's
    // error code is actually in .lastErrorCode
    fi.position = 0;
    fi.size = 0;
  }

  return fi;
}


int32_t RogueSD::size(const char *path)
{
  char c;
  uint32_t filesize = 0;
  int8_t resp;

  if (_fwLevel == 0)
  {
    // old
    lastErrorCode = ERROR_NOT_SUPPORTED;
    return -1;
  }

  resp = exists(path);

  if (resp == TYPE_FOLDER)
  {
    // path is a folder
    lastErrorCode = ERROR_NOT_A_FILE;
    return -1;
  }
  else if (resp == TYPE_FILE)
  {
    print(_prefix);
    print("L ");
    print(path);
    print('\r');

    if (_getResponse())
    {
      // we have the file info next
      while (!_commAvailable());

      filesize = _getNumber(10);
      
      // clear the rest
      while ((c = _readBlocked()) != '\r');

      _readBlocked();                   // consume prompt

      return (int32_t) filesize;
    }
    else
    {
      // had an error
      return -1;
    }
  }
  else
  {
    // path does not exist
    lastErrorCode = ERROR_FILE_DOES_NOT_EXIST;
    return -1;
  }

}


int8_t RogueSD::changeSetting(char setting, uint8_t value)
{
  print('S');
  if (_moduleType != uMMC) print('T');
  print(setting);
  print((int)value, DEC);
  print('\r');

  return _getResponse();
}


int16_t RogueSD::getSetting(char setting)
{
  uint8_t value;

  print('S');
  if (_moduleType != uMMC) print('T');
  print(setting);
  print('\r');

  while (!_commAvailable());
  if (_commPeek() != 'E')
  {
    value = _getNumber(10);
    _readBlocked();                    // consume prompt
  }
  else
  {
    value = _getResponse();            // get the error
  }

  return value;
}


void RogueSD::getTime(uint16_t *rtc)
{
  if (_fwLevel > 0)
  {
    print('T');
    print('\r');
    for (uint8_t i = 0; i < 7; i++)
    {
      rtc[i] = _getNumber(10);
      _readBlocked();                  // consume separators, and command prompt (at end)
    }
  }
  else
  {
    // old
    return;
  }
}


void RogueSD::setTime(uint16_t rtc[])
{
  if (_fwLevel > 0)
  {
    print('T');
    for (uint8_t i = 0; i < 6; i++)
    {
      print(rtc[i], DEC);
      print(' ');
    }

    print('\r');

    // A bug in earlier versions returned the time after setting
    while (_readBlocked() != _promptChar); // kludge: read everything including prompt

    // _readBlocked();                    // consume prompt
  }
  else
  {
    // old
    return;
  }
}


void RogueSD::setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
  uint16_t rtc[6];

  rtc[0] = year;
  rtc[1] = month;
  rtc[2] = day;
  rtc[3] = hour;
  rtc[4] = minute;
  rtc[5] = second;

  setTime(rtc);
}


int8_t RogueSD::open(const char *path)
{
  // defaults to READ
  return open(path, OPEN_READ);
}


int8_t RogueSD::open(const char *path, openMode mode)
{
  int8_t fh = getFreeHandle();

  if (fh > 0)
    return _open(fh, path, mode, 0);
  else
    return fh;
}


int8_t RogueSD::open(int8_t handle, const char *path)
{
  // defaults to READ
  return _open(handle, path, OPEN_READ, 0);
}


int8_t RogueSD::open(int8_t handle, const char *path, openMode mode)
{
  return _open(handle, path, mode, 0);
}


int8_t RogueSD::open_P(const char *path)
{
  return open_P(path, OPEN_READ);
}


int8_t RogueSD::open_P(const char *path, openMode mode)
{
  int8_t fh = getFreeHandle();

  if (fh > 0)
    return _open(fh, path, mode, 1);
  else
    return fh;
}


int8_t RogueSD::open_P(int8_t handle, const char *path)
{
  return _open(handle, path, OPEN_READ, 1);
}


int8_t RogueSD::open_P(int8_t handle, const char *path, openMode mode)
{
  return _open(handle, path, mode, 1);
}


// private
int8_t RogueSD::_open(int8_t handle, const char *path, openMode mode, int8_t pgmspc)
{
  int8_t resp;

  print(_prefix);
  print('O');
  print((char)('0' + handle));
  print(' ');

  switch (mode)
  {
    case OPEN_READ:
    case OPEN_RW:
      print('R');
      if (mode == OPEN_READ) break;
    case OPEN_WRITE:
      print('W');
      break;
    case OPEN_APPEND:
      print('A');
      break;
  }

  print(' ');
  if (pgmspc)
    print_P(path);
  else
    print(path);

  print('\r');

  resp = _getResponse();
  return resp ? handle : 0;
}



void RogueSD::close(int8_t handle)
{
  print(_prefix);
  print('C');
  print((char)('0' + handle));
  print('\r');
  _getResponse();
}


void RogueSD::closeAll(void)
{
  if (_fwLevel > 0)
  {
    // new
    print(_prefix);
    print('C');
    print('\r');
    _readBlocked();                     // consume prompt
  }
  else
  {
    // old
    for (uint8_t i = 1; i <= 4; i++)    // 4 = Max handles
    {
      print(_prefix);
      print('C');
      print((char)('0' + i));
      print('\r');
      _getResponse();
    }
  }
}


int32_t RogueSD::fileCount(const char *path, const char *filemask)
{
  int32_t fcount = 0;

  if (_fwLevel > 0)
  {
    print(_prefix);
    print("LC ");
    if (path != NULL)
      print(path);

    if (filemask != NULL && strlen(filemask) > 0)
    {
      if (strlen(path) && path[strlen(path) - 1] != '/')
      {
        print('/');
      }
      print(filemask);
    }
    print('\r');

    if (_getResponse())
    {
      fcount = _getNumber(10);

      _readBlocked();                   // consume prompt

      return fcount;
    }
    else
    {
      // error occurred with "LC" command
      return -1;
    }
  }
  else
  {
    // old
    lastErrorCode = ERROR_NOT_SUPPORTED;
    return -1;
  }
}


int8_t RogueSD::openDir(const char *dirname)
{
  int8_t resp = 0;

  if (_fwLevel > 0)
  {
    // new
    print(_prefix);
    print("LS ");
    print(dirname);
    print('\r');
    resp = _getResponse();
    if (resp)
      _readBlocked();                     // consume prompt only if OK
    return resp;
  }
  else
  {
    // old
    lastErrorCode = ERROR_NOT_SUPPORTED;
    return -1;
  }
}


int8_t RogueSD::readDir(char *dest, const char *filemask)
{
  // retrieve the next entry from the directory
  // returns:
  // 0 when no more files/folders (EOF)
  // -1 on failure
  // 1 if entry is a file
  // 2 if entry is a folder/directory

  // currently using the original file listing style, i.e. D/fs filename

  char c;
  int8_t entrytype = TYPE_FILE;

  if (_fwLevel > 0)
  {
    print(_prefix);
    print("LI ");
    if (filemask)
      print(filemask);
    else
      print('*');
    print('\r');

    if (_getResponse())
    {
      // we have the file info next
      while (!_commAvailable());

      if (_commPeek() == 'D')
      {
        // we have a directory
        _commRead();                    // consume 'D'
        entrytype = TYPE_FOLDER;
      }
      else
      {
        // it's a file, with a file size
        _getNumber(10);                 // discard (for now)
        entrytype = TYPE_FILE;
      }

      _readBlocked();                   // consume separator ' '

      // now get filename
      while ((c = _readBlocked()) != '\r')
      {
        *dest++ = c;
      }
      *dest = 0;                        // terminate string

      _readBlocked();                   // consume prompt

      return entrytype;
    }
    else
    {
      // had an error
      if (lastErrorCode == ERROR_EOF)
        return 0;
      else
        return -1;
    }
  }
  else
  {
    // old
    lastErrorCode = ERROR_NOT_SUPPORTED;
    return -1;
  }
}


int8_t RogueSD::entryToFilename(char *dest, uint8_t count, uint16_t entrynum, const char *path, const char *filemask)
{
  char c;
  int8_t entrytype = TYPE_FILE;

  if (_fwLevel > 0)
  {
    // new
    print(_prefix);
    print("LE ");
    print(entrynum, DEC);
    print(' ');
    if (path != NULL)
      print(path);

    if (filemask != NULL && strlen(filemask) > 0)
    {
      if (strlen(path) == 0 || path[strlen(path) - 1] != '/')
      {
        print('/');
      }
      print(filemask);
    }

    print('\r');

    if (_getResponse())
    {
      // we have the file info next
      while (!_commAvailable());

      if (_commPeek() == 'D')
      {
        // we have a directory
        _commRead();                    // consume 'D'
        entrytype = TYPE_FOLDER;
      }
      else
      {
        // it's a file, with a file size
        _getNumber(10);                 // discard (for now)
        entrytype = TYPE_FILE;
      }

      _readBlocked();                   // consume separator ' '

      // now get filename
      while ((c = _readBlocked()) != '\r')
      {
        if (count > 0)
        {
          count--;
          *dest++ = c;
        }
      }
      *dest = 0;                        // terminate string

      _readBlocked();                   // consume prompt

      return entrytype;
    }
    else
    {
      // had an error
      return 0;
    }
  }
  else
  {
    // old
    lastErrorCode = ERROR_NOT_SUPPORTED;
    return -1;
  }
}



int8_t RogueSD::remove(const char *path)
{
  print(_prefix);
  print('E');
  print(path);
  print('\r');

  return _getResponse();
}


int8_t RogueSD::rename(const char *oldpath, const char *newpath)
{
  if (oldpath == NULL || newpath == NULL)
    return 0;

  print(_prefix);
  print('N');
  print(oldpath);
  print('|');
  print(newpath);
  print('\r');

  return _getResponse();
}


// Read a single byte from the given handle.
int16_t RogueSD::readByte(int8_t handle)
{
  uint8_t ch = 0;

  print(_prefix);
  print('R');
  print((char)('0' + handle));
  print(' ');
  print('1');
  print('\r');

  // we will get either a space followed by 1 byte, or an error

  if (_getResponse())
  {
    // we have a single byte waiting
    ch = _readBlocked();
    _readBlocked();                     // consume prompt
    return (unsigned char) ch;
  }
  else
  {
    // had an error
    if (lastErrorCode == ERROR_EOF)
      return -1;
    else
      return -2;
  }
}


int16_t RogueSD::read(int8_t handle, uint16_t count, char *dest)
{
  // read up to count bytes into dest
  uint32_t bytesremaining;
  uint16_t i;
  fileInfo fi = getFileInfo(handle);

  // check first how many bytes are remaining
  bytesremaining = fi.size - fi.position;

  if (bytesremaining > 0)
  {
    if (count > bytesremaining)
      count = bytesremaining;
    print(_prefix);
    print('R');
    print((char)('0' + handle));
    print(' ');
    print(count, DEC);
    print('\r');
  }
  else
  {
    return 0;
  }

  // now read count bytes

  if (!_getResponse())
  {
    if (lastErrorCode == ERROR_EOF)
      return -1;
    else
      return -2;
  }

  for (i = 0; i < count; i++)
    dest[i] = _readBlocked();

  _readBlocked();                       // consume prompt

  return i;                             // return number of bytes read
}


int16_t RogueSD::readln(int8_t handle, uint16_t maxlength, char *dest)
{
  int8_t r, i;

  if (_fwLevel == 0)
    return -1;

  print(_prefix);
  print("RL");                          // Read a line, maxlength chars, maximum
  print((char)('0' + handle));
  print(' ');
  print(maxlength, DEC);
  print('\r');

  if (!_getResponse())
  {
    if (lastErrorCode == ERROR_EOF)
      // EOF
      return -1;
    else
      return -2;
  }

  // otherwise, read the data

  i = 0;
  r = _readBlocked();

  while (r != _promptChar)              // we could have a blank line
  {
    dest[i++] = r;
    r = _readBlocked();
  }

  dest[i] = 0;                          // terminate our string

  return i;
}


int8_t RogueSD::write(int8_t handle, uint16_t count, const char *data)
{
  print(_prefix);
  print('W');
  print((char)('0' + handle));
  print(' ');
  print(count, DEC);
  print('\r');

  while (count--)
    write(*data++);

  // after we are done, check the response
  return _getResponse();
}


int8_t RogueSD::writeByte(int8_t handle, char data)
{
  return write(handle, 1, &data);
}


int8_t RogueSD::write(int8_t handle, const char *data)
{
  return write(handle, strlen(data), data);
}


void RogueSD::writelnStart(int8_t handle)
{
  // be warned: if using this command with firmware less than 102.xx/111.xx
  // you must IMMEDIATELY write data within 10ms or the write time-out will occur

  print(_prefix);
  print('W');

  if (_fwLevel > 0)
  {
    // new
    print('L');
    print((char)('0' + handle));
  }
  else
  {
    // old
    print((char)('0' + handle));
    print(" 512");
  }

  print('\r');
}


int8_t RogueSD::writelnFinish(void)
{
  if (_fwLevel > 0)
  {

  // new
    print('\r');
  }
  else
  {
    // old
    // we wait for more than 10ms to terminate the Write command (write time-out)
    delay(11);
  }

  return _getResponse();
}


int8_t RogueSD::writeln(int8_t handle, const char *data)
{
  writelnStart(handle);

  while (*data)
  {
    write(*data++);
    if (*data == '\r') break;
  }

  return writelnFinish();
}


int8_t RogueSD::seek(int8_t handle, uint32_t newposition)
{
  print(_prefix);

  if (_fwLevel > 0)
  {
    // new
    // J fh position
    print('J');
    print((char)('0' + handle));
  }
  else
  {
    // old
    // we need to do an empty read to seek to our position
    // R fh 0 position
    print('R');
    print((char)('0' + handle));
    print(' ');
    print('0');
  }

  // common portion
  print(' ');
  print(newposition, DEC);
  print('\r');

  return _getResponse();
}


int8_t RogueSD::seekToEnd(int8_t handle)
{
  if (_fwLevel > 0)
  {
    // new
    print(_prefix);
    // J fh E
    print('J');
    print((char)('0' + handle));
    print('E');
    print('\r');
  }
  else
  {
    // old
    // two step process - get filesize, seek to that position
    return seek(handle, getFileInfo(handle).size);
  }

  return _getResponse();
}


// Added for sending PROGMEM strings
void RogueSD::print_P(const char *str)
{
  while (pgm_read_byte(str) != 0)
  {
    print(pgm_read_byte(str++));
  }
}


/*
|| Public (virtual - required by Print)
*/

#if ARDUINO >= 100

size_t RogueSD::write(uint8_t c)
{
  _comms->write(c);
  return 1;
}

#else

void RogueSD::write(uint8_t c)
{
  _comms->write(c);
}

#endif


/*
|| Private Methods
*/

int8_t RogueSD::_readBlocked(void)
{
  // int8_t r;

  while (!_commAvailable());
  // while((r = this->_readf()) < 0);   // this would be faster if we could guarantee that the _readf() function
                                        // would return -1 if there was no byte read
  return _commRead();
}


int16_t RogueSD::_readTimeout(uint16_t timeout)
{
  while (timeout)
  {
    if (_commAvailable())
      return (uint8_t) _commRead();

    timeout--;
    delay(10);
  }

  return -1;
}


int8_t RogueSD::_getResponse(void)
{
  // looking for a response
  // If we get a space " ", we return as good and the remaining data can be retrieved
  // " ", ">", "Exx>" types only
  uint8_t r;
  uint8_t resp = 0;

  // we will return 1 if all is good, 0 otherwise (lastErrorCode contains the response from the module)

  r = _readBlocked();

  if (r == ' ' || r == _promptChar)
    resp = 1;

  else if (r == 'E')
  {
    lastErrorCode = _getNumber(16);     // get our error code
    _readBlocked();                     // consume prompt

    resp = 0;
  }

  else
  {
    lastErrorCode = 0xFF;               // something got messed up, a resync would be nice
    resp = 0;
  }

  return resp;
}


int16_t RogueSD::_getVersion(void)
{
  // get the version, and module type
  print('V');
  print('\r');

  // Version format: mmm.nn[-bxxx] SN:TTTT-ssss...

  // get first portion mmm.nn
  _fwVersion = _getNumber(10);
  _readBlocked();                       // consume '.'
  _fwVersion *= 100;
  _fwVersion += _getNumber(10);
  // ignore beta version (-bxxx), if it's there
  if (_readBlocked() == '-')
  {
    for (char i = 0; i < 5; i++)        // drop bxxx plus space
      _readBlocked();
  }
  // otherwise, it was a space

  // now drop the SN:
  _readBlocked();
  _readBlocked();
  _readBlocked();

  if (_readBlocked() == 'R')
    _moduleType = rMP3;
  else
  {
    // either UMM1 or UMP1
    // so drop the M following the U
    _readBlocked();
    if (_readBlocked() == 'M')
      _moduleType = uMMC;
    else
      _moduleType = uMP3;
  }

  // ignore the rest
  while (_readBlocked() != '-');

  // consume up to and including prompt
  while (isalnum(_readBlocked()));

  return _fwVersion;
}


int32_t RogueSD::_getNumber(uint8_t base)
{
  uint8_t c, neg = 0;
  uint32_t val;

  val = 0;
  while (!_commAvailable());
  c = _commPeek();

  if (c == '-')
  {
    neg = 1;
    _commRead();  // remove
    while (!_commAvailable());
    c = _commPeek();
  }

  while (((c >= 'A') && (c <= 'Z'))
         || ((c >= 'a') && (c <= 'z'))
         || ((c >= '0') && (c <= '9')))
  {
    if (c >= 'a') c -= 0x57;            // c = c - 'a' + 0x0a, c = c - ('a' - 0x0a)
    else if (c >= 'A') c -= 0x37;       // c = c - 'A' + 0x0A
    else c -= '0';
    if (c >= base) break;

    val *= base;
    val += c;
    _commRead();                       // take the byte from the queue
    while (!_commAvailable());         // wait for the next byte
    c = _commPeek();
  }
  return neg ? -val : val;
}


uint8_t RogueSD::_commAvailable(void)
{
  return _comms->available();
}


int RogueSD::_commPeek(void)
{
  return _comms->peek();
}


int RogueSD::_commRead(void)
{
  return _comms->read();
}

