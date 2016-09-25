global loader

extern kmain

MAGIC_NUMBER		equ 	0x1BADB002
ALIGN_MODULES		equ 	0x00000001	; this is the flag field aligns on 4kB boundary
;FLAGS			equ 	0x0
CHECKSUM		equ	-(MAGIC_NUMBER + ALIGN_MODULES)
KERNEL_STACK_SIZE	equ 	4096

section .bss
align 4
kernel_stack: 
	resb KERNEL_STACK_SIZE

section .text
align 4
	dd MAGIC_NUMBER
	dd ALIGN_MODULES
	dd CHECKSUM

loader: 
	mov eax, 0xCAFEBABE
	mov esp, kernel_stack + KERNEL_STACK_SIZE
	
	push ebx
	call kmain
;	push dword 3
;	push dword 2
;	push dword 1
;	call sum_of_three

.hang:
	jmp .hang
				
