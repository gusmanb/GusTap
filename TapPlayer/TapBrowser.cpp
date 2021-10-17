// 
// 
// 

#include "TapBrowser.h"

char currentPath[257];
File currentPointer;
File longNameCatalog;

void checkLongNameIndex()
{
	if (longNameCatalog)
		longNameCatalog.close();

	longNameCatalog = TapBrowserOpenReadFile("LNAM.IDX");
}

bool TapBrowserInitialize(uint8_t CSPin)
{
	memset(currentPath, 0, 257);
	SD.end();

	if (SD.begin(CSPin))
	{
		return TapBrowserChangeDirectory("/");
	}

	return false;
}

void TapBrowserDeinitialize()
{
	currentPointer.close();
	currentPointer = File();
	longNameCatalog.close();
	longNameCatalog = File();
	SD.end();
}

bool TapBrowserChangeDirectory(const char* Path)
{
	if (Path[0] == '/')
	{
		if (strcmp("/", Path) && !SD.exists(Path))
			return false;

		File cp = SD.open(Path);

		if (!cp)
			return TapBrowserChangeDirectory(currentPath);
			
		currentPointer.close();
		currentPointer = cp;

		strcpy(currentPath, Path);
		size_t len = strlen(currentPath);

		if (len > 1 && currentPath[len - 1] == '/')
			currentPath[len - 1] = 0;

		checkLongNameIndex();

		return true;
	}
	else
	{
		int oldLen = strlen(currentPath);

		if(strlen(currentPath) > 1)
			strcat(currentPath, "/");

		strcat(currentPath, Path);

		if (!SD.exists(currentPath))
		{
			currentPath[oldLen] = 0;
			return false;
		}
		
		File cp = SD.open(currentPath);

		if (!cp)
		{
			currentPath[oldLen] = 0;
			TapBrowserChangeDirectory(currentPath);
			return false;
		}

		currentPointer.close();
		currentPointer = cp;

		oldLen = strlen(currentPath);

		if (oldLen > 1 && currentPath[oldLen - 1] == '/')
			currentPath[oldLen - 1] = 0;

		checkLongNameIndex();

		return true;
	}
}

uint8_t TapBrowserUpDirectory()
{

	size_t len = strlen(currentPath);

	if (len < 2)
		return 1;
	
	const char* lastPos = strrchr(currentPath, '/');

	if (lastPos)
	{
		uint16_t index = lastPos - currentPath;

		if (index == 0)
			index = 1;

		currentPath[index] = 0;
		TapBrowserChangeDirectory(currentPath);
		return index;
	}

	return 2;
}

File TapBrowserOpenReadFile(const char* Path)
{
	if (Path[0] == '/')
		return SD.open(Path, FILE_READ);
	else
	{

		int oldLen = strlen(currentPath);
		strcat(currentPath, "/");
		strcat(currentPath, Path);
		File t = SD.open(currentPath, FILE_READ);
		currentPath[oldLen] = 0;
		return t;

		/*currentPointer.rewindDirectory();

		File t = currentPointer.openNextFile(FILE_READ);

		while (t)
		{
			const char* tName = t.name();

			if (!strcmp(tName, Path))
				return t;

			t.close();

			t = currentPointer.openNextFile(FILE_READ);
		}

		return File();*/
	}
}

File TapBrowserOpenWriteFile(const char* Path)
{
	if (Path[0] == '/')
		return SD.open(Path, FILE_WRITE);
	else
	{
		currentPointer.rewindDirectory();

		File t = currentPointer.openNextFile(FILE_WRITE);

		while (t)
		{
			const char* tName = t.name();

			if (!strcmp(tName, Path))
				return t;

			t.close();

			t = currentPointer.openNextFile(FILE_WRITE);
		}
	}
}

bool TapBrowserDeleteFile(const char* Path)
{
	if (Path[0] == '/')
		return SD.remove(Path);
	else
	{
		int oldLen = strlen(currentPath);
		strcat(currentPath, Path);

		bool result = SD.remove(currentPath);
		currentPath[oldLen] = 0;
		return result;
	}
}

