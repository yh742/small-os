#ifndef KERNEL_H_
#define KERNEL_H_

#define KERNEL_VIRTUAL_ADDRESS	0xC0400000

struct kernel_meminfo {
	unsigned int kernel_physical_start;
	unsigned int kernel_physical_end;
	unsigned int virtual_start;
	unsigned int virtual_end;
} __attribute__((packed));

typedef struct kernel_meminfo kernel_meminfo_t;

#endif
