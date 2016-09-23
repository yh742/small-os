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

void fb_write_byte(char* byte)
{
	fb_write(byte, 1);
}

void fb_write_ui(unsigned int val)
{
	unsigned int n, digit;
    	if (val >= 1000000000) {
		n = 1000000000;
        } else {
	       	n = 1;
	       	while (n*10 <= val) {
			n *= 10;
		}
	}
    	while (n > 0) {
            digit = val / n;
	    char c = '0' + digit;
            fb_write_byte(&c);
            val %= n;
            n /= 10;
    	}
}


void fb_write_hex(unsigned int n)
{
	char *chars = "0123456789ABCDEF";
	unsigned char b = 0;
	int i;

	fb_write_s("0x");
	for (i = 7; i >= 0; --i) {
		b = (n >> i*4) & 0x0F;
	        fb_write_byte(&chars[b]);
	}
}



void fb_write_s(char *buf)
{
	char *c;
	unsigned int len = 0;
	for (c = buf; *c < '\0'; c++)
		len++;
	fb_write(buf, len);
	
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
