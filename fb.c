#include "io.h"
#include "fb.h"

#define FB_MEMORY 0x000B8000

#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT	0x3D5
#define FB_HIGH_BYTE	14
#define FB_LOW_BYTE	15
#define FB_MAX 		80*25

static char *fb = (char *)FB_MEMORY;
static int curr_pos = 0;

void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
	fb[i] = c;
	fb[i + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

void fb_move_cursor(unsigned short pos)
{
	outb(FB_COMMAND_PORT, FB_HIGH_BYTE);
	outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
	outb(FB_COMMAND_PORT, FB_LOW_BYTE);
	outb(FB_DATA_PORT, (pos & 0x00FF));
}

static void scroll()
{
	unsigned int x, y;
	x = 0;
	y = 160;
	while (y <= FB_MAX*2)
	{
		fb[x] = fb[y];
		fb[x+1] = fb[y+1];
		x+=2;
		y+=2;
	}
}

int fb_write(char *buf, unsigned int len)
{
	unsigned int i = 0;	
	for (i = 0; i < len; i++)
	{
		if (curr_pos > FB_MAX)
		{
			scroll();
			curr_pos = curr_pos - 80;
		}
		if (buf[i] == '\n')
		{
			curr_pos = curr_pos + 80;
			curr_pos = curr_pos - (curr_pos % 80);
		}
		else
		{
			fb_write_cell(curr_pos * 2, buf[i], FB_BLACK, FB_GREY);	
			curr_pos++;
		}
		fb_move_cursor(curr_pos);
	}	
	return i;
}
