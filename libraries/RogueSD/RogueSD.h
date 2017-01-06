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

#ifndef _ROGUESD_H
#define _ROGUESD_H

// RogueSD version
// @Version 2.0.0
#define ROGUESD_VERSION                       10000

#include <avr/pgmspace.h>
#include <stdint.h>
#include <Stream.h>
#include <Print.h>

/*
|| Public Constants
*/

#ifndef _ROGUEMP3_H

#define DEFAULT_PROMPT                        0x3E

#define ERROR_BUFFER_OVERRUN                  0x02
#define ERROR_NO_FREE_FILES                   0x03
#define ERROR_UNRECOGNIZED_COMMAND            0x04
#define ERROR_CARD_INITIALIZATION_ERROR       0x05
#define ERROR_FORMATTING_ERROR                0x06
#define ERROR_EOF                             0x07
#define ERROR_CARD_NOT_INSERTED               0x08
#define ERROR_MMC_RESET_FAIL                  0x09
#define ERROR_CARD_WRITE_PROTECTED            0x0a
#define ERROR_INVALID_HANDLE                  0xf6
#define ERROR_OPEN_PATH_INVALID               0xf5
#define ERROR_FILE_ALREADY_EXISTS             0xf4
#define ERROR_DE_CREATION_FAILURE             0xf3
#define ERROR_FILE_DOES_NOT_EXIST             0xf2
#define ERROR_OPEN_HANDLE_IN_USE              0xf1
#define ERROR_OPEN_NO_FREE_HANDLES            0xf0
#define ERROR_FAT_FAILURE                     0xef
#define ERROR_SEEK_NOT_OPEN                   0xee
#define ERROR_OPEN_MODE_INVALID               0xed
#define ERROR_READ_IMPROPER_MODE              0xec
#define ERROR_FILE_NOT_OPEN                   0xeb
#define ERROR_NO_FREE_SPACE                   0xea
#define ERROR_WRITE_IMPROPER_MODE             0xe9
#define ERROR_WRITE_FAILURE                   0xe8
#define ERROR_NOT_A_FILE                      0xe7
#define ERROR_OPEN_READONLY_FILE              0xe6
#define ERROR_NOT_A_DIR                       0xe5

#define ERROR_NOT_SUPPORTED                   0xff

#endif

#define TYPE_FILE                             1
#define TYPE_FOLDER                           2

#ifndef GOOD
#define GOOD                                  1
#endif
#ifndef BAD
#define BAD                                   0
#endif

/*
|| Aliases
*/
//#define entrytofilename entryToFilename
//#define filecount fileCount
//#define opendir openDir
//#define readdir readDir
//#define getmoduletype getModuleType
//#define getfreehandle getFreeHandle
//#define lasterrorcode LastErrorCode
//#define readbyte readByte
//#define writebyte writeByte
//#define getfileinfo getFileInfo
//#define getfilesize size
//#define seektoend seekToEnd
//#define gettime getTime
//#define settime setTime
//#define closeall closeAll
//#define changesetting changeSetting
//#define getsetting getSetting
//#define writeln_prep writelnStart
//#define writeln_finish writelnFinish
//#define rmdir rmDir

/*
|| Typedefs, structs, etc
*/

struct fileInfo {
                  uint32_t position;
                  uint32_t size;
                };

enum openMode {
                 OPEN_READ = 1,
                 OPEN_WRITE = 2,
                 OPEN_RW = 3,
                 OPEN_APPEND = 4
               };

#ifndef _ROGUEMP3_H
enum moduleType {
                  uMMC = 1,
                  uMP3,
                  rMP3
                };
#endif

/*
|| Class
*/

class RogueSD : public Print
{
  public:
    // properties
    uint8_t lastErrorCode;
    moduleType getModuleType(void) { return _moduleType; }
    inline int16_t version(void) { return _fwVersion; }

    // methods
    RogueSD(Stream &comms);             // constructor

    int8_t begin(bool blocking = false) { return sync(blocking); }
    int8_t sync(bool blocking = false);

