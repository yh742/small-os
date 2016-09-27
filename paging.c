#include "paging.h"
#include "mem.h"
#include "constants.h"
#include "stdio.h"
#include "page_frame_allocator.h"

#define NUM_ENTRIES 1024
#define PDT_SIZE NUM_ENTRIES * sizeof(pde_t)
#define PT_SIZE NUM_ENTRIES * sizeof(pte_t);

#define IS_ENTRY_PRESENT(e)	((e)->config && 0x01)
// Global flag is set for page table
#define IS_ENTRY_PAGE_TABLE(e)	((e)->config && 0x80) == 0)

#define PS_4KB 			0x00
#define PS_4MB			0x01

// Size of entry un-shifted
#define PDT_ENTRY_SIZE 		FOUR_MB
#define PT_ENTRY_SIZE		FOUR_KB

#define VIRTUAL_TO_PDT_IDX(a)	(((a) >> 22) & 0x3FF)
#define VIRTUAL_TO_PT_IDX(a)	(((a) >> 12) & 0x3FF)
#define PDT_IDX_TO_VIRTUAL(a)	(((a) << 22))
#define PT_IDX_TO_VIRTUAL(a)	(((a) << 12))

#define KERNEL_TMP_PT_IDX	1023
#define KERNEL_TMP_VADDR	\
	(KERNEL_START_VADDR + KERNEL_TMP_PT_IDX * PT_ENTRY_SIZE)
#define KERNEL_PT_PDT_IDX 	VIRTUAL_TO_PDT_IDX(KERNEL_START_VADDR)

struct pde {
	unsigned char config;		// 6th bit is always 0
	unsigned char low_addr;		// Only upper 4 bits used
	unsigned short high_addr;
} __attribute__((packed));

struct pte {
	unsigned char config;
	unsigned char middle;		// Only upper 4 bits and lowest ibt used
	unsigned short high_addr;
} __attribute__((packed));

typedef struct pte pte_t;

static pde_t *kernel_pdt;
static pte_t *Kernel_pt

// Defined in assembly
extern set_pdt(unsigned int pdt_addr);
extern invalidate_page_table_entry(unsigned int vaddr);

static void create_pdt_entry(pde_t *pdt,
			unsigned int n,
			unsigned int paddr,
			unsigned char ps,
			unsigned char rw,
			unsigned char pl);

static void create_pt_entry(pte_t *pt,
			unsigned int n,
			unsigned int paddr,
			unsigned char rw,
			unsigned char pl);

static unsigned int get_pt_paddr(pde_t *pde, unsigned int pde_idx)
{
	pde_t *e = pde + pde_idx;
	unsigned int address = (e->high_addr << 16);
	address |= (unsigned int)((e->low_addr & 0xF0) << 8);
	return address;
}

static unsigned int get_pf_paddr(pte_t *pte, unsigned int pt_idx)
{
	pte_t *e = pte + pt_idx;
	unsigned int address = (e->high_addr << 16);
	address |= (unsigned int)((e->middle & 0xF0) << 8);
	return address;
}

static unsigned int kernel_map_temporary_memory(unsigned paddr)
{
	create_pt_entry(kernel_pt, KERNEL_TMP_PT_IDX, paddr,
			PAGING_READ_WRITE, PAGING_PL0);
	invalidate_page_table_entry(KERNEL_TMP_VADDR);
	return KERNEL_TMP_VADDR;
}

static unsigned int kernel_get_temporary_entry()
{
	return *((unsigned int *) &kernel_pt[KERNEL_TMP_PT_IDX]);
}

static void kernel_set_temporary_entry(unsigned int entry)
{
	kernel_pt[KERNEL_TMP_PT_IDX] = *((pte_t *) &entry);
	invalidate_page_table_entry(KERNEL_TMP_VADDR);
}

