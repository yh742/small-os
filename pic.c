#include "io.h"
#include "pic.h"

#define PIC1_COMMAND 	0x20
#define PIC1_DATA	0x21

#define PIC2_COMMAND	0xA0
#define PIC2_DATA	0xA1

#define PIC1_ICW1	0x11
#define PIC2_ICW1	0x11

#define PIC1_ICW2	0x20
#define PIC2_ICW2	0x28

#define PIC1_ICW3	0x04
#define PIC2_ICW3	0x02

#define PIC1_ICW4	0x01
#define PIC2_ICW4	0x01

#define PIC_EOI		0x20

void pic_init()
{

	outb(PIC1_COMMAND, PIC1_ICW1);
	outb(PIC2_COMMAND, PIC2_ICW1);

	outb(PIC1_DATA, PIC1_ICW2);
	outb(PIC2_DATA, PIC2_ICW2);

	outb(PIC1_DATA, PIC1_ICW3);
	outb(PIC2_DATA, PIC2_ICW3);

	outb(PIC1_DATA, PIC1_ICW4);
	outb(PIC2_DATA, PIC2_ICW4);

	pic_mask(0xFD, 0xFF);
}

void pic_acknowledge()
{
	outb(PIC1_COMMAND, PIC_EOI);
	outb(PIC2_COMMAND, PIC_EOI);
}

void pic_mask(unsigned char mask1, unsigned char mask2)
{
	outb(PIC1_DATA, mask1);
	outb(PIC2_DATA, mask2);
}
