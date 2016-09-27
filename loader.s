%include "constants.inc"

global loader
global kernel_stack

; Kernel Entry
extern kmain

; Kernel Virtual and Physical Address
extern kernel_virtual_end
extern kernel_virtual_start
extern kernel_physical_end
extern kernel_physical_start

; Multiboot Magic
MAGIC_NUMBER		equ 	0x1BADB002
MODULE_ALIGN		equ	1<<0
MEM_INFO		equ	1<<1
FLAGS			equ 	MODULE_ALIGN | MEM_INFO		
CHECKSUM		equ	-(MAGIC_NUMBER + FLAGS)

; Settting for page tables /4096Kb, Writable, Present
KERNEL_PT_CFG		equ	000000000000000000000000000001011b
; Initial setting used for identity mapping the first 4MB physical <-> virtual
; Otherwise, right after paging is turned on the program will crash since nothing is mapped ~1MB
; This needs to be invalidate later
KERNEL_IDENTITY_MAP	equ	000000000000000000000000010001011b

section .data
align 4096
kernel_pt:
	times 1024 dd 0
kernel_pdt:
 	; Setup the first page directory entry to identity map value so the values directly map to physical memory
	dd KERNEL_IDENTITY_MAP
	times 1023 dd 0

section .data
align 4
grub_magic_number:
	dd 0
grub_multiboot_info:
	dd 0

section .bss
align 4
kernel_stack: 
	resb KERNEL_STACK_SIZE

section .text
align 4
	dd MAGIC_NUMBER
	dd FLAGS
	dd CHECKSUM

; Entry Point
loader: 
	; Tags refers to virtual address -> 
	; MUST use physical address before paging is enabled and page table is filled
	; Save the Grub Headers first
	mov ecx, (grub_magic_number - KERNEL_START_VADDR)	
	mov [ecx], eax
	mov ecx, (grub_multiboot_info - KERNEL_START_VADDR)
	mov [ecx], ebx

	; Setup page directory table and page tables
	; 1024 PDT x 1024 PT x 4096 Kb = 4GB addressable space
set_up_kernel_pdt:
	; KERNEL_PDT_IDX is the page entry where the kernel is located ~0xC0000000
	mov ecx, (kernel_pdt - KERNEL_START_VADDR + KERNEL_PDT_IDX * 4) 
	mov edx, (kernel_pt - KERNEL_START_VADDR)	
	or edx, KERNEL_PT_CFG
	mov [ecx], edx

set_up_kernel_pt:
	mov eax, (kernel_pt - KERNEL_START_VADDR)	
	;Config for page table and directory are the same
	mov ecx, KERNEL_PT_CFG		
;fill up the page table with 1024 four byte entries
.loop:
	mov [eax], ecx
	add eax, 4
	; adds 4kb directly to kernel config since 10 high bits are address
	add ecx, FOUR_KB
	cmp ecx, kernel_physical_end
	jle .loop

enable_paging:
	mov ecx, (kernel_pdt - KERNEL_START_VADDR)
	and ecx, 0xFFFFF000
	or ecx, 0x08	; enable page write-through? not needed..
	; write-through memory = cache, write back memory is written to when cache entries are evicted
	mov cr3, ecx	; load pdt
	
	mov ecx, cr4
	or ecx, 0x00000010	; set to 4MB mode to allow identity mapping
	mov cr4, ecx	
	
	mov ecx, cr0
	or ecx, 0x80000000	; set to allow paging
	mov cr0, ecx

	lea ecx, [higher_half]
	; Must use register to do absolute address jump
	jmp ecx


higher_half:
	mov [kernel_pdt], DWORD 0
	invlpg [0]
	mov esp, kernel_stack+KERNEL_STACK_SIZE

enter_kmain:
	push kernel_pt
	push kernel_pdt
	push kernel_virtual_end
	push kernel_virtual_start
	push kernel_physical_end
	push kernel_physical_start
	push DWORD [grub_magic_number]
	push DWORD [grub_multiboot_info]

hang:
	jmp hang
			
