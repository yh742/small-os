#include "serial.h"
#include "io.h"

#define SERIAL_DATA_PORT(base) 		(base)
#define SERIAL_FIFO_COMMAND_PORT(base)	(base + 2)
#define SERIAL_LINE_COMMAND_PORT(base) 	(base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base) 	(base + 5)
#define DLAB_LOW_BYTE_PORT(base)	(base)
#define DLAB_HI_BYTE_PORT(base)		(base + 1)

#define SERIAL_LINE_ENABLE_DLAB		0x80

void serial_configure_baud_rate(unsigned short com, unsigned short divisor)
{
	outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);	
	outb(DLAB_HI_BYTE_PORT(com), (divisor >> 8) & 0x00FF);
	outb(DLAB_LOW_BYTE_PORT(com), (divisor) & 0x00FF);
}

void serial_configure_line(unsigned short com)
{
	outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
	outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
	outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}

int serial_is_transmit_fifo_empty(unsigned short com)
{
	return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

void serial_write(unsigned short com, char a)
{
	while (serial_is_transmit_fifo_empty(com) == 0x0);
	outb(SERIAL_DATA_PORT(com), a);
}



