#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#define FOUR_KB			0x1000
#define ONE_MB			0x100000
#define FOUR_MB			0x400000
#define EIGHT_MB		0x800000

#define KERNEL_START_VADDR	0xC0000000	
#define KERNEL_PDT_INDX		(KERNEL_START_VADDR >> 22)
#define KERNEL_STACK_SIZE	FOUR_KB

#endif