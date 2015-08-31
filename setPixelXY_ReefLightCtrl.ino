/*
  Project: setPixelXY_ReefLightCtrl: A Rainbowduino V3.0 (Arduino based) RGB RGB Matrix controller for use as
  a Reef Aquarium Light. 
  Features: 
  1. Controls a RGB LED array using a time of the day scheduler.
  2. Communicates program status to a serial device (PC)
  3. 
  
  Development Track: 
  start to 08/28/2015: 
	a. Added a basic Time Of The Day Scheduler - counts 1000 ms longervals to calculate seconds / minutes / hours / day
	b. Added light control - controls the LED array to ramp the light output 
	c. Added serial communication with a PC client

*/

#include "time.h"
#include <Rainbowduino.h>



// utility
boolean lightOn = false;
boolean runOnce = false;
boolean reportTime = false;
boolean debugFlag = false;



#define MILLISECONDS_IN_SECOND 100


long initTime(long hour, long minute, long second){
	return (hour * 60L*60L + minute * 60L + second);
}


RGBColor initColor(long red, long green, long blue){
	RGBColor color;
	color.red = red;
	color.green = green;
	color.blue = blue;
	return color;
}

long currentTime;



boolean isInPeriod(long time, Period period){
	Serial.println(period.start);
	Serial.println(period.end);
	if (period.end > period.start){
		bool hasStarted = time > period.start;
		bool hasFinished = time > period.end;
		Serial.print("hasStarted");
		Serial.println(hasStarted);
		Serial.print("hasFinished");
		Serial.println(hasFinished);
		return (hasStarted && !hasFinished);
	}
	else {
		Serial.println("sadfj");
		bool pastStart = time > period.start;
		bool beforeEnd = time < period.end;
		return (pastStart || beforeEnd);
	}
}

Period initPeriod(long start, long end, RGBColor startColor, RGBColor endColor){
	Period period;
	period.start = start;
	period.end = end;
	period.startColor = startColor;
	period.endColor = endColor;
	return period;
}

#define NUMBER_OF_PERIODS 4

Period dayPeriods[NUMBER_OF_PERIODS];

void setColor(RGBColor color){
	for (long x = 0; x<8; x++) {
		for (long y = 0; y<8; y++) {
			Rb.setPixelXY(x, y, color.red, color.green, color.blue);
		}
	}
}


void setup()
{
  Rb.init();
  
  // enable and test serial communications
  Serial.begin(9600);
  
  // Setup debugging and other flags
  reportTime = true;
  
  debugFlag = true;
  
  // after serial setup, update UI to signal serial communications are ready
  if(Serial) {
	  Serial.println("Hi, program started!");
	} 
	
	currentTime = initTime(0,0,0);

	RGBColor dayColor = initColor(0, 255, 0);
	RGBColor nightColor = initColor(255, 0, 0);

	dayPeriods[0] = initPeriod(initTime(1, 30, 0), initTime(5, 0, 0), nightColor, dayColor);//dawn
	dayPeriods[1] = initPeriod(initTime(5, 0, 0), initTime(13, 0, 0), dayColor, dayColor);//day
	dayPeriods[2] = initPeriod(initTime(13, 0, 0), initTime(17, 30, 0), dayColor, nightColor);//dusk
	dayPeriods[3] = initPeriod(initTime(17, 30, 0), initTime(1, 30, 0), nightColor, nightColor);//night
	

	Serial.println(initTime(14, 0, 0));
	Serial.println(isInPeriod(initTime(14, 0, 0), dayPeriods[1]));
}


void loop()
{
  if (millis() % MILLISECONDS_IN_SECOND == 0) {
	  if (debugFlag){
		  currentTime += 1000;
	  }
	  else {
		  currentTime++;
	  }
	  if (currentTime > 60L * 60L * 24L){
		  currentTime = 0;
	  }
	  Serial.println(currentTime);
	  Period currentPeriod;
	  boolean hasFound = false;
	  for (long i = 0; i < NUMBER_OF_PERIODS; i++){
		  if (isInPeriod(currentTime, dayPeriods[i])){
			  hasFound = true;
			  currentPeriod = dayPeriods[i];
			  Serial.println(i);
		  }
	  }
	  if (hasFound){
		  RGBColor delta = initColor(
			  currentPeriod.endColor.red - currentPeriod.startColor.red,
			  currentPeriod.endColor.green - currentPeriod.startColor.green,
			  currentPeriod.endColor.blue - currentPeriod.startColor.blue);
		  double fractionElapsed;
		  if (currentPeriod.start > currentPeriod.end){
			  long duration = 60L * 60L * 24L - currentPeriod.start + currentPeriod.end;
			  long elapsed;
			  if (currentTime > currentPeriod.end){
				  elapsed = duration - (currentPeriod.end - currentTime);
			  }
			  else {
				  elapsed = currentTime - currentPeriod.start;
			  }
			  fractionElapsed = (double)elapsed / (double)duration;
		  }
		  else {
			  fractionElapsed = ((double)(currentTime - currentPeriod.start)) / ((double)(currentPeriod.end - currentPeriod.start));
		  }

		  RGBColor newColor = initColor(
			  fractionElapsed*delta.red + currentPeriod.startColor.red,
			  fractionElapsed*delta.green + currentPeriod.startColor.green,
			  fractionElapsed*delta.blue + currentPeriod.startColor.blue
			  );

		  setColor(newColor);
	  }
	  else {
		  Serial.println("You screwed up your periods");
	  }
  }
}