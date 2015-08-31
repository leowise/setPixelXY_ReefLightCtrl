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

typedef struct {
	int hour;
	int minute;
	int second;
} Time;


Time initTime(int hour, int minute, int second){
	Time time;
	time.hour = hour;
	time.minute = minute;
	time.second = second;
	return time;
}

boolean timeEqual(Time a, Time b){
	return a.hour == b.hour && a.minute == b.minute && a.second == b.second;
}

void incrementTime(Time *time){
	time->second++;
	if (time->second == 60){
		time->second = 0;
		time->minute++;
		if (time->minute == 60){
			time->minute = 0;
			time->hour++;
			if (time->hour == 24){
				time->hour = 0;
			}
		}
	}
}

Time currentTime;
Time dawnStartTime;
Time duskStartTime;


typedef struct {
	Time start;
	Time end;
} Period;

boolean isInPeriod(Time time, Period period){
	return 
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
	dawnStartTime = initTime(3, 30, 0);
	duskStartTime = initTime(16, 30, 0);

	GetTod();
}


void loop()
{
  if (millis() % millisecondsInSecond == 0) {
	  incrementTime(&currentTime);

	  if (timeEqual(currentTime, dawnStartTime)){

	  }


	  everySecTask();

	  if (currentTime.second == 0){
		  everyMinTask();
	  }

	  if (currentTime.minute == 0){
		  everyHrTask();
	  }
  }
}

String GetTod() {

	

	if ((TimeHMS[0] == dawnStartTime[0]) && (TimeHMS[1] == dawnStartTime[1])) {
		TOD = "dawnStart";
		tod = dawnStart;

	}
	else if ((TimeHMS[0] == duskStartTime[0]) && (TimeHMS[1] == duskStartTime[1])) {
		TOD = "duskStart";
		tod = duskStart;

	}
	else if (tod == dawnStart && TimeHMS[1] <= 30) {
		// day time 
		TOD = "dawn";
		tod = dawn;


	}
	else if (tod == dawnStart && TimeHMS[1] >= 59) {
		// day time 
		TOD = "day";
		tod = day;

	}
	else if (tod == duskStart && TimeHMS[1] <= 30) {
		// night time 
		TOD = "dusk";
		tod = dusk;
	}
	else if (tod == duskStart && TimeHMS[1] >= 59) {
		// day time 
		TOD = "night";
		tod = night;

	}

	if (debugFlag) {
		Serial.println("-------------");
		Serial.println("GetTod() says:");
		Serial.println("TimeHMS: ");
		for (int i = 0; i < 3; i++) {
			Serial.println(TimeHMS[i]);
		}

		Serial.println("-------");

		Serial.println(TOD);

	}



	return TOD;
}


// sets the moon light to a given level
void setMoonLight(char level) {
  
  int R, G, B = 0;
  
  if(level = 'l') {
	R = 2;
	G = 5;
	B = 10;
   
  }
  else if(level == 'm') {
	R = 4;
	G = 10;
	B = 20;
  }
  else if(level == 'h') {
	R = 8;
	G = 20;
	B = 40;
  }
  else {
	// TODO: this is an error - handle it here
  }
  
  for(x=0;x<8;x++)
	{
	 for(y=0;y<8;y++)
	  {
		Rb.setPixelXY(x, y, R, G, B);
	  }
	}
}

void everySecTask() {
  /*
  if(reportTime) {
	  Serial.print("seconds: ");
	  Serial.println(seconds);          
	  Serial.print("minutes: ");
	  Serial.println(minutes);
	  Serial.print("hours: ");
	  Serial.println(hours);
	}
	*/
}

// using the tod information we calculate and set the LEDs RGB parameters 
void lightSequence() {

  switch(tod) { 

	case dawnStart:
		// we switch from night light to dawnStart light
		for (x = 0; x<8; x++) {
			for (y = 0; y<8; y++) {
				Rb.setPixelXY(x, y, dawnStartRGB[0], dawnStartRGB[1], dawnStartRGB[2]);
			}
		}
	break;
	case dawn:
		if (rampSteps >= 0) {
			for(x=0;x<8;x++) {
				for(y=0;y<8;y++) {
					Rb.setPixelXY(x, y, dawnStartRGB[0], dawnStartRGB[1], dawnStartRGB[2]);
				}
			}

			if(dawnStartRGB[2] < BRBMax) {
				dawnStartRGB[0] += R_Step;
				dawnStartRGB[1] += G_Step;
				dawnStartRGB[2] += B_Step;
			}
			else {

				dawnStartRGB[0] = R_Step;
				dawnStartRGB[1] = G_Step;
				dawnStartRGB[2] = B_Step;

			}
		}
		 
	break;
	case day:
		//handle day time
	break;
	case duskStart:
		// handle start of dusk time
		break;
	case dusk:
		// handle dusk time 
	break;
	case night:
		//handle night time
	break;
	default:
		Serial.print("switch(_tod) default reached! revise your code!");

  }

  
 
 
}

void duskLightSequence() {
  
}

void dayLightSequence() {
  
}

void nightLightSequence() {
  
}

void everyMinTask() {

	// since everything depends on the time of the day, we call tod and go from there!
	GetTod();
	lightSequence();
  
	//if (rampSteps >= 0) {
	//
	//if(reportTime) {
	//  Serial.print("seconds: ");
	//  Serial.println(TimeHMS[2]);          
	//  Serial.print("minutes: ");
	//  Serial.println(TimeHMS[1]);
	//  Serial.print("hours: ");
	//  Serial.println(TimeHMS[0]);
	//}
	//
	//for(x=0;x<8;x++)
	//{
	// for(y=0;y<8;y++)
	//  {
	//	  Rb.setPixelXY(x, y, dawnStartRGB[0], dawnStartRGB[1], dawnStartRGB[2]);
	//  }
	//}
	//if (dawnStartRGB[2] < BRBMax) {
	//	dawnStartRGB[0] += R_Step;
	//	dawnStartRGB[1] += G_Step;
	//	dawnStartRGB[2] += B_Step;
	//}
	//else {
	// 
	//	dawnStartRGB[0] = R_Step;
	//	dawnStartRGB[1] = G_Step;
	//	dawnStartRGB[2] = B_Step;
	// rampSteps = 25;
	// 
	//}

  //}
  
	//rampSteps--;
  
  if(debugFlag) {
	  GetTod();
   Serial.print("dawnDuskRamp: ");
   Serial.println(rampSteps);
   Serial.print("dawnRGB: ");
   Serial.print("R:");
   Serial.print(dawnStartRGB[0]);
   Serial.print(" G:");
   Serial.print(dawnStartRGB[1]);
   Serial.print(" B:");
   Serial.println(dawnStartRGB[2]);
   Serial.print("Time: ");
   Serial.print("H:");
   Serial.print(TimeHMS[0]);
   Serial.print(" M:");
   Serial.print(TimeHMS[1]);
   Serial.print(" S:");
   Serial.println(TimeHMS[2]);
   Serial.print("Time Of Day: ");
   Serial.println(TOD);
   Serial.println("---------------");
  }
  
}

void everyHrTask() { 
  
}

void everyDayTask() {
  
}
