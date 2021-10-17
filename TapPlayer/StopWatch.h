#pragma once

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class StopWatch
{
public:
	void Initialize();
	void Reset();
	uint8_t Elapsed();
};

extern StopWatch stopWatch;