bool TapBrowserFileExists(const char* Path)
{
	if (Path[0] == '/')
		return SD.exists(Path);
	else
	{
		int oldLen = strlen(currentPath);
		strcat(currentPath, Path);

		bool result = SD.exists(currentPath);
		currentPath[oldLen] = 0;
		return result;
	}
}

bool TapBrowserBeginDir(char* FileNameBuffer, bool* IsDirectory)
{
	currentPointer.rewindDirectory();
	return TapBrowserContinueDir(FileNameBuffer, IsDirectory);
}

bool TapBrowserContinueDir(char* FileNameBuffer, bool* IsDirectory)
{
	File current = currentPointer.openNextFile(FILE_READ);
	if (!current)
		return false;
	else
	{
		while (current)
		{
			strcpy(FileNameBuffer, current.name());
			*IsDirectory = current.isDirectory();
			current.close();

			if (*IsDirectory)
				return true;
			
			char* dot = strrchr(FileNameBuffer, '.');

			if (dot && !strcmp(dot, ".TAP"))
				return true;
			else
				current = currentPointer.openNextFile();
		}

		return false;
	}
}

const char* TapBrowserCurrentPath()
{
	return currentPath;
}

bool TapBrowserLongNamesAvailable()
{
	return longNameCatalog;
}

int32_t longNameStart;

bool TapBrowserGetLongName(const char* FileName)
{
	longNameStart = -1;

	if (!longNameCatalog)
		return false;

	char tempBuffer[15];
	char readChar;
	int pos = 0;

	memset(tempBuffer, 0, 15);
	tempBuffer[0] = '*';
	strcat(tempBuffer, FileName);
	strcat(tempBuffer, ":");

	if (!longNameCatalog.find(tempBuffer, strlen(tempBuffer)))
	{
		longNameCatalog.seek(0);

		if (!longNameCatalog.find(tempBuffer, strlen(tempBuffer)))
			return -2;
	}

	longNameStart = longNameCatalog.position();
	return true;
}

int8_t TapBrowserReadLongName(char* OutputBuffer, uint8_t FirstChar, int8_t Length)
{
	if (longNameStart < 0)
		return -1;

	char readChar;
	int pos = 0;

	longNameCatalog.seek(longNameStart);

	while (pos < FirstChar)
	{
		pos++;
		readChar = longNameCatalog.read();
		if (readChar == '?')
			return 0;
	}

	pos = 0;

	while (pos < Length)
	{
		readChar = longNameCatalog.read();

		if (readChar == '?')
			return pos;

		OutputBuffer[pos++] = readChar;
	}

	return pos;
}

//int8_t TapBrowserGetLongName(const char* FileName, char* OutputBuffer, uint8_t FirstChar, int8_t Length)
//{
//	if (!longNameCatalog)
//		return -1;
//
//	char tempBuffer[15];
//	char readChar;
//	int pos = 0;
//
//	memset(tempBuffer, 0, 15);
//	tempBuffer[0] = '*';
//	strcat(tempBuffer, FileName);
//	strcat(tempBuffer, ":");
//
//	if (!longNameCatalog.find(tempBuffer, strlen(tempBuffer)))
//	{
//		longNameCatalog.seek(0);
//
//		if (!longNameCatalog.find(tempBuffer, strlen(tempBuffer)))
//			return -2;
//	}
//
//	while (pos < FirstChar)
//	{
//		pos++;
//		readChar = longNameCatalog.read();
//		if (readChar == '?')
//			return 0;
//	}
//
//	pos = 0;
//
//	while (pos < Length)
//	{
//		readChar = longNameCatalog.read();
//
//		if (readChar == '?')
//			return pos;
//
//		OutputBuffer[pos++] = readChar;
//	}
//
//	return pos;
//}
