; boot.s
; Entry point from the bootloader.

[BITS 32]
[SECTION .text]


[GLOBAL start]
[EXTERN kmain]
[EXTERN kern_phys_end]
[EXTERN kern_phys_start]



; Make sure we are multiboot compliant
MULTIBOOT_PAGE_ALIGN    equ 1<<0
MULTIBOOT_MEMORY_INFO   equ 1<<1
MULTIBOOT_HEADER_MAGIC  equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS  equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

ALIGN 4
multiboot_header:
	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM


start:
	mov esp, sys_stack
	push esp

	push kern_phys_end
	push kern_phys_start

	push ebx	; Multiboot structure

	call kmain
	jmp $


[section .bss]
	resb 0x1000
	sys_stack:
