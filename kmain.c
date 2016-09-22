#include "gdt.h"
#include "idt.h"
#include "fb.h"
#include "serial.h"

int sum_of_three(int arg1, int arg2, int arg3)
{
	return arg1 + arg2 + arg3;
}

int kmain()
{	
	/*
	serial_configure_baud_rate(COM1, 1);
	serial_configure_line(COM1);
	serial_write(COM1, 'f');
	serial_write(COM1, 'f');
	serial_write(COM1, 'f');
	serial_write(COM1, 'f');
	serial_write(COM1, 'f');
	
	
	char buff[] = "x. this is a test string and a really long stirng that is\n";
	int i = 0;
	for (i = 0; i < 30; i ++)
	{
		buff[0] = 0x30 + i;
		fb_write(buff, sizeof(buff) - 1);
	}
	*/

	gdt_init();
	idt_init();
	return 0;
}
