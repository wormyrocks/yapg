#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#define DEFAULT_PIXELS_NUM 72
#define DEFAULT_PX_REV_NUM 60
#define DEFAULT_SPEED_NUM 1
#define DEFAULT_FRAMES_ANIMATION 10

struct pixel {
	unsigned char r;
	unsigned char g;
	unsigned char b;
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
    
    // do some bounds checking here or yolo, whatever
    if (x >= m || y >= n || z >= k) {
        printf("%s: index out of bounds\n", __func__);
        return NULL;
    }
	return &(a->data[x*m*n + y*n + z]);
}


#define RGB(r, g, b) ((struct pixel) {r, g, b})

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
	struct pixel_data frames;



	if( access( fname, F_OK ) != -1 ) {
		printf("File exists. Reading...\n");
		fp = fopen(fname, "r+");
		fscanf(fp, "%c%c%c%c", &pixels, &revs, &speed, &total_frames);
		printf("There are %d pixels in the strip.\nThere are %d horizontal pixels in one revolution.\nThere are %d revolutions between each frame.\nThere are %d frames in one animation.", pixels,revs,speed,total_frames);
		char junk;
		fscanf(fp, "%c%c%c%c", &junk, &junk, &junk, &junk);
		
		
		frames.m = pixels;
		frames.n = revs;
		frames.k = total_frames;

		unsigned long size = frames.m * frames.n * frames.k * sizeof(struct pixel);
		frames.data = malloc(size);
		memset(frames.data, 0x00, size);
		fread(frames.data, sizeof(char), size, fp);


	} else {
		fp = fopen(fname,"w");
		printf("pixels/strip (0-255) [default: 72] ");
		scanf("%i", &input);
		if (input < 1 || input > 255) input = DEFAULT_PIXELS_NUM;
		pixels = (uint8_t)input;
		fprintf(fp, "%c", pixels);

		printf("pixels/revolution (0-255) [default: 60] ");
		scanf("%i", &input);
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
	
		fprintf(fp, "%c%c%c%c", 0x00, 0x00, 0x00, 0x00);
		
		frames.m = pixels;
		frames.n = revs;
		frames.k = total_frames;

		unsigned long size = frames.m * frames.n * frames.k * sizeof(struct pixel);
		frames.data = malloc(size);
		memset(frames.data, 0x00, size);
		
		fwrite(frames.data, sizeof(char), size, fp);
	}

	initscr();
	int i;
	int j;

	struct pixel *p;
	for (i = 0; i < frames.m; ++i){
		for (j = 0; j < frames.n; ++j){
			p = get_pixel(&frames, i, j, 0);
			if (p->r > 0 || p->g > 0 || p->g > 0) printw("o");
			else printw(".");
		}
		printw("\n");
	}
	move(0,0);
	refresh();
	getch();
	fclose(fp);
	endwin();
	return (0);

}