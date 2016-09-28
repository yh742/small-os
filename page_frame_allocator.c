#include "common.h"
#include "constants.h"
#include "mem.h"
#include "kernel.h"
#include "paging.h"
#include "multiboot.h"
#include "string.h"
#include "stdio.h"
#include "math.h"

#define MAX_NUM_MEMORY_MAP	100

struct memory_map {
	unsigned int addr;
	unsigned int len;
};
typedef struct memory_map memory_map_t;

struct page_frame_bitmap {
	unsigned int *start;
	unsigned int len;
};
typedef struct page_frame_bitmap page_frame_bitmap_t;

static page_frame_bitmap_t page_frames;

static memory_map_t mmap[MAX_NUM_MEMORY_MAP];

static unsigned int mmap_len;

static unsigned int fill_memory_map(multiboot_info_t const *mbinfo,
				kernel_meminfo_t const *mem)//,
				//unsigned int fs_paddr,
				//unsigned int fs_size)
{
	unsigned int addr, len, i = 0;
	if ((mbinfo->flags & 0x00000020) == 0) {
		printf("fill_memory_map: no memory map from grub\n");
		return 0;
	}

	multiboot_memory_map_t *entry = 
		(multiboot_memory_map_t *) mbinfo->mmap_addr;
		
	while ((unsigned int) entry < mbinfo->mmap_addr + mbinfo->mmap_length) {
		if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
			addr = (unsigned int) entry->addr;
			len = (unsigned int) entry->len;
			if (addr <= mem->kernel_physical_start &&
				(addr + len) > mem->kernel_physical_end) {

				addr = mem->kernel_physical_end;
				len = len - mem->kernel_physical_end;
			}
			
			if (addr > ONE_MB) {	
				/*
				if (addr < fs_addr && ((addr + len) > (fs_paddr + fs_size))) {
					mmap[i].addr = addr;
					mmap[i].len = fs_addr - addr;
					++i;
					
					addr = fs_paddr + fs_size;
					len -= (fs_paddr + fs_size) - addr;
				}
				*/
			
				mmap[i].addr = addr;
				mmap[i].len = len;
				++i;
			}
		}
		
		// Goto the next entry
		entry = (multiboot_memory_map_t *)
			(((unsigned int) entry) + entry->size + sizeof(entry->size));
	}

	return i;
}

static unsigned int construct_bitmap(memory_map_t *mmap, unsigned int n)
{
	unsigned int i, bitmap_pfs, bitmap_size, paddr, vaddr, mapped_mem;	
	unsigned int total_pfs = 0;

	for (i = 0; i < n; i++) {
		total_pfs += mmap[i].len / FOUR_KB;
	}
	
	// Divide pages by eight since bitmap'd and figure out how many frame we need
	bitmap_pfs = div_ceil(div_ceil(total_pfs, 8), FOUR_KB);	
	
	// Allocate the physical address for the bitmap 
	for (i = 0; i < n; ++i) {
		if (mmap[i].len >= bitmap_pfs * FOUR_KB) {
			paddr = mmap[i].addr;
			mmap[i].addr += bitmap_pfs * FOUR_KB;
			mmap[i].len -= bitmap_pfs * FOUR_KB;
			break;
		}
	}
	
	// This the amount of frames to track (minus the bitmap frames)
	page_frames.len = total_pfs - bitmap_pfs;
	bitmap_size = div_ceil(page_frames.len, 8);

	if (i == n) {
		printf("construct_bitmap: couldn't find place for bitmapin kernel.\
			paddr: %X, bitmap_size: %u, bitmap_pfs: %u\n",
			paddr, bitmap_size);
		return 1;
	}


	vaddr = pdt_kernel_find_next_vaddr(bitmap_size);
	if (vaddr == 0) {
		printf("construct_bitmap: Could not find virtual address for bitmap in kernel. paddr: %X, bitmap_size: %u, bitmap_pfs: %u\n", paddr, bitmap_size);
		return 1;

	}
	printf("construct_bitmap: bitmap vaddr: %X, bitmap paddr: %X, page_frames.len: %u, bitmap_size: %u, bitmap_pfs: %u\n",
	      vaddr, paddr, page_frames.len, bitmap_size, bitmap_pfs);

	// Map in kernel space
	mapped_mem = pdt_map_kernel_memory(paddr, vaddr, bitmap_size,
		                       PAGING_PL0, PAGING_READ_WRITE);
	if (mapped_mem < bitmap_size) {
		printf("construct_bitmap: Could not map kernel memory for bitmap. paddr: %X, vaddr: %X, bitmap_size: %u\n",
		  paddr, vaddr, bitmap_size);
		return 1;
	}

	page_frames.start = (unsigned int *) vaddr;

	// Set all the bits to 1
	memset(page_frames.start, 0xFF, bitmap_size);
	// Set the last byte appropriately to 1's	
	unsigned int *last = (unsigned int *)((unsigned int)page_frames.start + bitmap_size - 1);
	*last = 0;
	for (i = 0; i < page_frames.len % 8; ++i) {
	*last |= 0x01 << (7 - i);
	}

	return 0;

}