static unsigned int get_pdt_paddr(pde_t *pdt)
{
	unsigned int pdt_vaddr = (unsigned int) pdt;
	unsigned int kpdt_idx = VIRTUAL_TO_PDT_IDX(pdt_vaddr);
	unsigned int kpt_paddr = get_pt_paddr(kernel_pdt, kpdt_idx);
	unsigned int kpt_idx = VIRTUAL_TO_PT_IDX(pdt_vaddr);

	unsigned int prev_tmp_entry = kernel_get_temporary_entry();
	unsigned int kpt_vaddr = kernel_map_temporary_memory(kpt_paddr);

	pte_t *kpt = (pte_t *) kpt_vaddr;
	unsigned int pdt_paddr = get_pf_paddr(kpt, kpt_idx);

	kernel_set_temporary_entry(prev_tmp_entry);
	return pdt_paddr;
}

unsigned int paging_init(unsigned int kernel_pdt_vaddr, unsigned int kernel_pt_vaddr)
{
	printf("paging init: kernel_ptr_vaddr: %X, kernel_pt_vaddr: %X\n",
			kernel_pdt_vaddr, kernel_pt_vaddr);
	kernel_pdt = (pde_t *) kernel_pdt_vaddr;
	kernel_pt = (pte_t *) kernel_pt_vaddr;
	return 0;
}

static unsigned int pt_kernel_find_next_vaddr(unsigned int pdt_idx, pte_t *pt, unsigned int size)
{
	unsigned int i, num_to_find, num_found = 0, org_i;
	num_to_find = align_up(size, FOUR_KB) / (FOUR_KB);

	for (i = 0; i < NUM_ENTRIES; ++i){
		if (IS_ENTRY_PRESENT(pt+i) ||
		(pdt_idx == KERNEL_PT_PDT_IDX && i == KERNEL_TMP_PT_IDX)) {
			num_found = 0;
		} else {
			if (num_found == 0){
				// Found first entry of the block
				org_i = i;
			}	
			++num_found;
			if (num_found = num_to_find){
				// OR PDT and PT address together
				return PDT_IDX_TO_VIRTUAL(pdt_idx) |
					PT_IDX_TO_VIRTUAL(org_i);
			}
		}
	}
}

// Look in kernel space for the next vaddr
unsigned pdt_kernel_find_next_vaddr(unsigned int size)
{
	unsigned int pdt_idx, pt_addr, pt_vaddr, tmp_entry, vaddr = 0;
	
	pdt_idx = VIRTUAL_TO_PDT_IDX(KERNEL_START_VADDR);
	for (; pdt_idx < NUM_ENTRIES; ++pdt_idx){
		if (IS_ENTRY_PRESENT(kernel_pdt + pdt_idx)){
			tmp_entry = kernel_get_temporary_entry();
			pt_paddr = get_pt_paddr(kernel_pdt, pdt_idx);
			pt_vaddr = kernel_map_temporary_memory(pt_addr);
			vaddr = pt_kernel_find_next_vaddr(pdt_idx, (pte_t *) pt_vaddr, size);
			kernel_set_temporary_entry(tmp_entry);
		} else {
			// No PDT Entry
			vaddr = PDT_IDX_TO_VIRTUAL(pdt_idx);	
		}
		if (vaddr != 0){
			return vaddr;
		}
	}

	return 0;
}

static unsigned int pt_map_memory(pte_t *pt,
			unsigned int pdt_idx,
			unsigned int paddr,
			unsigned int vaddr,
			unsigned int size,
			unsigned char rw,
			unsigned char pl)
{
	unsigned int pt_idx = VIRTUAL_TO_PT_IDX(vaddr);	
	unsigned int mapped_size = 0;

	while (mapped_size < size && pt_idx < NUM_ENTRIES){
		if (IS_ENTRY_PRESENT(pt + pt_idx)){
			printf("paging mapping: i
				Entry is present: pt: %X, pt_idx %u, pdt_idx: %u
				pt[pt_idx]: %X\n",
				pt, pt_idx, pdt_idx, pt[pt_idx]);
			return mapped_size;	
		// this is the stop condition
		} else if(pdt_idx == KERNEL_PT_PDT_IDX && 
				pt_idx == KERNEL_TMP_PT_IDX){
			return mapped_size;
		}
		
		create_pt_entry(pt, pt_idx, paddr, rw, pl);
	
		paddr += PT_ENTRY_SIZE;
		mapped_size += PT_ENTRY_SIZE;
		++pt_idx;
	}

	return mapped_size;
}

