; flush.s
; GDT, IDT and TSS flush

[BITS 32]

[GLOBAL gdt_flush]
[GLOBAL idt_flush]
[GLOBAL tss_flush]

; Write the GDT pointer
; Takes address to GDT pointer as argument (uint32_t)
gdt_flush:
	mov eax, [esp+4]	; Load parameter in eax
	lgdt [eax]			; Load GDT

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; Changes to CS register to 0x08
	jmp 0x08:.flush
.flush:
	ret


; Write the IDT pointer
; Takes address to IDT pointer as argument (uint32_t)
idt_flush:
	mov eax, [esp+4]
	lidt [eax]
	ret


tss_flush:
	mov eax, [esp+4]
	or eax, 0x03
	ltr ax
	ret
