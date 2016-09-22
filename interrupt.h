#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

void enable_interrupts();

void disable_interrupts();

struct stack_state {
	unsigned int eip;
	unsigned int cs;
	unsigned int eflags;
//	unsigned int user_esp;
//	unsigned int user_ss;
} __attribute__((packed));

struct idt_info {
	unsigned int idt_index;
	unsigned int error_code;
} __attribute__((packed));

struct cpu_info {
	unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int edx;
	unsigned int ecx;
	unsigned int ebx;
	unsigned int eax;
	unsigned int esp;
} __attribute__((packed));

typedef void (*interrupt_handler_t)(struct cpu_info, struct idt_info, struct stack_state);

unsigned int register_interrupt_handler(unsigned int interrupt, interrupt_handler_t handler);

#endif
