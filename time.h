// time.h

#ifndef _TIME_h
#define _TIME_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

typedef long Time;

typedef struct {
	int red;
	int green;
	int blue;
} RGBColor;

typedef struct {
	Time start;
	Time end;
	RGBColor startColor;
	RGBColor endColor;
} Period;

#endif

