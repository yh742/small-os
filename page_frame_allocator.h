#ifndef PAGE_FRAME_ALLOCATOR_H_
#define PAGE_FRAME_ALLOCATOR_H_

#include "kernel.h"
#include "multiboot.h"

unsigned int pfa_init(
                multiboot_info_t const *mbinfo,
                kernel_meminfo_t const *mem, 
                unsigned int fs_paddr,
                unsigned int fs_size);

unsigned int pfa_allocate(unsigned int num_page_frames);

void pfa_free(unsigned int paddr);

void pfa_free_count(unsigned int paddr, unsigned int n);


#endif

