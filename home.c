#include <signal.h> 
#include <unistd.h>
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <errno.h> 
#include <wiringPi.h> 
#include <wiringPiSPI.h> 
#include "softServo.h"

#define CS_MCP3208 8 //GPIO 8 
#define SPI_CHANNEL 0 
#define SPI_SPEED 1000000 //1Mhz

#define trigPin 1	//gpio 21
#define echoPin 29	//gpio 18

#define FLAME_IN 5  //gpio7

#define LEDBAR	26 // BCM_GPIO 12

#define BUZCONTROL  28	//GPIO 20

#define RCSERVO  23	//GPIO 23

#define servopwron 27 //27

// the event counter 
volatile int eventCounter = 0; 

// -------------------------------------------------------------------------
// myInterrupt:  called every time an event occurs
void myInterrupt(void) {
   eventCounter++;

}

// spi communication with Rpi and get sensor data 

int read_light(unsigned char adcChannel) 
{
	unsigned char buff[3];
	int adcValue = 0;
	
	buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
	buff[1] = ((adcChannel & 0x07) << 6);
	buff[2] = 0x00;
	
	digitalWrite(CS_MCP3208, 0);
	wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);
	
	buff[1] = 0x0f & buff[1];
	adcValue = (buff[1] << 8 ) | buff[2];
	
	digitalWrite(CS_MCP3208, 1);
	
	return adcValue;
}

int read_ultraSonic(){
		int distance=0;
		int pulse = 0;
	
		long startTime;
		long travelTime;
		digitalWrite (trigPin, LOW);
		usleep(2);
		digitalWrite (trigPin, HIGH);
		usleep(10);
		digitalWrite (trigPin, LOW);
		
		while(digitalRead(echoPin) == LOW);
		
		startTime = micros();
		
		while(digitalRead(echoPin) == HIGH);
		travelTime = micros() - startTime;
		
		distance = travelTime * 0.034 / 2;
		
		return distance;
		//printf( "Distance: %dcm\n", distance);
		//delay(200);
}

int

int main(void) {

	unsigned char adcChannel_light = 0; // before 3

	int adcValue_light = 0;

	float vout_light;
	float vout_oftemp;
	float percentrh = 0;
	float supsiondo = 0;

	

	printf("start");

	
	if(wiringPiSetupGpio() == -1)
	{
		fprintf(stdout, "Unable to start wiringPi :%s\n", strerror(errno));
		return 1;
	}
	
	if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
	{
		fprintf(stdout, "wiringPiSPISetup Failed :%s\n", strerror(errno));
		return 1;
	}

	if (wiringPiSetup () < 0) {
      fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
      return 1;
 	}

  	// set Pin 17/0 generate an interrupt on high-to-low transitions
  	// and attach myInterrupt() to the interrupt
  	if ( wiringPiISR (FLAME_IN, INT_EDGE_FALLING, &myInterrupt) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  	}
	
	pinMode(CS_MCP3208, OUTPUT);
	pinMode (trigPin, OUTPUT);
	pinMode (echoPin, INPUT);
	pinMode (LEDBAR, OUTPUT) ;
	pinMode (BUZCONTROL, OUTPUT);
	pinMode(servopwron, OUTPUT);

	while(1)
	{
		
		int jarak = read_ultraSonic();
		int lightValue = read_light(adcChannel_light);
		if(jarak <= 3)
		{
			printf( "Distance: %dcm\n", jarak);
			printf("servo on \n");
        	delay(1000);
			softServoWrite(RCSERVO, -250);
			delay(3000);
		}else if(jara > 3)
		{
			printf( "Distance: %dcm\n", jarak);
			printf("servo off \n");
			softServoWrite(RCSERVO, 1250);
			delay(3000);   
		}

		if(lightValue < 500)
		{
			printf("light sensor = %u\n", lightValue);
			printf("Led Bar off\n");
			digitalWrite (LEDBAR, 0) ;
		}else if(lightValue > 500)
		{
			printf("light sensor = %u\n", lightValue);
			printf("Led Bar on\n");
			digitalWrite (LEDBAR, 1) ;

		}

		if(eventCounter == 1)
		{
			printf( "%d\n", eventCounter );
			printf("Buzzer on\n");
			digitalWrite(BUZCONTROL, 1);
		}else if(eventCounter == 0)
		{
			printf( "%d\n", eventCounter );
			printf("Buzzer off\n");
			digitalWrite(BUZCONTROL, 0);
		}
		delay(100);
	}
	return 0;
}
