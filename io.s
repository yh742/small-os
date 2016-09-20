global outb	;write a byte
global inb	;read a byte
global inw	;read a word

section .text

;[esp + 8] the date byte
;[esp + 4] the address

outb:
	mov al, [esp + 8]
	mov dx, [esp + 4]
	out dx, al
	ret

inb:
	mov dx, [esp + 4]
	in al, dx
	ret

inw:
	mov dx, [esp + 4]
	in ax, dx
	ret
