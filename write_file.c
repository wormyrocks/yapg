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

	for (i = 0; i < pixels; ++i){
		for (j = 0; j < revs; ++j){
			p = getPixel(frames, i, j, z);
 			if (p->r + p->g + p->b > 0) printw("o");
 			else printw(".");
		}
		printw("\n");
	}
}
void inccolor(unsigned char *p){
	if (*p == 0xc0) *p = 0xff;
	else if (*p > 0xc0) *p = 0x00;
	else *p = *p + 0x40;
}

void shift(struct pixel *frames, unsigned z, char dir)
{
	struct pixel *frames_copy;
	struct pixel *a;
	struct pixel *b;

	unsigned long size2 = pixels * revs * sizeof(struct pixel);
	frames_copy = malloc(size2);
	memset(frames_copy, 0x00, size2);

	unsigned i,j;
	unsigned tmp, tmp2;
	unsigned vx, vy;
	switch (dir){
		case 'u':
			vx = 0;
			vy = -1;
			break;
		case 'd':
			vx = 0;
			vy = 1;
			break;
		case 'l':
			vx = -1;
			vy = 0;
			break;
		case 'r':
			vx = 1;
			vy = 0;
			break;
	}
	for (i = 0; i < pixels; ++i){
		for (j = 0; j < revs; ++j){

			if (dir == 'u' && i == 0) tmp = pixels - 1;
			else if (dir == 'd' && i == pixels - 1) tmp = 0;
			else tmp = i + vy;

			if (dir == 'l' && j == 0) tmp2 = revs - 1;
			else if (dir == 'r' && j == revs - 1) tmp2 = 0;
			else tmp2 = j + vx;

			a = getPixel(frames, i, j, z);
			b = getPixel(frames_copy, tmp, tmp2, z);
			b->r = a->r;
			b->g = a->g;
			b->b = a->b;
		}
	}
	for (i = 0; i < pixels; ++i){
		for (j = 0; j < revs; ++j){
			a = getPixel(frames, i, j, z);
			b = getPixel(frames_copy, i, j, z); 
			a->r = b->r;
			a->g = b->g;
			a->b = b->b;						
		}
	}
	free(frames_copy);
}

