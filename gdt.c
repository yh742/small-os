#include "gdt.h"
#include "gdt_asm.h"

#define SEG_BASE	0x00000000
#define SEG_LIM		0xFFFFF
#define GDT_SIZE	3

#define RX_TYPE 	0xA
#define RW_TYPE		0x2

struct gdt {
	unsigned short size;
	unsigned int address;
} __attribute__((packed));

struct gdt_entry {
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_mid;
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
} __attribute__((packed));

struct gdt_entry gdt_entries[GDT_SIZE];

static void create_gdt_entry(unsigned int n, unsigned char pl, unsigned int type);

void gdt_init()
{
	struct gdt gdt_ptr;
	gdt_ptr.size = sizeof(struct gdt_entry) * GDT_SIZE;
	gdt_ptr.address = (unsigned int)&gdt_entries;
	// Null entry
	create_gdt_entry(0, PL0, 0);
	// Kernel Code Segment
	create_gdt_entry(1, PL0, RX_TYPE);
	// Kernel Data Segment
	create_gdt_entry(2, PL0, RW_TYPE);

	// Load Table Pointer Struct
	gdt_load_and_set((unsigned int)&gdt_ptr);
}

static void create_gdt_entry(unsigned int n, unsigned char pl, unsigned int type)
{
	// Setup base address should start at zero
	gdt_entries[n].base_low 	= 	(SEG_BASE & 0xFFFF);
	gdt_entries[n].base_mid		= 	(SEG_BASE >> 16) & 0xFF; 
	gdt_entries[n].base_high	= 	(SEG_BASE >> 24) & 0XFF;

	// Bit 7 gr -> 1 for page access (4kb)
	// Bit 6 sz ->  for 32 bit protected mode
	// Setup limit address should be the whole address space
	gdt_entries[n].limit_low	= 	(SEG_LIM & 0xFFFF);
	gdt_entries[n].granularity 	|= 	((SEG_LIM >> 16) & 0x0F) | (0x01 << 7) | (0x01 << 6);


	// Bit 7 - Present
	// Bit 6/5 - Ring0 or Ring3
	// Bit 4 - Always 1
	// Bit 3 - Excutable Bit (for code)
	// Bit 2 - Direction bit (0 grow upwards)
	// Bit 1 - RW? (1 for R/W)
	// Bit 0 - Present?
	gdt_entries[n].access 		=	(0x01 << 7) | ((pl & 0x03) << 5) | (0x01 << 4) | (type & 0x0F);
}