unsigned int pfa_init(multiboot_info_t const *mbinfo,
		kernel_meminfo_t const *mem)//,
		//unsigned int fs_paddr,
		//unsigned int fs_size)
{
	unsigned int i, n, addr, len;	
	n = fill_memory_map(mbinfo, mem);//, fs_paddr, fs_size);
	if (n == 0) {
		return 1;
	}
	
	printf("pfa_init: kernel_physical_start: %X\n,\
			kernel_physical_end: %X\n,\
			kernel_virtual_start: %X\n,\
			kernel_virtual_end: %X\n",\
			mem->kernel_physical_start, mem->kernel_physical_end,
			mem->virtual_start, mem->virtual_end);

	mmap_len = n;

	for (i = 0; i < n; i++) {
		addr = align_up(mmap[i].addr, FOUR_KB);
		len = align_down(mmap[i].len - (addr - mmap[i].addr), FOUR_KB);

		mmap[i].addr = addr;
		mmap[i].len = len;
		
		printf("pfa_init: mmap[%u] -> addr: %X, len: %u, pfs: %u\n", 
			i, addr, len, len / FOUR_KB);
	}
	
	return construct_bitmap(mmap, n);		
}


static void toggle_bit(unsigned int bit_idx)
{
    unsigned int *bits = page_frames.start;
    bits[bit_idx/32] ^= (0x01 << (31 - (bit_idx % 32)));
}

static void toggle_bits(unsigned int bit_idx, unsigned int num_bits)
{
    unsigned int i;
    for (i = bit_idx; i < bit_idx + num_bits; ++i) {
        toggle_bit(i);
    }
}

static unsigned int paddr_for_idx(unsigned int bit_idx)
{
    unsigned int i, current_offset = 0, offset = bit_idx * FOUR_KB;
    for (i = 0; i < mmap_len; ++i) {
        if (current_offset + mmap[i].len <= offset) {
            current_offset += mmap[i].len;
        } else {
            offset -= current_offset;
            return mmap[i].addr + offset;
        }
    }

    return 0;
}

static unsigned int idx_for_paddr(unsigned int paddr)
{
    unsigned int i, byte_offset = 0;
    for (i = 0; i < mmap_len; ++i) {
        if (paddr < mmap[i].addr + mmap[i].len) {
            byte_offset += paddr - mmap[i].addr;
            return byte_offset / FOUR_KB;
        } else {
            byte_offset += mmap[i].len;
        }
    }

    return page_frames.len;
}

static unsigned int fits_in_one_mmap_entry(unsigned int bit_idx, unsigned int pfs)
{
    unsigned int i, current_offset = 0, offset = bit_idx * FOUR_KB;
    for (i = 0; i < mmap_len; ++i) {
        if (current_offset + mmap[i].len <= offset) {
            current_offset += mmap[i].len;
        } else {
            offset -= current_offset;
            if (offset + pfs * FOUR_KB <= mmap[i].len) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    return 0;
}

unsigned int pfa_allocate(unsigned int num_page_frames)
{
    unsigned int i, j, cell, bit_idx;
    unsigned int n = div_ceil(page_frames.len, 32), frames_found = 0;

    for (i = 0; i < n; ++i) {
        cell = page_frames.start[i];
        if (cell != 0) {
            for (j = 0; j < 32; ++j) {
                if (((cell >> (31 - j)) & 0x1) == 1) {
                    if (frames_found == 0) {
                        bit_idx = i * 32 + j;
                    }
                    ++frames_found;
                    if (frames_found == num_page_frames) {
                        if (fits_in_one_mmap_entry(bit_idx, num_page_frames)) {
                            toggle_bits(bit_idx, num_page_frames);
                            return paddr_for_idx(bit_idx);
                        } else {
                            frames_found = 0;
                        }
                    }
                } else {
                    frames_found = 0;
                }
            }
        } else {
            frames_found = 0;
        }
    }

    return 0;
}

void pfa_free(unsigned int paddr)
{
	unsigned int bit_idx = idx_for_paddr(paddr);
	if (bit_idx == page_frames.len) {
		// Overrun
		printf("pfa_free: invalid paddr %X\n", paddr);
	} else {
		toggle_bit(bit_idx);
	}
}

void pfa_free_cont(unsigned int paddr, unsigned int n)
{
	unsigned int i;
	for (i = 0; i < n; i++) {
		pfa_free(paddr + i * FOUR_KB);
	}
}