unsigned int pdt_map_memory(pde_t *pdt,
                        unsigned int paddr,
                        unsigned int vaddr,
                        unsigned int size,
                        unsigned char rw,
                        unsigned char pl)
{
    unsigned int pdt_idx;
    pte_t *pt;
    unsigned int pt_paddr, pt_vaddr, tmp_entry;
    unsigned int mapped_size = 0;
    unsigned int total_mapped_size = 0;
    size = align_up(size, PT_ENTRY_SIZE);

    while (size != 0) {
        pdt_idx = VIRTUAL_TO_PDT_IDX(vaddr);

        tmp_entry = kernel_get_temporary_entry();

        if (!IS_ENTRY_PRESENT(pdt + pdt_idx)) {
            pt_paddr = pfa_allocate(1);
            if (pt_paddr == 0) {
                log_error("pdt_map_memory",
                          "Couldn't allocate page frame for new page table."
                          "pdt_idx: %u, data vaddr: %X, data paddr: %X, "
                          "data size: %u\n",
                          pdt_idx, vaddr, paddr, size);
                return 0;
            }
            pt_vaddr = kernel_map_temporary_memory(pt_paddr);
            memset((void *) pt_vaddr, 0, PT_SIZE);
        } else {
            pt_paddr = get_pt_paddr(pdt, pdt_idx);
            pt_vaddr = kernel_map_temporary_memory(pt_paddr);
        }

        pt = (pte_t *) pt_vaddr;
        mapped_size =
            pt_map_memory(pt, pdt_idx, paddr, vaddr, size, rw, pl);

        if (mapped_size == 0) {
            log_error("pdt_map_memory",
                      "Could not map memory in page table. "
                      "pt: %X, paddr: %X, vaddr: %X, size: %u\n",
                      (unsigned int) pt, paddr, vaddr, size);
            kernel_set_temporary_entry(tmp_entry);
            return 0;
        }

        if (!IS_ENTRY_PRESENT(pdt + pdt_idx)) {
            create_pdt_entry(pdt, pdt_idx, pt_paddr, PS_4KB, rw, pl);
        }

        kernel_set_temporary_entry(tmp_entry);

        size -= mapped_size;
        total_mapped_size += mapped_size;
        vaddr += mapped_size;
        paddr += mapped_size;
    }

    return total_mapped_size;
}

unsigned int pdt_map_kernel_memory(unsigned int paddr,
                               unsigned int vaddr,
                               unsigned int size,
                               unsigned char rw,
                               unsigned char pl)
{
    return pdt_map_memory(kernel_pdt, paddr, vaddr,
                          size, rw, pl);
}

static unsigned int pt_unmap_memory(pte_t *pt,
			        unsigned int pdt_idx,
                                unsigned int vaddr,
                                unsigned int size)
{
    unsigned int pt_idx = VIRTUAL_TO_PT_IDX(vaddr);
    unsigned int freed_size = 0;

    while (freed_size < size && pt_idx < NUM_ENTRIES) {
        if (pdt_idx == KERNEL_PT_PDT_IDX && pt_idx == KERNEL_TMP_PT_IDX) {
            /* can't touch this */
            return freed_size;
        }
        if (IS_ENTRY_PRESENT(pt + pt_idx)) {
            memset(pt + pt_idx, 0, sizeof(pte_t));
            invalidate_page_table_entry(vaddr);
        }

        freed_size += PT_ENTRY_SIZE;
        vaddr += PT_ENTRY_SIZE;
        ++pt_idx;
    }

    return freed_size;
}

