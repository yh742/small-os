global gdt_load_and_set

SEG_KERNEL_CS		equ	0x08
SEG_KERNEL_DS		equ	0x10

section .text

gdt_load_and_set:
	mov eax, [esp + 4]
	lgdt [eax]
	
	; hack for changing CS to the code segment
	jmp SEG_KERNEL_CS:.reload_segments

.reload_segments:
	mov ax, SEG_KERNEL_DS
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov gs, ax
	mov fs, ax
	ret

	
