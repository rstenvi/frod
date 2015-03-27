; boot_ap.s
; Entry point of the application processors (AP)
; Processor is started in real mode, so we have to do much of the same job as a
; bootloader would have to. 
;
; This code will be stored at address 0x7000

[BITS 16]
[SECTION .text]

; Entry point must be at the beginning
jmp apstart

; Align on 4 so we can treat this area as an uint32_t array
; First index is 1
ALIGN 4
stack_addr:  dd 0	; Virtual stack address
entry_point: dd 0
page_dir:    dd 0	; Physical address to dir that should be loaded


apstart:
	cli

	; Set up the segment registers
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; Load temporary GDT
	lgdt [gdt_ptr]

	; Enable protected mode
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	jmp 0x08:pmode

[BITS 32]

pmode:
	; We are now in protected mode with temporary GDT
	mov word ax, 0x0010	; GDT data segment
	mov ds, ax
	mov es, ax
	mov ss, ax

	xor ax, ax
	mov fs, ax
	mov gs, ax

	mov eax, [page_dir]
	mov cr3, eax

	mov eax, cr0
	or eax, (1 << 31)
	mov cr0, eax

	; Stack allocated in main()
	mov dword esp, [stack_addr]
	mov dword eax, [entry_point]
	cld
	call eax

.loop:
	jmp .loop
	
gdt_start:
	dd 0, 0 ; Required null descriptor
	
	; Code (0x08)
	dw 0xffff
	dw 0x0000
	db 0x00
	db 10011010b
	db 11001111b
	db 0x00

	; Data (0x10)
	dw 0xffff
	dw 0x0000
	db 0x00
	db 10010010b
	db 11001111b
	db 0x00
gdt_end:

; Pointer that is actually loaded with lgdt
gdt_ptr:
	dw gdt_end - gdt_start - 1 
	dd gdt_start

append:
