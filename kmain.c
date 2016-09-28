#include "fb.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "stdio.h"
#include "serial.h"
#include "keyboard.h"
#include "interrupt.h"
#include "multiboot.h"
#include "kernel.h"
#include "common.h"
#include "paging.h"

#define CHECK_FLAG(a,b) (a & (1 << b))

#define KINIT_ERROR_LOAD_FS 		1
#define KINIT_ERROR_INIT_FS 		2
#define KINIT_ERROR_INIT_PAGING 	3
#define KINIT_ERROR_INIT_PFA 		4
#define KINIT_ERROR_MALLOC_ROOT_VFS 	5
#define KINIT_ERROR_INIT_VFS 		6
#define KINIT_ERROR_INIT_SCHEDULER 	7

typedef void (*call_module_t)(void);

// Bring multiboot info struct into the higher half
static multiboot_info_t *remap_multiboot_info(unsigned int mbaddr)
{
	multiboot_info_t *mbinfo = (multiboot_info_t *) PHYSICAL_TO_VIRTUAL(mbaddr);
	mbinfo->mods_addr = PHYSICAL_TO_VIRTUAL(mbinfo->mods_addr);
	mbinfo->mmap_addr = PHYSICAL_TO_VIRTUAL(mbinfo->mmap_addr);
	return mbinfo;
}

static unsigned int kinit(kernel_meminfo_t *mem, 
			const multiboot_info_t *mbinfo,
			unsigned int kernel_pdt_vaddr,
			unsigned int kernel_pt_vaddr)
{
	UNUSED(mem);
	UNUSED(mbinfo);
	disable_interrupts();

	gdt_init();
	idt_init();
	pic_init();
	kbd_init();
	serial_init(COM1,1);

	int res = paging_init(kernel_pdt_vaddr, kernel_pt_vaddr);
	if (res != 0) {
		return KINIT_ERROR_INIT_PAGING;
	}
	
	res = pfa_init(mbinfo, mem);// fs_paddr, fs_size);
	if (res != 0) {
		return KINIT_ERROR_INIT_PFA;
	}

	enable_interrupts();
	return 0;
}

int kmain(unsigned int mbaddr, unsigned int magic_number, 
	kernel_meminfo_t mem, unsigned int kernel_pdt_vaddr, 
	unsigned int kernel_pt_vaddr)
{	
	unsigned int res;
	// Map multiboot struct to higher half
	multiboot_info_t *mbinfo = remap_multiboot_info(mbaddr);
	// Adjust framebuffer to correct position	
	fb_init();

	if (magic_number != MULTIBOOT_BOOTLOADER_MAGIC) {
		printf("ERROR: magic number is wrong!\n");	
		printf("magic_number: %u\n", magic_number);
		return 0xDEADDEAD;
	
	}

	res = kinit(&mem, mbinfo, kernel_pdt_vaddr, kernel_pt_vaddr);
	if (res != 0){
		switch (res) {
		    case KINIT_ERROR_LOAD_FS:
		        printf("ERROR: Could not load filesystem!\n");
		        break;
		    case KINIT_ERROR_INIT_FS:
		        printf("ERROR: Could not initialize filesystem!\n");
		        break;
		    case KINIT_ERROR_INIT_PAGING:
		        printf("ERROR: Could not initialize paging!\n");
		        break;
		    case KINIT_ERROR_INIT_PFA:
		        printf("ERROR: Could not initialize page frame allocator!\n");
		        break;
		    case KINIT_ERROR_MALLOC_ROOT_VFS:
		        printf("ERROR: Could not allocate VFS node for root FS!\n");
		        break;
		    case KINIT_ERROR_INIT_VFS:
		        printf("ERROR: Could not initialize virtual filesystem!\n");
		        break;
		    case KINIT_ERROR_INIT_SCHEDULER:
		        printf("ERROR: Could not initialize scheduler!\n");
		        break;
		    default:
		        printf("ERROR: Unknown error\n");
		        break;
		}

		return 0xDEADDEAD;
	}
	
	/*
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
	}*/	
	
	printf("kmain: kernel initialized successfully!\n");
	//start_init();

	return 0xDEADBEEF;
}
