#include "StopWatch.h"

StopWatch stopWatch;

void StopWatch::Initialize()
{
	PRR &= ~(1 << PRTIM2);
	GTCCR = 0;
	TCCR2A = 0;
	TCCR2B = 0b00000010; //ckio/8, 0.5uS
	TIMSK2 = 0;

}

void StopWatch::Reset()
{
	TCNT2 = 0;
}

uint8_t StopWatch::Elapsed()
{
	return TCNT2 / 2;
}