#ifndef FB_H_
#define FB_H_

#define FB_BLACK 	0
#define FB_BLUE 	1
#define FB_GREEN 	2
#define FB_CYAN		3
#define FB_RED		4
#define FB_MAGNETA	5
#define FB_BROWN	6
#define FB_GREY		7
#define FB_DGREY	8
#define FB_LBLUE	9
#define FB_LGREEN	10
#define FB_LCYAN	11
#define FB_LRED		12
#define FB_LMAGNETA	13
#define FB_LBROWN	14
#define FB_WHITE	15


void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);

void fb_move_cursor(unsigned short pos);

int fb_write(char *buf, unsigned int len);

#endif
