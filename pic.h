#ifndef _PIC_H_
#define _PIC_H_

#define PIC1_START 	0x20
#define PIC2_START	0x28

#define PIT_INT_IDX 	PIC1_START
#define KBD_INT_IDX	PIC1_START + 1

#define COM1_INT_IDX	PIC1_START + 4
#define COM2_INT_IDX	PIC2_START + 3

void pic_init(void);

void pic_acknowledge(void);

void pic_mask(unsigned char mask1, unsigned char mask2);

#endif
