// TapBrowser.h

#ifndef _TAPBROWSER_h
#define _TAPBROWSER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <SPI.h>
#include <SD.h>

bool TapBrowserInitialize(uint8_t CSPin);
void TapBrowserDeinitialize();
bool TapBrowserChangeDirectory(const char* Path);
uint8_t TapBrowserUpDirectory();
File TapBrowserOpenReadFile(const char* Path);
File TapBrowserOpenWriteFile(const char* Path);
bool TapBrowserDeleteFile(const char* Path);
bool TapBrowserFileExists(const char* Path);
bool TapBrowserBeginDir(char* FileNameBuffer, bool* IsDirectory);
bool TapBrowserContinueDir(char* FileNameBuffer, bool* IsDirectory);
const char* TapBrowserCurrentPath();
bool TapBrowserLongNamesAvailable();
bool TapBrowserGetLongName(const char* FileName);
int8_t TapBrowserReadLongName(char* OutputBuffer, uint8_t FirstChar, int8_t Length);

#endif

