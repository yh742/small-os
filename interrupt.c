#include "interrupt.h"
#include "idt.h"
#include "stddef.h"
#include "stdio.h"
#include "pic.h"

interrupt_handler_t interrupt_handlers[IDT_NUM_ENTRIES];

unsigned int register_interrupt_handler(unsigned int interrupt, interrupt_handler_t handler)
{
	if (interrupt > 255){
		return 1;
	}

	if (interrupt_handlers[interrupt] != NULL){
		return 1;
	}

	printf("Registering interrupt #%u\n", interrupt);
	interrupt_handlers[interrupt] = handler;
	return 0;
}

void interrupt_handler(struct cpu_info state, struct idt_info info, struct stack_state exec)
{
	if (interrupt_handlers[info.idt_index] != NULL)
	{
		interrupt_handlers[info.idt_index](state, info, exec);
	}
	else
	{
		printf("Interrupt handler for IRQ#%u does not exist!\n", info.idt_index);
	}
}
