/*
 Name:		GusTap.ino
 Created:	09/10/2021 23:29:12
 Author:	Gus
*/

#include <SD.h>
#include <Arduino.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiAvrI2c.h>
#include "TapPlayer.h"
#include "TapBrowser.h"

#define I2C_ADDRESS 0x3C
#define RTN_CHECK 1
#define BUTTON_NEXT 6
#define BUTTON_ENTER 5
#define BUTTON_UP 4

#define BLOCK_HEADER 0
#define BLOCK_DATA 0xFF

#define NAMEBUFFER_SIZE 15
#define LONG_NAMEBUFFER_SIZE 22

SSD1306AsciiAvrI2c display;
TickerState state;

char nameBuffer[NAMEBUFFER_SIZE];
char longNameBuffer[LONG_NAMEBUFFER_SIZE];

bool showLongName = false;
uint8_t longNamePos;
uint32_t lastNameMillis;

File* currentFile;
uint32_t blockLength;
uint32_t currentReadLength;
uint16_t currentBlock;
uint16_t lastLength;
uint8_t percent;
IDLE_STAGE currentStage;

uint8_t blockType;

// the setup function runs once when you press reset or power the board
void setup() {

	display.begin(&Adafruit128x32, I2C_ADDRESS);
	display.setFont(System5x7);
	display.clear();
	display.println(F("     GusTap 1.0"));

	pinMode(BUTTON_NEXT, INPUT_PULLUP);					//next file
	pinMode(BUTTON_UP, INPUT_PULLUP);					//up folder
	pinMode(BUTTON_ENTER, INPUT_PULLUP);					//enter

	while (!TapBrowserInitialize(10))
	{
		display.setCursor(0, 2);
		display.print(F("    SD not ready"));
		delay(2000);
	}

	display.setCursor(0, 2);
	display.println(F("       SD OK       "));
	display.setCursor(0, 2);

	delay(500);
}

bool ReadLongName()
{

	if (millis() - lastNameMillis < 500)
		return false;

	memset(longNameBuffer, 0, LONG_NAMEBUFFER_SIZE);

	int8_t result = TapBrowserReadLongName(longNameBuffer, longNamePos, LONG_NAMEBUFFER_SIZE - 1);

	if (result < 0)
		return false;

	if (result < LONG_NAMEBUFFER_SIZE - 1)
		longNamePos = 0;
	else
		longNamePos++;

	lastNameMillis = millis();

	return true;
}

void continueLongName()
{
	if (ReadLongName())
	{
		display.setCursor(0, 3);
		display.clearField(0, 3, 21);
		display.println(longNameBuffer);
	}
}

void displayFileName(bool isDir)
{
	showLongName = false;

	display.setCursor(0, 3);
	display.clearField(0, 3, 21);

	if (isDir)
	{
		display.print(F("/"));
		display.println(nameBuffer);
	}
	else
	{
		if (TapBrowserLongNamesAvailable())
		{
			if (TapBrowserGetLongName(nameBuffer))
			{
				longNamePos = 0;
				lastNameMillis = 0;
				if (ReadLongName())
				{
					showLongName = true;
					display.println(longNameBuffer);
				}
				else
					display.println(nameBuffer);
			}
			else
				display.println(nameBuffer);
		}
		else
			display.println(nameBuffer);

	}

}

void chooseFile()
{
	
	bool tickerActive = false;
	bool browsing = true;
	bool changePath = false;
	const char* currentPath;
	
	bool isDir;
	uint32_t tickTime;
	int n;
	bool empty = false;

	while (browsing)
	{
		display.clear();
		display.println(F("     Browse TAP"));

		tickTime = 0;
		n = 0;
		changePath = false;

		currentPath = TapBrowserCurrentPath();

		display.print(F("Path:"));
		if (strlen(currentPath) > 14)
		{
			display.tickerInit(&state, Adafruit5x7, 1, false, display.fontWidth() * 6, display.fontWidth() * 20);
			tickerActive = true;
		}
		else
		{
			display.print(currentPath);
			tickerActive = false;
		}
		memset(nameBuffer, 0, NAMEBUFFER_SIZE);

		if (!TapBrowserBeginDir(nameBuffer, &isDir))
		{
			display.setCursor(0, 3);
			display.clearField(0, 3, 21);
			display.print(F("--EMPTY--"));
			empty = true;
		}
		else
		{
			displayFileName(isDir);

			empty = false;
		}

		uint32_t tickTime = 0;
		int n = 0;

		while (!changePath)
		{
			if (tickerActive)
			{
				if (tickTime <= millis()) {
					tickTime = millis() + 30;

					// Should check for error. rtn < 0 indicates error.
					int8_t rtn = display.tickerTick(&state);

					// See above for definition of RTN_CHECK.
					if (rtn <= RTN_CHECK) {
						// Should check for error. Return of false indicates error.
						display.tickerText(&state, currentPath);
					}
				}
			}

			if (showLongName)
				continueLongName();

			if (!empty)
			{
			
				bool firstPause = false;
				int32_t startMillis;

				while (!digitalRead(BUTTON_NEXT))
				{
					if (!TapBrowserContinueDir(nameBuffer, &isDir))
					{
						if (TapBrowserBeginDir(nameBuffer, &isDir))
							changePath = true;
					}
					else
					{
						displayFileName(isDir);
					}

					if (!firstPause)
					{
						firstPause = true;
						delay(100);
						startMillis = millis();

						while (!digitalRead(BUTTON_NEXT) && millis() - startMillis < 900);
					}
					else	//while (!digitalRead(BUTTON_NEXT));
						delay(200);

					if (digitalRead(BUTTON_NEXT))
						delay(100);
				}

				if (!digitalRead(BUTTON_ENTER))
				{
					display.setCursor(0, 3);
					display.clearField(0, 3, 21);
					

					if (isDir)
					{
						if(TapBrowserChangeDirectory(nameBuffer))
							changePath = true;
					}
					else
					{
						changePath = true;
						browsing = false;
					}

					delay(50);
					while (!digitalRead(BUTTON_ENTER));
					delay(50);
				}
			}

			if (browsing && !changePath)
			{
				if (!digitalRead(BUTTON_UP))
				{

					if (TapBrowserUpDirectory())
						changePath = true;

					delay(50);
					while (!digitalRead(BUTTON_UP));
					delay(50);
					
				}
			}
		}
	}
}

