/*
  Project: setPixelXY_ReefLightCtrl: A Rainbowduino V3.0 (Arduino based) RGB RGB Matrix controller for use as
  a Reef Aquarium Light controller. 
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

#define MILLISECONDS_IN_SECOND 1000
#define SECONDS_IN_A_MINUTE 60L
#define MINUTES_IN_AN_HOUR 60L
#define HOURS_IN_A_DAY 24L


Time initTime(int hour, int minute, int second){
	return (hour * MINUTES_IN_AN_HOUR * SECONDS_IN_A_MINUTE + minute * SECONDS_IN_A_MINUTE + second);
}

Time incrementTime(Time *time){
	++*time;
	*time %= HOURS_IN_A_DAY * MINUTES_IN_AN_HOUR * SECONDS_IN_A_MINUTE;
}


RGBColor initColor(int red, int green, int blue){
	RGBColor color;
	color.red = red;
	color.green = green;
	color.blue = blue;
	return color;
}

Time currentTime;



boolean isInPeriod(Time time, Period period){
	if (period.end > period.start){
		bool hasStarted = time >= period.start;
		bool hasFinished = time > period.end;
		return (hasStarted && !hasFinished);
	}
	else {
		bool pastStart = time >= period.start;
		bool beforeEnd = time < period.end;
		return (pastStart || beforeEnd);
	}
}

Period initPeriod(Time start, Time end, RGBColor startColor, RGBColor endColor){
	Period period;
	period.start = start;
	period.end = end;
	period.startColor = startColor;
	period.endColor = endColor;
	return period;
}

Time elapsedOfPeriod(Period period, Time time){
	if (period.start > period.end && time > period.end){
		return durationOfPeriod(period) - (period.end - time);
	}
	else {
		return time - period.start;
	}
}

Time durationOfPeriod(Period period){
	if (period.start > period.end){
		return HOURS_IN_A_DAY * MINUTES_IN_AN_HOUR * SECONDS_IN_A_MINUTE - period.start + period.end;
	}
	else {
		return period.end - period.start;
	}
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
	Serial.begin(115200);
  
	// after serial setup, update UI to signal serial communications are ready
	if(Serial) {
		Serial.println("Hi, program started!");
	} 
	
	currentTime = initTime(20,26,0);

	RGBColor dayColor = initColor(65, 105, 225);
	RGBColor nightColor = initColor(3, 4, 9);

	dayPeriods[0] = initPeriod(initTime(3, 30, 0), initTime(4, 30, 0), nightColor, dayColor);//dawn
	dayPeriods[1] = initPeriod(initTime(4, 30, 0), initTime(17, 30, 0), dayColor, dayColor);//day
	dayPeriods[2] = initPeriod(initTime(17, 30, 0), initTime(18, 30, 0), dayColor, nightColor);//dusk
	dayPeriods[3] = initPeriod(initTime(18, 30, 0), initTime(3, 30, 0), nightColor, nightColor);//night
}


void loop()
{
  if (millis() % MILLISECONDS_IN_SECOND == 0) {
	  incrementTime(&currentTime);
	  for (int i = 0; i < NUMBER_OF_PERIODS; i++){
		  if (isInPeriod(currentTime, dayPeriods[i])){
			  Period currentPeriod = dayPeriods[i];

			  double fractionElapsed = ((double)elapsedOfPeriod(currentPeriod, currentTime)) / ((double)durationOfPeriod(currentPeriod));

			  RGBColor delta = initColor(
				  currentPeriod.endColor.red - currentPeriod.startColor.red,
				  currentPeriod.endColor.green - currentPeriod.startColor.green,
				  currentPeriod.endColor.blue - currentPeriod.startColor.blue);

			  RGBColor newColor = initColor(
				  fractionElapsed*delta.red + currentPeriod.startColor.red,
				  fractionElapsed*delta.green + currentPeriod.startColor.green,
				  fractionElapsed*delta.blue + currentPeriod.startColor.blue
				  );

			  setColor(newColor);
			  return;
		  }
	  }
	Serial.println("You screwed up your periods");
  }
}