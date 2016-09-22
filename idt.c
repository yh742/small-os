#include "idt.h"
#include "idt_asm.h"
#include "gdt.h"

// Interrupt gates don't disable all other interrupts
#define IDT_INTERRUPT_GATE	0x0
#define IDT_TRAP_GATE		0x1

#define CREATE_IDT_GATE(num) \
	create_idt_gate(num, (unsigned int) &interrupt_handler_##num, IDT_TRAP_GATE, PL0);

#define DECLARE_INT_HANDLER(num) \
	void interrupt_handler_##num(void);

struct idt_ptr {
	unsigned short size;
	unsigned int address;
};

struct idt_gate {
	unsigned short offset_low;
	unsigned short segment_sel;
	unsigned char zero;	// reserved
	unsigned char config;
	unsigned short offset_high;
};

struct idt_gate idt[IDT_NUM_ENTRIES];

static void create_idt_gate(unsigned short n, unsigned int handler, 
		unsigned short type, unsigned short pl);

// Link to fucntions in interrupt_asm.s
DECLARE_INT_HANDLER(0);
DECLARE_INT_HANDLER(1);
DECLARE_INT_HANDLER(2);
DECLARE_INT_HANDLER(3);
DECLARE_INT_HANDLER(4);
DECLARE_INT_HANDLER(5);
DECLARE_INT_HANDLER(6);
DECLARE_INT_HANDLER(7);
DECLARE_INT_HANDLER(8);
DECLARE_INT_HANDLER(9);
DECLARE_INT_HANDLER(10);
DECLARE_INT_HANDLER(11);
DECLARE_INT_HANDLER(12);
DECLARE_INT_HANDLER(13);
DECLARE_INT_HANDLER(14);
DECLARE_INT_HANDLER(15);
DECLARE_INT_HANDLER(16);
DECLARE_INT_HANDLER(17);
DECLARE_INT_HANDLER(18);
DECLARE_INT_HANDLER(19);

// User Interrupt 
DECLARE_INT_HANDLER(32);
DECLARE_INT_HANDLER(33);
DECLARE_INT_HANDLER(34);
DECLARE_INT_HANDLER(35);
DECLARE_INT_HANDLER(36);
DECLARE_INT_HANDLER(37);
DECLARE_INT_HANDLER(38);
DECLARE_INT_HANDLER(39);
DECLARE_INT_HANDLER(40);
DECLARE_INT_HANDLER(41);
DECLARE_INT_HANDLER(42);
DECLARE_INT_HANDLER(43);
DECLARE_INT_HANDLER(44);
DECLARE_INT_HANDLER(45);
DECLARE_INT_HANDLER(46);
DECLARE_INT_HANDLER(47);



void idt_init()
{
	struct idt_ptr ptr; 
	ptr.size = sizeof(struct idt_gate) * IDT_NUM_ENTRIES;
	ptr.address = (unsigned int)&idt;

	// Hardware Generated Exceptions
	CREATE_IDT_GATE(0);
	CREATE_IDT_GATE(1);
	CREATE_IDT_GATE(2);
	CREATE_IDT_GATE(3);
	CREATE_IDT_GATE(4);
	CREATE_IDT_GATE(5);
	CREATE_IDT_GATE(6);
	CREATE_IDT_GATE(7);
	CREATE_IDT_GATE(8);
	CREATE_IDT_GATE(9);
	CREATE_IDT_GATE(10);
	CREATE_IDT_GATE(11);
	CREATE_IDT_GATE(12);
	CREATE_IDT_GATE(13);
	CREATE_IDT_GATE(14);
	CREATE_IDT_GATE(15);
	CREATE_IDT_GATE(16);
	CREATE_IDT_GATE(17);
	CREATE_IDT_GATE(18);
	CREATE_IDT_GATE(19);

	// User defined interrupt gates
	CREATE_IDT_GATE(32);
	CREATE_IDT_GATE(33);
	CREATE_IDT_GATE(34);
	CREATE_IDT_GATE(35);
	CREATE_IDT_GATE(36);
	CREATE_IDT_GATE(37);
	CREATE_IDT_GATE(38);
	CREATE_IDT_GATE(39);
	CREATE_IDT_GATE(40);
	CREATE_IDT_GATE(41);
	CREATE_IDT_GATE(42);
	CREATE_IDT_GATE(43);
	CREATE_IDT_GATE(44);
	CREATE_IDT_GATE(45);
	CREATE_IDT_GATE(46);
	CREATE_IDT_GATE(47);

	idt_load_and_set((unsigned int)&ptr);
}

void create_idt_gate(unsigned short n, unsigned int handler,
		unsigned short type, unsigned short pl)
{
	idt[n].offset_low 	=	handler & (0xFFFF);
	idt[n].offset_high 	= 	(handler >> 16) & (0xFFFF);
	// Reserved
	idt[n].zero 		=	0;
	// P -> Set to 1 for enabled interrupts
	// DPL -> Set to Ring0 or Ring3
	// S -> Set to 0 for interrupts gates
	// Type -> Gate Types 
	idt[n].config		=	 (0x1 << 7) | 
					 ((pl << 0x03) << 5) | 
					 (0x0 << 4) | 
					 (0x1 << 3) | 
					 (0x1 << 2) | 
					 (0x1 << 1) | 
					 (type & 0x01);
}