void showBlockInfo()
{
	display.clear();

	if (blockType == BLOCK_HEADER)
	{
		display.println(F("   Playing header   "));
		display.setCursor(0, 2);
		display.print("  Name: ");
		display.println(nameBuffer);
	}
	else
	{
		display.print(F("Playing block "));
		display.println(currentBlock);
		display.print(blockLength);
		display.println(F(" bytes"));
		display.print("Name: ");
		display.println(nameBuffer);
		display.println(F("0% [----------] 100%"));
	}
}

uint8_t getFileByte()
{
	currentReadLength++;
	return currentFile->read();
}

uint16_t getFileShort()
{
	if (currentFile->available() < 2)
		return 0;

	currentFile->readBytes((char*)&blockLength, 2);
	currentReadLength = 0;
	lastLength = 0;
	percent = 0;
	currentBlock++;

	uint32_t pos = currentFile->position();
	blockType = currentFile->read();

	if (blockType == BLOCK_HEADER)
	{
		currentFile->read();
		memset(nameBuffer, 0, NAMEBUFFER_SIZE);
		currentFile->readBytes(nameBuffer, 10);
	}
	
	showBlockInfo();
	
	currentFile->seek(pos);

	return blockLength;
}

void playActivity(IDLE_STAGE Stage)
{
	switch (Stage)
	{
	case IDLE_PLAYING_BLOCK:

		if (currentStage != Stage)
			currentStage = Stage;

		if (blockType == BLOCK_DATA && currentReadLength != lastLength)
		{
			lastLength = currentReadLength;
			uint8_t currentPercent = min(round((lastLength * 10.0f) / (float)blockLength), 10);

			if (currentPercent != percent)
			{
				percent = currentPercent;
				display.setCursor(5 * display.fontWidth(), 3);
				for (int buc = 1; buc <= percent; buc++)
					display.print(F("#"));
			}
		}

		if (!digitalRead(BUTTON_ENTER))
			tapPlayer.Pause();
		else if (!digitalRead(BUTTON_UP))
			tapPlayer.Stop();
		else if (!digitalRead(BUTTON_NEXT))
			tapPlayer.Skip();

		break;

	case IDLE_WAITING_NEXT_BLOCK:

		if (currentStage != Stage)
		{
			currentStage = Stage;
			display.setCursor(0, 3);
			display.clearToEOL();
			display.setCursor(0, 3);
			display.println(F(" Waiting next block "));
		}

		if (!digitalRead(BUTTON_ENTER))
			tapPlayer.Pause();
		else if (!digitalRead(BUTTON_UP))
			tapPlayer.Stop();
		
		break;

	case IDLE_PAUSE:

		if (currentStage != Stage)
		{
			currentStage = Stage;
			display.setCursor(0, 3);
			display.clearToEOL();
			display.setCursor(0, 3);
			display.println(F("        Paused        "));
		}

		if (!digitalRead(BUTTON_ENTER))
			tapPlayer.Resume();
		else if (!digitalRead(BUTTON_UP))
			tapPlayer.Stop();

		break;
	}
}

void playFile()
{
	display.clear();
	File mainFile = TapBrowserOpenReadFile(nameBuffer);

	if (!mainFile)
	{
		display.println(F("Failed to open file"));
		delay(2000);
		return;
	}

	currentFile = &mainFile;
	currentBlock = 0;
	currentStage = IDLE_NONE;

	if (!tapPlayer.Initialize(getFileShort, getFileByte, playActivity, 3))
	{
		currentFile = NULL;
		mainFile.close();

		display.println(F("Error on player init"));
		delay(2000);
		return;
	}

	if (!tapPlayer.Play())
	{
		currentFile = NULL;
		mainFile.close();

		display.clear();
		display.println(F("Error playing file"));
		delay(2000);
		return;
	}

	currentFile = NULL;
	mainFile.close();

	display.clear();
	display.println(F("Play finished"));
	delay(2000);
}
// the loop function runs over and over again until power down or reset
void loop() {
	chooseFile();
	playFile();
}
