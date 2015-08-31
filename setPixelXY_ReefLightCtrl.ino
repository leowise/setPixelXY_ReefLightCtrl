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

#include <Rainbowduino.h>

// TODO: Needed in this project?
uint32_t colorRGB[13] = {0xFFFFFF,0x000000,0xFFFFFF,0x000,0xFF0000,0x00FF00,0x0000FF,0xFF0000,0x00FF00,0x0000FF,0xFF0000,0x00FF00,0x0000FF };
unsigned char x,y,z;

// utility
boolean lightOn = false;
boolean runOnce = false;
boolean reportTime = false;
boolean debugFlag = false;



long millisecondsInSecond = 1000;

typedef int Time;

#define HOURS_IN_DAY 24
#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MINUTE 60

Time initTime(int hour, int minute, int second){
	return hour * SECONDS_IN_HOUR + minute * SECONDS_IN_MINUTE + second;
}

void incrementTime(Time *time){
	(*time)++;
	*time &= SECONDS_IN_HOUR * HOURS_IN_DAY;
}

typedef struct {
	int red;
	int green;
	int blue;
} RGBColor;

RGBColor initColor(int red, int green, int blue){
	RGBColor color;
	color.red = red;
	color.green = green;
	color.blue = blue;
}

Time currentTime;

typedef struct {
	Time start;
	Time end;
	RGBColor startColor;
	RGBColor endColor;
} Period;

boolean isInPeriod(Time time, Period period){
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

Period initPeriod(Time start, Time end, RGBColor startColor, RGBColor endColor){
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
	for (x = 0; x<8; x++) {
		for (y = 0; y<8; y++) {
			Rb.setPixelXY(x, y, color.red, color.green, color.blue);
		}
	}
}



// light control - ramp step for each color to maintain the Royal Blue parameters
int R_Step = 3;
int G_Step = 4;
int B_Step = 9;

int RRBMax = 75;   // Royal Blue color - Red max intensity - ideal 65
int GRBMax = 100;  // Royal Blue color - Green max intensity -ideal 105
int BRBMax = 225;  // Royal Blue color - Blue max intensity -ideal 225

int rampSteps = 25;
int dawnStartRGB[3] = {3, 4, 9};			// initialized with dawn starting level!
int duskStartRGB[3] = {75, 100, 225};		// initialized with day RGB level!

char level = 'l';		// light levels for moon light (l, m, h)
String TOD = "";		// time of the day string

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
	
	if(debugFlag) {
		millisecondsInSecond = 10; //  really fast! Debugging going on!
		TOD = "debug";
		tod = dawnStart;
	}
	
	currentTime = initTime(3, 30, 0);

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
			  Time duration = SECONDS_IN_HOUR * HOURS_IN_DAY - currentPeriod.start + currentPeriod.end;
			  Time elapsed;
			  if (currentTime > currentPeriod.end){
				  elapsed = duration - (currentPeriod.end - currentTime);
			  }
			  else {
				  elapsed = currentTime - currentPeriod.start;
			  }
			  fractionElapsed = (double)elapsed / (double)duration;
		  }
		  else {
			  fractionElapsed = ((double)(currentTime - currentPeriod.start)) / ((double)(currentPeriod.start - currentPeriod.end));
		  }
		  setColor(initColor(
			  fractionElapsed*delta.red + currentPeriod.startColor.red,
			  fractionElapsed*delta.green + currentPeriod.startColor.green,
			  fractionElapsed*delta.blue + currentPeriod.startColor.blue
			  )
			);
	  }
	  else {
		  Serial.println("You screwed up your periods");
	  }
  }
}