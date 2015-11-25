// evan kahn
// e155, final project

#include "easyPIO.h"
#include <math.h>
#define PI 3.14159265

#define LEDS 72
#define COLORS 3
#define RPM 500
#define FRAMES 40
#define RGB(r, g, b) ((struct Pixel) {r, g, b})

#define RED RGB(255, 0, 0)
#define GREEN RGB(0, 255, 0)
#define BLUE RGB(0, 0, 255)

#define WHITE RGB(255,255,255)
#define WHITE_64 RGB(64,64,64)

struct Pixel {
	char r;
	char g;
	char b;
};

void setLED (struct Pixel p){
	//printf("%x%x%x",p.b,p.g,p.r);
	spiSendReceive(0xff);
	spiSendReceive(p.b);
	spiSendReceive(p.g);
	spiSendReceive(p.r);
}

void initFrame (){
	int i;
	for (i=0;i<4;i++) spiSendReceive(0x00);
}

void endFrame (){
	struct Pixel p = {255,255,255};
	setLED(p);
}


void writeFrame(struct Pixel frames[FRAMES][LEDS], int framenum){
	int i;
	initFrame();
	for (i=0;i<LEDS+1;i++){	
		setLED(frames[framenum][i]);
	}
	endFrame();
}

void updateSine(struct Pixel frames[FRAMES][LEDS], int framenum){
	int i;
	for (i = 0; i < FRAMES; ++i){
		frames[i][(int)((LEDS/2-1) * (1 + sin(PI * i * 2 / (FRAMES/2.0))))] = BLUE;
	}
}

int main() {

	pioInit();
	pinMode(20, OUTPUT);
	spiInit(1000000);
	digitalWrite(20, 1);
	delayMicros(1000000);

	struct Pixel frames[FRAMES][LEDS] = {{0}};
	int framenum = 0;

	//	frames per revolution: FRAMES
	// 	revolutions per second: RPM/60
	// 	seconds per revolution: 60/RPM
	// 	seconds per frame: 60/FRAMES/RPM
	// 	microseconds per frame: 1000000 * 60 / FRAMES / RPM

	int delay = 1000000 * 60 / (FRAMES * RPM);
	
	updateSine(frames, 0);
	int i;
	while(1){
		writeFrame(frames,framenum);
		delayMicros(delay);
		framenum = framenum + 1;
		if (framenum == FRAMES) framenum = 0;
	}
}

