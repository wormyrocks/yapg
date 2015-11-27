#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>


#define DEFAULT_PIXELS_NUM 72
#define DEFAULT_PX_REV_NUM 60
#define DEFAULT_SPEED_NUM 1
#define DEFAULT_FRAMES_ANIMATION 10

struct Pixel {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

#define RGB(r, g, b) ((struct Pixel) {r, g, b})

#define RED RGB(0xff, 0, 0)
#define GREEN RGB(0, 0xff, 0)
#define BLUE RGB(0, 0, 0xff)

int main(){
	char fname[64];

	printf("Enter filename: ");
	scanf("%s",fname);
	strcat(fname, ".led");
	FILE *fp;
	
	int input;
	uint8_t pixels;
	uint8_t revs;
	uint8_t speed;
	uint8_t total_frames;
	
	if( access( fname, F_OK ) != -1 ) {
		printf("File exists. Reading...\n");
		fp = fopen(fname, "r+");
		fscanf(fp, "%c%c%c%c", &pixels, &revs, &speed, &total_frames);
		printf("There are %d pixels in the strip.\nThere are %d horizontal pixels in one revolution.\nThere are %d revolutions between each frame.\nThere are %d frames in one animation.", pixels,revs,speed,total_frames);
		char junk;
		fscanf(fp, "%c%c%c%c", &junk, &junk, &junk, &junk);
		struct Pixel frames[4][4][4];
		fread(frames, sizeof(char), sizeof(frames), fp);
	} else {
		fp = fopen(fname,"w");
		printf("pixels/strip (0-255) [default: 72] ");
		scanf("%i", &input);
		if (input < 1 || input > 255) input = DEFAULT_PIXELS_NUM;
		pixels = (uint8_t)input;
		fprintf(fp, "%c", pixels);

		printf("pixels/revolution (0-255) [default: 60] ");
		scanf("%i", &input);
		pixels = (uint8_t)input;
		if (input < 1 || input > 255) input = DEFAULT_PX_REV_NUM;
		revs = (uint8_t)input;
		fprintf(fp, "%c", revs);
		
		printf("number of frames between update (0-255) [default: 1] ");
		scanf("%i", &input);
		if (input < 1 || input > 255) input = DEFAULT_SPEED_NUM;
		speed = (uint8_t)input;
		fprintf(fp, "%c", speed);
		
		printf("number of frames in your animation (0-255) [default: 10] ");
		scanf("%i", &input);
		if (input < 1 || input > 255) input = DEFAULT_FRAMES_ANIMATION;
		total_frames = (uint8_t)input;
		fprintf(fp, "%c", total_frames);
		
		fprintf(fp, "%c%c%c%c", 0,0,0,0);
		
		struct Pixel frames;
		malloc((sizeof(struct Pixel))*total_frames*revs*pixels);

		frames[0][0][0] = RED;
		fwrite(frames, sizeof(char), sizeof(frames), fp);
	}

	fclose(fp);
	return (0);

}