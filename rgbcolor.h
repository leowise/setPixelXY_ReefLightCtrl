// rgbcolor.h

#ifndef _RGBCOLOR_h
#define _RGBCOLOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

typedef struct {
	int red;
	int green;
	int blue;
} RGBColor;

#endif

