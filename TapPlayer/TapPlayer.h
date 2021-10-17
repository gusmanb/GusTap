// TapPlayerLibrary.h

#ifndef _TapPlayer_h
#define _TapPlayer_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

typedef enum IDLE_STAGE
{
	IDLE_NONE = 0,
	IDLE_PLAYING_BLOCK = 1,
	IDLE_WAITING_NEXT_BLOCK = 2,
	IDLE_PAUSE = 3
};

typedef uint16_t (*TapReadLengthCallback)(void);
typedef uint8_t(*TapReadDataCallback)(void);
typedef void (*TapIdleCallback)(IDLE_STAGE);

class TapPlayer
{

public:
	bool Initialize(TapReadLengthCallback ReadLength, TapReadDataCallback ReadData, TapIdleCallback IdleCallback, uint8_t OutputPin);
	bool Play();
	void Stop();
	void Pause();
	void Resume();
	void Skip();
	void PulseHandler();
private:
	int16_t bufferA[65];
	int16_t bufferB[65];
	volatile int16_t* currentBuffer;
	volatile uint8_t currentBufferId;
	uint8_t nextBufferId;

	volatile bool swapBuffer = false;
	volatile bool blockFinished = false;
	volatile bool stop = false;
	volatile bool pause = false;

	volatile int16_t currentPeriod;
	volatile int16_t currentPulseCount;
	volatile uint8_t currentSense = 0;

	TapReadLengthCallback readLength;
	TapReadDataCallback readData;
	TapIdleCallback idle;
	uint8_t outputPin;

	void StartBufferPlay();
	void StopBufferPlay();
	byte StorePreface(int16_t* buffer, bool DataBlock);
	uint8_t StoreByte(int16_t* buffer, uint8_t data);
	uint16_t StoreBuffer(uint8_t BufferSize, int16_t* Buffer, uint16_t LengthLeft);
	bool PlayBlock(uint16_t Length, uint8_t DataType);
	bool skipBlock;
};

extern TapPlayer tapPlayer;

#endif