    // Card info (in KiB)
    int32_t cardInfo(uint8_t getSize);
    int32_t cardSize(void) { return cardInfo(true); }
    int32_t freeSpace(void) { return cardInfo(false); }

    // File info
    int8_t status(int8_t handle = 0);
    int8_t getFreeHandle(void);
    int8_t exists(const char *path);
    fileInfo getFileInfo(int8_t handle);
    int32_t size(int8_t handle)
    {
      return getFileInfo(handle).size;
    }
    int32_t size(const char *path); // using "L filename"


    // Settings
    int8_t changeSetting(char setting, uint8_t value);
    int16_t getSetting(char setting);
    void getTime(uint16_t *rtc);
    void setTime(uint16_t rtc[]);
    void setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);


    // Open
    int8_t open(const char *path);
    int8_t open(const char *path, openMode mode);
    int8_t open(int8_t handle, const char *path);
    int8_t open(int8_t handle, const char *path, openMode mode);
    int8_t open_P(const char *path);
    int8_t open_P(const char *path, openMode mode);
    int8_t open_P(int8_t handle, const char *path);
    int8_t open_P(int8_t handle, const char *path, openMode mode);


    // Close
    void close(int8_t handle);
    void closeAll(void);


    // Directory
    int32_t fileCount(const char *path, const char *filemask = NULL);
    int8_t openDir(const char *path);
    int8_t readDir(char *dest, const char *filemask = NULL);
    int8_t entryToFilename(char *dest, uint8_t count, uint16_t entrynum, const char *folder = NULL, const char *filemask = NULL);
    int8_t rmDir(const char *path)
    {
      return remove(path);
    }


    // Delete
    // delete/remove a file/directory (directory must be empty)
    int8_t remove(const char *path);


    // Rename/Move
    // rename a file/directory
    int8_t rename(const char *oldpath, const char *newpath);


    // Read
    // read single byte (-1 if no data)
    int16_t readByte(int8_t handle);

    // read exactly count bytes into dest
    int16_t read(int8_t handle, uint16_t count, char *dest);

    // read up to maxlength characters into dest
    int16_t readln(int8_t handle, uint16_t maxlength, char *dest);

//    int16_t readprep(int8_t handle, uint16_t bytestoread);


    // Write
    // write exactly count bytes to file
    int8_t write(int8_t handle, uint16_t count, const char *data);
    
    // write a single byte to the file
    int8_t writeByte(int8_t handle, char data);

    // write a string (NUL terminated) to file
    int8_t write(int8_t handle, const char *data);

    // we will need to set up the write time-out to make this work properly (done in sync())
    // then you can use the Print functions to print to the file
    void writelnStart(int8_t handle);
    int8_t writelnFinish(void);
    int8_t writeln(int8_t handle, const char *data);


    // Seek
    int8_t seek(int8_t handle, uint32_t newposition);
    int8_t seekToEnd(int8_t handle);


    // Helpers
    void print_P(const char *str);
#if ARDUINO >= 100
    size_t write(uint8_t);
#else
    void write(uint8_t);  // needed for Print
#endif
    
  private:

    // We use polymorphism to interact with a serial class.
    // Stream is an abstract base class which defines a base set
    // of functionality for serial classes.
    // Examples of Wiring serial classes are HardwareSerial,
    // or SoftwareSerial
    Stream *_comms;

    uint8_t _promptChar;
    int16_t _fwVersion;
    moduleType _moduleType;
    uint8_t _fwLevel;
    const char *_prefix;

    // methods
    int8_t _readBlocked(void);
    int16_t _readTimeout(uint16_t timeout);  // Time out = timeout * 0.01 seconds
    int8_t _getResponse(void);
    int16_t _getVersion(void);
    int32_t _getNumber(uint8_t base);

    int8_t _open(int8_t handle, const char *path, openMode mode, int8_t pgmspc);

    uint8_t _commAvailable(void);
    int _commPeek(void);
    int _commRead(void);
};

#endif
// _ROGUESD_H
