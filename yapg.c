// evan kahn
// e155, final project

#include "easyPIO.h"
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>

#define PI 3.14159265

#define LEDOUT 1 // turn on for LED output
#define DEBUG 0  // turn on for debug prints
#define RPM 500  // set rotation speed here

unsigned char pixels, revs, total_frames, speed;

struct pixel {
	unsigned char r, g, b;
};

//help with pixel_data structure from eric mueller; adapted from following code:
//   	https://gist.github.com/hmc-cs-emueller/c4ebbe9f3b4064fed16a

struct pixel *getPixel(struct pixel *frames, unsigned i, unsigned j, unsigned k){
	unsigned addr = i + j * pixels + k * pixels * revs;
	return frames + addr;
}

// details on gamma correction here:
// https://learn.adafruit.com/led-tricks-gamma-correction/the-quick-fix

unsigned char gamma_correct(unsigned char color){
	int corrected = pow(color,3)/(255*255);
	return (unsigned char) corrected;
}


void setLED (struct pixel *p){
	if (LEDOUT){
		spiSendReceive(0xff); // full brightness; sending partial brightness on APA102 is pointless
		spiSendReceive(gamma_correct(p->b));
		spiSendReceive(gamma_correct(p->g));
		spiSendReceive(gamma_correct(p->r));
	}
}

// LED information here:
// https://cdn-shop.adafruit.com/datasheets/APA102.pdf

void initFrame (){
	int i;
	if (LEDOUT) for (i=0;i<4;i++) spiSendReceive(0x00);
}

void endFrame (){
	if (LEDOUT){ // end SPI frame
		spiSendReceive(0xff);
		spiSendReceive(0xff);
		spiSendReceive(0xff);
		spiSendReceive(0xff);
	}
}


void lightUp(struct pixel *frames, int revnum, int framenum){
	int i;
	initFrame();
	struct pixel *p;
	if (DEBUG == 1) printf("\nbegin horiz %d\n", revnum);
	for (i=0;i<pixels;i++){	
		p = getPixel(frames, i, revnum, framenum);
		if (DEBUG){
			printf("%02d 0x%02x%02x%02x ",i,p->b,p->g,p->r);
		}
		setLED(p);
		if (DEBUG && ((i+1) % 8 == 0)) printf("\n");
	}
	endFrame();

}

void sighandler (int sig){ // if user hits ctrl-C during operation, shut down the globe
		if (LEDOUT){
			digitalWrite(22, 0);
			delayMicros(100000);
			int i;
			struct pixel p;
			p.r = 0x00;
			p.g = 0x00;
			p.b = 0x00;
			initFrame();
			for (i = 0; i < pixels; ++i){
				setLED(&p); // turn off all LEDs
			}
			endFrame();
		}
		exit(0);
}

int main() {
	signal(SIGINT, sighandler);
	unsigned int time_1;
	if (LEDOUT){ // if we're writing to LEDs and not debugging
		pioInit();
		spiInit(10000000); // initialize SPI output to LED strip
		pinMode(22, OUTPUT); // pin 22 tells the FPGA to turn the motor on
		pinMode(27, OUTPUT); // pin 27 resets the FPGA
		digitalWrite(27, 1); // reset the FPGA
		delayMicros(10000);
		digitalWrite(27, 0);
	}


	//	frames per revolution: revs
	// 	revolutions per second: revs/60
	// 	seconds per revolution: 60/RPM
	// 	seconds per frame: 60/revs/RPM
	// 	microseconds per frame: 1000000 * 60 / (revs * RPM)

	unsigned revolutions = 0;
	struct pixel *frames;

	char fname[64];
	int done = 0;
	FILE *fp;
	do{
		printf("Enter filename: ");
		scanf("%s",fname);
		strcat(fname, ".led");

	if( access( fname, F_OK ) != -1 ) {
		fp = fopen(fname, "r");
		fscanf(fp, "%c%c%c%c", &pixels, &revs, &speed, &total_frames);
		char junk;
		fscanf(fp, "%c%c%c%c", &junk, &junk, &junk, &junk);
		unsigned long size = pixels * revs * total_frames * sizeof(struct pixel);
		frames = malloc(size);
		if (!frames){
			printf("malloc failed");
			exit(1);
		}
		memset(frames, 0x00, size);
		fread(frames, sizeof(char), size, fp); // load frame data into memory
		done = 1;

	}else{
		printf("not found.\n");
	}
	}while (!done);

	digitalWrite(22,1);

	int revnum = revs - 1; // revs counts the number of vertical lines in one full revolution
	int delay = 1000000 * 60 / (revs * RPM); // constant delay based on motor speed and revs
	int framenum = 0; // if we are displaying an animation, keep track of what frame we're on
	while(1){
		time_1 = getTime();
		lightUp(frames,revnum, framenum);

		if (revnum == 0) revnum = revs - 1;
		else revnum = revnum - 1;

		if (revnum == 0){
			++revolutions;
		}
		if (revolutions == speed){ // speed variable controls the number of times one frame of an animation is displayed 
			revolutions = 0;
			framenum = ((framenum + 1)%total_frames); // increment frame
		}
		while (getTime() < time_1 + delay){}; // wait until next line should be displayed
	}

	printf("\n");
	return(0);
}
