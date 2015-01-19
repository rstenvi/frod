; memory_asm.s
; Routines to get som basic information about memory, NOT amount of memory and
; memory maps, that we get from GRUB.

[BITS 32]

[GLOBAL paging_enabled]
[GLOBAL get_page_dir_addr]
[GLOBAL load_page_dir_addr]
[GLOBAL paging_enable]
[GLOBAL mov_esp_base]
[GLOBAL flush_tlb_entry]

[EXTERN interrupt_enabled]


mov_esp_base:
	mov eax, [esp+4]
	mov esp, eax
	ret

; Returns 1 if paging is enabled, 0 if it's not, ZF-flag is also set
; appropriately.
paging_enabled:
	mov eax, cr0
	shr eax, 31
	and eax, 1
	ret

; Get the address that is loaded in CR3 (paging)
get_page_dir_addr:
	mov eax, cr3
	ret

; Change page directory
load_page_dir_addr:
	mov eax, [esp+4]
	mov cr3, eax
	ret

; Enable paging, address is passed on the stack.
paging_enable:
	; Load the pagedir in cr3
	mov eax, [esp+4]
	mov cr3, eax

	; Set bit 31 in cr0 to enable paging
	mov eax, cr0
	or eax, 1 << 31

	; Don't allow supervisor to write to read-only pages
	or eax, 1 << 16
	mov cr0, eax
	ret

; Flushes TLB entry. Disables and enables interrupts if necessary.
flush_tlb_entry:
	push ebp
	mov ebp, esp

	mov eax, [esp+4]

	call interrupt_enabled
	jnz .enableint
	invlpg [eax]
	jmp .end

.enableint:
	cli
	invlpg [eax]
	sti

.end:
	mov esp, ebp
	pop ebp
	ret
