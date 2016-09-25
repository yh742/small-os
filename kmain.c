#include "fb.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "stdio.h"
#include "serial.h"
#include "keyboard.h"
#include "interrupt.h"
#include "multiboot.h"

#define CHECK_FLAG(a,b) (a & (1 << b))

typedef void (*call_module_t)(void);

int sum_of_three(int arg1, int arg2, int arg3)
{
	return arg1 + arg2 + arg3;
}

//ebx is the multiboot struct
int kmain(unsigned int ebx)
{	
	/*
	serial_configure_baud_rate(COM1, 1);
	serial_configure_line(COM1);
	serial_write(COM1, 'f');
	serial_write(COM1, 'f');
	serial_write(COM1, 'f');
	serial_write(COM1, 'f');
	serial_write(COM1, 'f');
	
	
	char buff[] = "x. this is a test string and a really long stirng that is\n";
	int i = 0;
	for (i = 0; i < 30; i ++)
	{
		buff[0] = 0x30 + i;
		fb_write(buff, sizeof(buff) - 1);
	}
	*/
	
	// System Init Here
	fb_init();

	printf("%u. GDT init here\n", 1);
	gdt_init();
	printf("%u. IDT init here\n", 2);
	idt_init();
	printf("%u. PIC init here\n", 3);
	pic_init();
	
	// Interrupt Init Here
	printf("%u. KBD init here\n", 4);
	kbd_init();
	enable_interrupts();

	// Load Modules 
	multiboot_info_t *mbinfo = (multiboot_info_t *)ebx;
	//unsigned int address_of_module = mbinfo->mods_addr;
	//call_module_t start = (call_module_t)address_of_module;	
	//start();

	printf("flags = %X\n", mbinfo->flags);
	
	if (CHECK_FLAG(mbinfo->flags, 3)){

		multiboot_module_t *mod;
		unsigned int i;
	
		printf("module count =  %u, module address = %X\n", 
			mbinfo->mods_count, mbinfo->mods_addr);

		for (i = 0, mod = (multiboot_module_t *) mbinfo->mods_addr;
			i < mbinfo->mods_count; i++, mod++){
			printf("mod_start = %X, mod_end = %X, cmdline = %s\n",
				mod->mod_start, mod->mod_end, mod->cmdline);		
				call_module_t start = (call_module_t)(mod->mod_start + 1);
				start();
		}	
	}	
	return 0;
}
