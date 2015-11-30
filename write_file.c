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

#define VSPACE 1

struct pixel {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

uint8_t pixels;
uint8_t revs;
uint8_t speed;
uint8_t total_frames;

//help with pixel_data structure from eric mueller; adapted from following code:
//   	https://gist.github.com/hmc-cs-emueller/c4ebbe9f3b4064fed16a

#define RGB(r, g, b) ((struct pixel) {r, g, b})

#define RED RGB(0xff, 0, 0)
#define GREEN RGB(0, 0xff, 0)
#define BLUE RGB(0, 0, 0xff)

struct pixel *getPixel(struct pixel *frames, unsigned i, unsigned j, unsigned k){
	unsigned long addr = i + j * pixels + k * pixels * revs;
	return frames + addr;
}

void save(FILE *fp, struct pixel *frames){
	unsigned long size = pixels * revs * total_frames * sizeof(struct pixel);
	rewind(fp);
	fprintf(fp, "%c%c%c%c", pixels, revs, speed, total_frames);
	fprintf(fp, "%c%c%c%c", 0x00, 0x00, 0x00, 0x00);
	fwrite(frames, sizeof(char), size, fp);
}

void refresh_data(struct pixel *frames, unsigned z){
	move(VSPACE,0);
	unsigned i,j,k;
	struct pixel *p;
	unsigned addr;
	for (i = 0; i < pixels; ++i){
		for (j = 0; j < revs; ++j){
			p = getPixel(frames, i, j, z);
 			if (p->r + p->g + p->g > 0) printw("o");
 			else printw(".");
		}
		printw("\n");
	}
}

int main(){
	char fname[64];

	printf("Enter filename: ");
	scanf("%s",fname);
	strcat(fname, ".led");
	FILE *fp;
	
	int input;

	struct pixel *frames;

	if( access( fname, F_OK ) != -1 ) {
		fp = fopen(fname, "r+");
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

		fread(frames, sizeof(char), size, fp);

	} else {
		fp = fopen(fname,"w");
		printf("pixels/strip (0-255) [default: 72] ");
		scanf("%i", &input);
		if (input < 1 || input > 255) input = DEFAULT_PIXELS_NUM;
		pixels = (uint8_t)input;

		printf("pixels/revolution (0-255) [default: 60] ");
		scanf("%i", &input);
		if (input < 1 || input > 255) input = DEFAULT_PX_REV_NUM;
		revs = (uint8_t)input;
		
		printf("number of frames between update (0-255) [default: 1] ");
		scanf("%i", &input);
		if (input < 1 || input > 255) input = DEFAULT_SPEED_NUM;
		speed = (uint8_t)input;

		printf("number of frames in your animation (0-255) [default: 10] ");
		scanf("%i", &input);
		if (input < 1 || input > 255) input = DEFAULT_FRAMES_ANIMATION;
		total_frames = (uint8_t)input;
		
		unsigned long size = pixels * revs * total_frames * sizeof(struct pixel);
		frames = malloc(size);
		if (!frames){
			printf("malloc failed");
			exit(1);
		}
		memset(frames, 0x00, size);
		save(fp, frames);
	}

	initscr();
	keypad(stdscr, TRUE);

	unsigned i;
	unsigned j;
	unsigned y,x;
	unsigned z = 0;
		
	unsigned my = 0, mx = 0;
	char str[10];
	int ch;
	noecho();
	printw("striplen %d pixels/rev %d framerate %d num. frames %d\n", pixels,revs,speed,total_frames);
	struct pixel *q;

	refresh_data(frames, z);
	
	move(pixels + VSPACE, 0);
	printw("\n");
	move(pixels + VSPACE, 0);
	printw("x: %d, y: %d, frame: %d",mx, my, z);
	move(VSPACE,0);
	while((ch = getch()) != 0x71)
	{
		getyx(stdscr, y, x);	
		my = y - VSPACE;
		mx = x;
		switch(ch)
		{
			case 0x6a:
				if (mx <= 0){
					mx = revs - 1;
				}else mx = (mx - 1) % revs;
				break;
			case 0x6c:
				mx = (mx + 1) % revs;
				break;
			case 0x69:
				if (my <= 0){
					my = pixels - 1;
				}else my = (my - 1) % pixels;
				break;
			case 0x6b:
				my =  (my + 1) % pixels;
				break;
			case 0x4a:
				if (mx <= 5){
					mx = revs - 1;
				}else mx = (mx - 5) % revs;
				break;
			case 0x4c:
				mx = (mx + 5) % revs;
				break;
			case 0x49:
				if (my <= 0){
					my = pixels - 5;
				}else my = (my - 5) % pixels;
				break;
			case 0x4b:
				my = (my + 5) % pixels;
				break;
			case 0x75:
				if (z == 0) z = total_frames - 1;
				else --z;
				refresh_data(frames, z);
				break;
			case 0x6f:
				if (z == total_frames - 1) z = 0;
				else ++z;
				refresh_data(frames, z);
				break;
			case 0xa:
				move(pixels+VSPACE+1, 0);
				echo();
				do {
					q = getPixel(frames, my, mx, 0);
					printw("Pixel color: 0x%02x%02x%02x", q->r,q->g,q->b);
					move(pixels + VSPACE + 1, 15);
					getyx(stdscr, y, x);
					refresh();
					ch = getch();
				}
				while (ch != 0xa && ch != 0x71);
				move(pixels + VSPACE + 1,0);
				printw("\n");
				noecho();
				break;

		}
		x = mx;
		y = my + VSPACE;
		move(pixels + VSPACE,0);
		printw("\n");
		move(pixels + VSPACE,0);
		printw("x: %d, y: %d, frame: %d",mx, my, z);
		move (y, x);
		refresh();
	}
	
	fclose(fp);
	endwin();
	return (0);

}