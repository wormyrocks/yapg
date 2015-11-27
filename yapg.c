// evan kahn
// e155, final project

#include "easyPIO.h"
#include <math.h>
#include <string.h>
#define PI 3.14159265

#define LEDOUT 0
#define DEBUG 1

#define LEDS 72
#define COLORS 3
#define RPM 500
#define FRAMES 40
#define RGB(r, g, b) ((struct pixel) {r, g, b})

#define RED RGB(0xff, 0, 0)
#define GREEN RGB(0, 0xff, 0)
#define BLUE RGB(0, 0, 0xff)

#define WHITE RGB(0xff,0xff,0xff)
#define WHITE_64 RGB(0x80,0x80,0x80)

struct pixel {
	unsigned char r, g, b;
};

//help with pixel_data structure from eric mueller; adapted from following code:
//   	https://gist.github.com/hmc-cs-emueller/c4ebbe9f3b4064fed16a

struct pixel_data {
	//m: pixels per colum
	//n: columns per image
	//k: frames per animation
    unsigned m, n, k;
    struct pixel *data;
};

struct pixel *get_pixel(struct pixel_data *a, unsigned x, unsigned y, unsigned z)
{
    unsigned m = a->m;
    unsigned n = a->n;
    unsigned k = a->k;
    struct pixel *data = a->data;
    
    // do some bounds checking here or yolo, whatever
    if (x >= m || y >= n || z >= k) {
        printf("%s: index out of bounds\n", __func__);
        return NULL;
    }

    return &a->data[x*m*n + y*n + z];
}

void setLED (struct pixel p){
	if (LEDOUT){
		spiSendReceive(0xff);
		spiSendReceive(p.b);
		spiSendReceive(p.g);
		spiSendReceive(p.r);
	}
}

void initFrame (){
	int i;
	if (LEDOUT) for (i=0;i<4;i++) spiSendReceive(0x00);
}

void endFrame (){
	struct pixel p = {0xff,0xff,0xff};
	setLED(p);
	setLED(p);
}


void displayFrame(struct pixel frames[FRAMES][LEDS], int framenum){
	int i;
	initFrame();
	if (DEBUG == 1) printf("begin frame %d\n", framenum);
	for (i=0;i<LEDS;i++){	
		struct pixel p = frames[framenum][i]; 
		if (DEBUG){
			printf("%02d 0x%02x%02x%02x ",i,p.b,p.g,p.r);
		}
		setLED(p);
		if (DEBUG && ((i+1) % 8 == 0)) printf("\n");
	}
	endFrame();

}

void clearFrame(struct pixel frames[FRAMES][LEDS]){
	memset(*frames, 0, sizeof(*frames));
}

void updateSine(struct pixel frames[FRAMES][LEDS], int amplitude){
	int i;
	clearFrame(frames);
	for (i = 0; i < FRAMES; ++i){
		int sinx = (int)(amplitude * (1 + sin(PI * i * 2 / (FRAMES/2.0))));
		frames[i][sinx] = WHITE;
	}
}

int main() {

	if (LEDOUT){
		pioInit();
		pinMode(20, OUTPUT);
		spiInit(1000000);
		digitalWrite(20, 1);
		delayMicros(1000000);
	}
	
	struct pixel frames[FRAMES][LEDS] = {{0}};
	int framenum = 0;

	//	frames per revolution: FRAMES
	// 	revolutions per second: RPM/60
	// 	seconds per revolution: 60/RPM
	// 	seconds per frame: 60/FRAMES/RPM
	// 	microseconds per frame: 1000000 * 60 / FRAMES / RPM

	int delay = 1000000 * 60 / (FRAMES * RPM);
	int amplitude = LEDS/2-1;
	int dir = -1;
	int revolutions = 0;
	
	updateSine(frames, amplitude);
	
	while(revolutions < 5){

		displayFrame(frames,framenum);
		
		if (LEDOUT) delayMicros(delay);
		
		framenum = ((framenum + 1) % FRAMES);
		
		if (framenum == FRAMES - 1){
			++revolutions;
		}
		
		if (framenum == 0){
			if (amplitude == LEDS/2-1){
				dir = -1;
			}
			else if (amplitude == 0){
				dir = 1;
			}
			amplitude += dir;
			updateSine(frames,amplitude);
		}
	}
	
	
	return(0);
}

