; cpu_asm.s
; Various function and helper-functions to get information about the CPU.

; --------------------- Global functions -------------------------
[GLOBAL get_vendor_id]
[GLOBAL cpuid_supported]
[GLOBAL flush_caches]
[GLOBAL interrupt_enabled]
[GLOBAL read_eip]



; ----------------- Global function implementations -------------

; Get the vendor ID, function is passed a pointer that can hold 12 characters
; pluss an additional 0-byte.
get_vendor_id:
	; Create stack frame
	push ebp
	mov ebp, esp
	
	; Get vendor ID
	mov eax, 0	; 0 to get vendor ID
	cpuid		; Place result in EBX, EDX and ECX (in that order)

	; Place parameter in eax
	mov dword eax, [ebp+8]	; Place address
	
	; Place the 12-character string in the address of eax
	mov dword [eax], ebx
	mov dword [eax+4], edx
	mov dword [eax+8], ecx

	; 0-byte at the end
	xor ebx, ebx
	mov byte [eax+12], bl
	
	; Result is also returned in the parameter given
	mov ebx, eax
	
	pop ebp	; Restore base pointer
	ret

; Check if CPUID instruction is supported by testing if bit 0x200000 in EFLAGS
; can be modified.
; Returns 1 if CPUID is supported, 0 if it's not, it also sets the ZF flag
cpuid_supported:
	; Create stack frame
	push ebp
	mov ebp, esp
	
	pushfd				; Push flags on the stack
	pop eax				; Get flags
	mov ecx, eax		; Save flags
	xor eax, 0x200000	; Modify
	push eax				; Push flags
	popfd					; Restore flags

	pushfd			; Push flags again
	pop eax			; Get flags
	xor eax, ecx	; Get changed bits
	shr eax, 21		; Move bit 21 to bit 0
	and eax, 1		; Unset all other bits

	push ecx		; Push original flags
	popfd		; Set original flags

	pop ebp	; Restore base pointer
	ret


; Flush the cache
flush_caches:
	push ebp
	mov ebp, esp

	; Check if interrupts are enabled
	call interrupt_enabled

	; If interrupts are enabled, we diable them and enable them afterwards,
	; otherwise we just invalidate the cache
	jnz .enableint
	invd
	jmp .end
.enableint:
	cli
	invd
	sti

.end:
	pop ebp	; Restore base pointer
	ret



; Checks if interrupts are enabled.
; Returns 1 if they are enabled, 0 if they are not, ZF is also set
interrupt_enabled:
	push ebp
	mov ebp, esp

	; Get eflags in eax
	pushfd
	pop eax
	push eax
	popfd

	; Test the 9th bit, interrupt enable flag (IF)
	shr eax, 9
	and eax, 1	; Sets the ZF bit appropriately

	pop ebp	; Restore base pointer
	ret


read_eip:
	pop eax	; Get return address in eax
	jmp eax	; Jump to return address


