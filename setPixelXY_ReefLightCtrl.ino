/*
  Project: setPixelXY_ReefLightCtrl: A Rainbowduino V3.0 (Arduino based) RGB RGB Matrix controller for use as
  a Reef Aquarium Light. 
  Features: 
  1. Controls a RGB LED array using a time of the day scheduler.
  2. Communicates program status to a serial device (PC)
  3. 
  
  Development Track: 
  start to 08/28/2015: 
	a. Added a basic Time Of The Day Scheduler - counts 1000 ms intervals to calculate seconds / minutes / hours / day
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



long millisecondsInSecond = 1000;



#define HOURS_IN_DAY 24
#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MINUTE 60

TIME initTime(int hour, int minute, int second){
	return hour * SECONDS_IN_HOUR + minute * SECONDS_IN_MINUTE + second;
}

void incrementTime(TIME *time){
	Serial.println(*time);
	*time = *time + 1;
	*time = *time % (SECONDS_IN_HOUR * HOURS_IN_DAY);
}



RGBColor initColor(int red, int green, int blue){
	RGBColor color;
	color.red = red;
	color.green = green;
	color.blue = blue;
	return color;
}

TIME currentTime;



boolean isInPeriod(TIME time, Period period){
	return (
			(period.end > period.start) && //non-crossovers
			(time >= period.start) &&
			(time < period.end)
		) || (
			(period.end < period.start) && ( //cross-overs
				(time >= period.start) ||
				(time < period.end)
			)
		);
}

Period initPeriod(TIME start, TIME end, RGBColor startColor, RGBColor endColor){
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
	for (int x = 0; x<8; x++) {
		for (int y = 0; y<8; y++) {
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
  
  debugFlag = false;
  
  // after serial setup, update UI to signal serial communications are ready
  if(Serial) {
	  Serial.println("Hi, program started!");
	} 
	
	if(debugFlag) {
		millisecondsInSecond = 10; //  really fast! Debugging going on!
	}
	
	currentTime = initTime(3, 30, 0);
	Serial.println(currentTime);

	RGBColor dayColor = initColor(75, 100, 225);
	RGBColor nightColor = initColor(3, 4, 9);

	dayPeriods[0] = initPeriod(initTime(3, 30, 0), initTime(4, 0, 0), nightColor, dayColor);//dawn
	dayPeriods[1] = initPeriod(initTime(4, 0, 0), initTime(16, 0, 0), dayColor, dayColor);//day
	dayPeriods[2] = initPeriod(initTime(16, 0, 0), initTime(16, 30, 0), dayColor, nightColor);//dusk
	dayPeriods[3] = initPeriod(initTime(16, 30, 0), initTime(3, 30, 0), nightColor, nightColor);//night


}


void loop()
{
  if (millis() % millisecondsInSecond == 0) {
	  incrementTime(&currentTime);
	  Serial.println(currentTime);
	  Period currentPeriod;
	  boolean hasFound = false;
	  for (int i = 0; i < NUMBER_OF_PERIODS; i++){
		  if (isInPeriod(currentTime, dayPeriods[i])){
			  hasFound = true;
			  currentPeriod = dayPeriods[i];
		  }
	  }
	  if (hasFound){
		  RGBColor delta = initColor(
			  currentPeriod.endColor.red - currentPeriod.startColor.red,
			  currentPeriod.endColor.green - currentPeriod.startColor.green,
			  currentPeriod.endColor.blue - currentPeriod.startColor.blue);
		  double fractionElapsed;
		  if (currentPeriod.start > currentPeriod.end){
			  TIME duration = SECONDS_IN_HOUR * HOURS_IN_DAY - currentPeriod.start + currentPeriod.end;
			  TIME elapsed;
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
		  Serial.println(fractionElapsed);
		  RGBColor newColor = initColor(
			  fractionElapsed*delta.red + currentPeriod.startColor.red,
			  fractionElapsed*delta.green + currentPeriod.startColor.green,
			  fractionElapsed*delta.blue + currentPeriod.startColor.blue
			  );
		  Serial.println(newColor.red);
		  setColor(newColor);
	  }
	  else {
		  Serial.println("You screwed up your periods");
	  }
  }
}