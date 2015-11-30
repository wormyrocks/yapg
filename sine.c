#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI 3.14159265358

uint8_t leds, revs, speed, totalframes;


struct pixel {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct pixel *getPixel(struct pixel *frames, int i, int j, int k){
	unsigned addr = i + j * leds + k * leds * revs;
	return frames + addr;
}


int main(){
	leds = 72;
	revs = 40;
	speed = 3;
	int sinx;
	int amplitude = leds/2-1;
	totalframes = amplitude * 2;
	FILE *fp;
	struct pixel *frames, *p;
	fp = fopen("sine.led","w");
	fprintf(fp, "%c%c%c%c%c%c%c%c", leds, revs, speed, totalframes, 0x00,0x00,0x00,0x00);
	unsigned long size = leds * revs * totalframes * sizeof(struct pixel);

	frames = malloc(size);
	memset(frames, 0x00, size);

	int amp = amplitude;	

	int dir = -2;
	int i,j;
	
	for (j = 0; j < totalframes; ++j){
		for (i = 0; i < revs; ++i){
			int sinx = (int)(.5+leds/2 + amp * (sin(PI * i * 2.0 / (revs / 2.0))));
			p = getPixel(frames, sinx, i, j);
			p->r = 0xff;
			p->g = 0xff;
			p->b = 0xff;
		}
		if (amp == amplitude){
			dir = -2;
		}else if (amp == -1 * amplitude){
			dir = 2;
		}
		amp = amp + dir;
	}

	fwrite(frames, sizeof(char), size, fp);
	fclose(fp);
}