void copy_next (struct pixel *frames, unsigned z)
{
	unsigned next;
	struct pixel *a;
	struct pixel *b;
	unsigned i,j;
	next = (z + 1) % total_frames;
	for (i = 0; i < pixels; ++i){
		for (j = 0; j < revs; ++j){
			b = getPixel(frames, i, j, z);
			a = getPixel(frames, i, j, next); 
			a->r = b->r;
			a->g = b->g;
			a->b = b->b;						
		}
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
	noecho();
	unsigned y, x, xb;
	printw("i/j/k/l - move cursor [+shift: fast]\n");
	printw("u/o - move between frames\n");
	printw("enter - edit color value\n");
	printw("\nr/g/b - increment color value\n");
	printw("h - increment white value\n");
	printw("x - clear pixel\nc - copy pixel\nv - paste pixel");
	printw("\nw/a/s/d - shift frame up/left/down/right\n");
	printw("z - copy to next frame\n");
	printw("\nshift-s - save\n");
	printw("q - quit\n");
	keypad(stdscr, TRUE);
	getch();
	move(0,0);
	unsigned i;
	unsigned j;
	unsigned z = 0;

	unsigned my = 0, mx = 0;
	char str[6];

	struct pixel *px_copy;
	px_copy->r = 0x00;
	px_copy->g = 0x00;
	px_copy->b = 0x00;

	unsigned ch, m;
	unsigned done;
	done = 0;
	printw("striplen %d pixels/rev %d framerate %d num. frames %d\n", pixels,revs,speed,total_frames);
	struct pixel *q;

	refresh_data(frames, z);
	move(pixels + VSPACE, 0);
	printw("\n");
	move(pixels + VSPACE, 0);
	printw("x: %d, y: %d, frame: %d",mx, my, z);
	move(VSPACE,0);
	while(!done)
	{
		ch = getch();
		getyx(stdscr, y, x);
		move(VSPACE + pixels + 1,0);
		printw("\n\n");
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
			case 'c':
				px_copy = getPixel(frames, my, mx, z);
				move(pixels + VSPACE + 1, 0);
				printw("pixel copied.");
				break;
			case 'v':
				q = getPixel(frames, my, mx, z);
				q->r = px_copy->r;
				q->g = px_copy->g;
				q->b = px_copy->b;
				refresh_data(frames, z);
				move(pixels + VSPACE + 1, 0);
				printw("pixel pasted.");

				break;
			case 0xa: //enter
				xb = mx;
				move(pixels+VSPACE+1, 0);
				q = getPixel(frames, my, mx, z);
				unsigned rgb;

				printw("Pixel color: 0x%02x%02x%02x\nNew value:   0x", q->r,q->g,q->b);
				move(pixels + VSPACE + 2, 15);
				echo();
				getnstr(str, 6);
				unsigned numchars;
				if (sscanf(str, "%06x%n", &rgb, &numchars) == 1 && numchars == 6){
					q->r = (rgb & 0xff0000) >> 0x10;
					q->g = (rgb & 0x00ff00) >> 0x8;
					q->b = (rgb & 0x0000ff);
					refresh_data(frames,z);
				}
				move(pixels + VSPACE + 1,0);
				printw("\n\n");
				noecho();
				mx = xb;
				break;
			case 'S':
				move(pixels + VSPACE + 1,0);
				printw("Saved.");
				save(fp, frames);
				break;
			case 'q':
				move(pixels + VSPACE + 1,0);
				printw("Save (s) Quit (q)");
				m = getch();
				switch(m){
					case 'S':
					case 0x73:
						save(fp, frames);
						done = 1;
						break;
					case 0x71:
						done = 1;
						break;
					default:
						move(pixels + VSPACE + 1,0);
						printw("\n");
				}
				break;
			case 'r':
				q = getPixel(frames, my, mx, z);
				inccolor(&(q->r));
				refresh_data(frames, z);
				move(pixels + VSPACE + 1,0);
				printw("Pixel color: 0x%02x%02x%02x", q->r,q->g,q->b);
				break;
			case 'g':
				q = getPixel(frames, my, mx, z);
				inccolor(&(q->g));
				refresh_data(frames, z);
				move(pixels + VSPACE + 1,0);
				printw("Pixel color: 0x%02x%02x%02x", q->r,q->g,q->b);
				break;
			case 'b':
				q = getPixel(frames, my, mx, z);
				inccolor(&(q->b));
				refresh_data(frames, z);
				move(pixels + VSPACE + 1,0);
				printw("Pixel color: 0x%02x%02x%02x", q->r,q->g,q->b);
				break;
			case 'h':
				q = getPixel(frames, my, mx, z);
				inccolor(&(q->b));
				inccolor(&(q->g));
				inccolor(&(q->r));
				refresh_data(frames, z);
				move(pixels + VSPACE + 1,0);
				printw("Pixel color: 0x%02x%02x%02x", q->r,q->g,q->b);
				break;
			case 'x':
				q = getPixel(frames, my, mx, z);
				q->r = 0x00;
				q->g = 0x00;
				q->b = 0x00;
				refresh_data(frames, z);
				move(pixels + VSPACE + 1,0);
				printw("Pixel color: 0x%02x%02x%02x", q->r,q->g,q->b);
				break;
			case 'w':
				shift(frames, z, 'u');
				refresh_data(frames,z);
				break;
			case 'a':
				shift(frames, z, 'l');
				refresh_data(frames,z);
				break;
			case 's':
				shift(frames, z, 'd');
				refresh_data(frames,z);
				break;
			case 'd':
				shift(frames, z, 'r');
				refresh_data(frames,z);
				break;
			case 'z':
				copy_next(frames,z);
				move(pixels + VSPACE + 1,0);
				printw("Copied to next frame.");
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
