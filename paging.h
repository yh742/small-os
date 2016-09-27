#ifndef PAGING_H_
#define PAGING_H_

#define PAGING_READ_ONLY	0
#define PAGING_WRITE_ONLY 	1
#define PAGING_PL0		0
#define PAGING_PL3		1

typedef struct pde pde_t;

unsigned int paging_init(unsigned int kernel_pdt_vaddr, 
		unsigned int kernel_pt_vaddr);

unsigned int pdt_kernel_find_next_vaddr(unsigned int size);

unsigned int pdt_map_kernel_memory(unsigned int paddr,
				unsigned int vaddr,
				unsigned int size,
				unsigned char rw,
				unsigned char pl);

unsigned int pdt_unmap_kernel_memory(unsigned int vaddr, 
				unsigned int size);
unsigned int pdt_unmap_memory(pde_t *pdt,
			unsigned int paddr,
			unsigned int vaddr,
			unsigned int size,
			unsigned char rw,
			unsigned char pl);

pde_t *pdt_create(unsigned int *out_paddr);

void pdt_delete(pde_t *pdt);

void pdt_load_process_pdt(pde_t *pdt, unsigned int pdt_paddr);

#endif
