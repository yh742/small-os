#include "interrupt.h"
#include "idt.h"
#include "fb.h"


interrupt_handler_t interrupt_handlers[IDT_NUM_ENTRIES];

unsigned int register_interrupt_handler(unsigned int interrupt, interrupt_handler_t handler)
{
	if (interrupt > 255)
		return 1;
	if (interrupt_handlers[interrupt] != (void *)0)
		return 1;
	interrupt_handlers[interrupt] = handler;
	return 0;
}

void interrupt_handler(struct cpu_info state, struct idt_info info, struct stack_state exec)
{
	if (interrupt_handlers[info.idt_index] != (void *)0)
	{
		interrupt_handlers[info.idt_index](state, info, exec);
	}
	else
	{
		char buff[] = "ISR does not exist!";
		fb_write(buff, sizeof(buff) - 1);
	}
}