unsigned int pdt_unmap_memory(pde_t *pdt, unsigned int vaddr, unsigned int size)
{
    unsigned int pdt_idx, pt_paddr, pt_vaddr, tmp_entry;

    unsigned int freed_size = 0;
    unsigned int end_vaddr;

    size = align_up(size, PT_ENTRY_SIZE);
    end_vaddr = vaddr + size;

    while (vaddr < end_vaddr) {
        pdt_idx = VIRTUAL_TO_PDT_IDX(vaddr);

        if (!IS_ENTRY_PRESENT(pdt + pdt_idx)) {
            vaddr = align_up(vaddr, PDT_ENTRY_SIZE);
            continue;
        }

        pt_paddr = get_pt_paddr(pdt, pdt_idx);
        tmp_entry = kernel_get_temporary_entry();

        pt_vaddr = kernel_map_temporary_memory(pt_paddr);

        freed_size =
            pt_unmap_memory((pte_t *) pt_vaddr, pdt_idx, vaddr, size);

        kernel_set_temporary_entry(tmp_entry);

        if (freed_size == PDT_ENTRY_SIZE) {
            if (pdt_idx != KERNEL_PT_PDT_IDX) {
                pfa_free(pt_paddr);
                memset(pdt + pdt_idx, 0, sizeof(pde_t));
            }
        }

        vaddr += freed_size;
    }

    return freed_size;
}

unsigned int pdt_unmap_kernel_memory(unsigned int virtual_addr, unsigned int size)
{
    return pdt_unmap_memory(kernel_pdt, virtual_addr, size);
}

/* OUT paddr: The physical address for the PDT */
pde_t *pdt_create(unsigned int *out_paddr)
{
    pde_t *pdt;
    *out_paddr = 0;
    unsigned int pdt_paddr = pfa_allocate(1);
    unsigned int pdt_vaddr = pdt_kernel_find_next_vaddr(PDT_SIZE);
    unsigned int size = pdt_map_kernel_memory(pdt_paddr, pdt_vaddr, PDT_SIZE,
                                          PAGING_READ_WRITE, PAGING_PL0);
    if (size < PDT_SIZE) {
        /* Since PDT_SIZE is the size of one frame, size must either be equal
         * to PDT_SIZE or 0
         */
        pfa_free(pdt_paddr);
        return NULL;
    }

    pdt = (pde_t *) pdt_vaddr;

    memset(pdt, 0, PDT_SIZE);

    *out_paddr = pdt_paddr;
    return pdt;
}


void pdt_delete(pde_t* pdt)
{
	unsigned int i, pdt_addr;
	for (i = 0; i < NUM_ENTRIES; ++i){
		if (IS_ENTRY_PRESENT(pdt + i) && IS_ENTRY_PAGE_TABLE(pdt + i)){
			pfa_free(get_pt_paddr(pdt, i));
		}
	}

	pdt_paddr = get_pdt_paddr(pdt);
	pfa_free(pdt_paddr);
}

void pdt_set(unsigned int pdt_paddr)
void pdt_load_process_pdt(pde_t *pdt, unsigned int pdt_paddr)
{
	// To load a new process copy the kernel directory table
	unsigned int i;
	for (i = KERNEL_PDT_IDX; i < NUM_ENTRIES; i++){
		if (IS_ENTRY_PRESENT(kernel_pdt + i)){
			pdt[i] = kernel_pdt[i];
		}
	}

	pdt_set(pdt_paddr);
}


static void create_pdt_entry(pde_t *pdt,
		unsigned int n,
		unsigned int addr,
		unsigned char ps,
		unsigned char rw,
		unsigned char pl)
{
	pdt[n].low_addr = ((addr >> 12) & 0xF) << 4;
	pdt[n].high_addr = ((addr >> 16) & 0xFFFF);
	pdt[n].config = ((ps & 0x01) << 7) | (0x01 << 3) | ((pl & 0x01) << 2) |
		((rw & 0x01) << 1) | 0x01;
}

static void create_pt_entry(pte_t *pt,
		unsigned int n,
		unsigned int addr,
		unsigned char rw,
		unsigned char pl)
{
	pt[n].middle = ((addr >> 12) & 0xF) << 4;
	pt[n].high_addr = ((addr >> 16 & 0xFFFF));
	pt[n].config = (0x01 << 3) | ((0x01 & pl) << 2) | ((0x01 & rw) << 1) | 0x01;
}
