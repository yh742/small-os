#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "interrupt.h"

void kbd_init();

void kbd_handle(struct cpu_info, struct idt_info, struct stack_state);

#endif
