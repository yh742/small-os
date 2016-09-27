global pdt_set
global invalidate_page_table_entry

section .text

pdt_set:
	mov eax, [esp + 4]
	and eax, 0xFFFFF000
	or eax, 0x08
	mov cr3, eax
	ret

invalidate_page_table_entry:
	mov eax, [esp + 4]
	invlpg [eax]
	ret
