// 
// 
// 

#include "TapPlayer.h"
#include "TapPlayerDefines.h"
#include <TimerOne.h>
#include "StopWatch.h"

TapPlayer tapPlayer;

void TimerISR()
{
	tapPlayer.PulseHandler();
}

void TapPlayer::PulseHandler()
{
	currentSense = !currentSense;
	digitalWrite(outputPin, currentSense);
	stopWatch.Reset();

	if (currentPulseCount == 0)
	{
		currentPeriod = *currentBuffer;

		if (currentPeriod == -1)
		{
			Timer1.stop();
			blockFinished = true;
			return;
		}
		else if (currentPeriod == -2)
		{
			if (currentBufferId == 0)
			{
				currentBuffer = bufferB;
				currentBufferId = 1;
			}
			else
			{
				currentBuffer = bufferA;
				currentBufferId = 0;
			}

			swapBuffer = true;
			currentPeriod = *currentBuffer;
		}
		
		currentBuffer += 1;
		currentPulseCount = *currentBuffer;
		currentBuffer += 1;
	}

	
	currentPulseCount -= 1;

	Timer1.setPeriod(currentPeriod - stopWatch.Elapsed());
}

void TapPlayer::StartBufferPlay()
{
	currentBufferId = 0;
	currentBuffer = bufferA;

	currentPeriod = *currentBuffer;
	currentBuffer += 1;
	currentPulseCount = *currentBuffer;
	currentBuffer += 1;

	stopWatch.Initialize();

	Timer1.initialize(500000);
	Timer1.attachInterrupt(TimerISR);
	Timer1.stop();
	Timer1.start();
}

void TapPlayer::StopBufferPlay()
{
	Timer1.stop();
}

byte TapPlayer::StorePreface(int16_t* buffer, bool DataBlock) //Quantity of bytes used in the buffer
{
	int16_t* lenPointer = buffer;
	buffer += 1;
	int16_t* countPointer = buffer;
	buffer += 1;

	*lenPointer = PILOTFREQ;
	*countPointer = DataBlock ? PILOTDATACOUNT : PILOTHEADERCOUNT;

	lenPointer = buffer;
	buffer += 1;
	countPointer = buffer;
	buffer += 1;

	*lenPointer = SYNCFIRSTFREQ;
	*countPointer = 1;

	lenPointer = buffer;
	buffer += 1;
	countPointer = buffer;
	buffer += 1;

	*lenPointer = SYNCSECONDFREQ;
	*countPointer = 1;

	return 6;
}

uint8_t TapPlayer::StoreByte(int16_t* buffer, uint8_t data)
{
	bool prevBit;
	bool firstBit = false;

	uint8_t offset = 0;
	int16_t* lenPointer;
	int16_t* countPointer;

	for (int8_t buc = 7; buc > -1; buc--)
	{
		bool currentBit = (data & bit(buc)) && true;

		if (firstBit && (currentBit == prevBit))
			*countPointer += 2;
		else
		{
			lenPointer = buffer + offset;
			countPointer = buffer + offset + 1;
			offset += 2;
			firstBit = true;

			if (currentBit)
				*lenPointer = ONEPULSEFREQ;
			else
				*lenPointer = ZEROPULSEFREQ;

			prevBit = currentBit;
			*countPointer = 2;
		}
	}

	return offset;
}

uint16_t TapPlayer::StoreBuffer(uint8_t BufferSize, int16_t* Buffer, uint16_t LengthLeft)
{
	uint8_t usedWords = 0;

	while (BufferSize - usedWords >= 16 && LengthLeft > 0)
	{
		usedWords += StoreByte(Buffer + usedWords, readData());
		LengthLeft -= 1;
	}

	int16_t* freqPointer = Buffer + usedWords;

	if (LengthLeft == 0)
		*freqPointer = -1;
	else
		*freqPointer = -2;

	return LengthLeft;
}

bool TapPlayer::PlayBlock(uint16_t Length, uint8_t DataType)
{
	blockFinished = false;
	swapBuffer = false;
	nextBufferId = 0;
	currentSense = 0;
	currentPeriod = 0;
	currentPulseCount = 0;
	skipBlock = false;

	digitalWrite(outputPin, 0);
	delay(500);

	uint8_t usedWords = StorePreface(bufferA, DataType);
	usedWords += StoreByte(&bufferA[usedWords], DataType);

	Length = StoreBuffer(64 - usedWords, &bufferA[usedWords], Length);

	if (Length == 0)
	{
		StartBufferPlay();
		while (!blockFinished);
		return true;
	}

	Length = StoreBuffer(64, bufferB, Length);

	if (Length == 0)
	{
		StartBufferPlay();
		while (!blockFinished);
		return true;
	}

	StartBufferPlay();

	while (Length > 0)
	{
		while (!swapBuffer)
		{
			idle(IDLE_PLAYING_BLOCK);
			if (stop)
			{
				StopBufferPlay();
				stop = false;
				return false;
			}
		}

		swapBuffer = false;

		if (skipBlock)
		{
			Timer1.stop();
			while (Length > 0)
			{
				readData();
				Length--;
			}
			blockFinished = true;
		}
		else
		{
			if (nextBufferId == 0)
			{
				Length = StoreBuffer(64, bufferA, Length);
				nextBufferId = 1;
			}
			else
			{
				Length = StoreBuffer(64, bufferB, Length);
				nextBufferId = 0;
			}
		}
	}

	while (!blockFinished);
	return true;
}

bool TapPlayer::Initialize(TapReadLengthCallback ReadLength, TapReadDataCallback ReadData, TapIdleCallback Idle, uint8_t OutputPin)
{
	readLength = ReadLength;
	readData = ReadData;
	idle = Idle;
	outputPin = OutputPin;
	return true;
}

bool TapPlayer::Play()
{
	pinMode(outputPin, OUTPUT);

	uint16_t blockLength;
	uint8_t blockType;

	stop = false;
	pause = false;

	while ((blockLength = readLength()))
	{
		blockType = readData();
		blockLength -= 1;

		if (!PlayBlock(blockLength, blockType))
			return false;
		
		unsigned long tapDelay = millis();

		while (millis() - tapDelay < 2000)
		{
			idle(IDLE_WAITING_NEXT_BLOCK);
			if (stop)
			{
				stop = false;
				return false;
			}
		}

		while (pause)
		{
			idle(IDLE_PAUSE);
			if (stop)
			{
				stop = false;
				return false;
			}
		}
	}

	return true;
}

void TapPlayer::Stop()
{
	stop = true;
}

void TapPlayer::Pause()
{
	pause = true;
}

void TapPlayer::Resume()
{
	pause = false;
}

void TapPlayer::Skip()
{
	skipBlock = true;
}