; cpu_asm.s
; Various function and helper-functions to get information about the CPU.

; --------------------- Global functions -------------------------
[GLOBAL get_vendor_id]
[GLOBAL cpuid_supported]
[GLOBAL flush_caches]
[GLOBAL interrupt_enabled]
[GLOBAL read_eip]
[GLOBAL cpu_supported]

; CPUID specification from Intel
; http://bochs.sourceforge.net/techspec/24161821.pdf

%define CPUID_CHECK_MASK_FPU   (1)
%define CPUID_CHECK_MASK_VME   (1<<1)
%define CPUID_CHECK_MASK_DE    (1<<2)
%define CPUID_CHECK_MASK_PSE   (1<<3)
%define CPUID_CHECK_MASK_TSC   (1<<4)
%define CPUID_CHECK_MASK_MSR   (1<<5)
%define CPUID_CHECK_MASK_PAE   (1<<6)
%define CPUID_CHECK_MASK_MCE   (1<<7)
%define CPUID_CHECK_MASK_CX8   (1<<8)
%define CPUID_CHECK_MASK_APIC  (1<<9)
; 10
%define CPUID_CHECK_MASK_SEP   (1<<11)
%define CPUID_CHECK_MASK_MTRR  (1<<12)
%define CPUID_CHECK_MASK_PGE   (1<<13)
%define CPUID_CHECK_MASK_MCA   (1<<14)
%define CPUID_CHECK_MASK_CMOV  (1<<15)
%define CPUID_CHECK_MASK_PAT   (1<<16)
%define CPUID_CHECK_MASK_PSE36 (1<<17)
%define CPUID_CHECK_MASK_PSN   (1<<18)
%define CPUID_CHECK_MASK_CLFSH (1<<19)
; 20
%define CPUID_CHECK_MASK_DS    (1<<21)
%define CPUID_CHECK_MASK_ACPI  (1<<22)
%define CPUID_CHECK_MASK_MMX   (1<<23)
%define CPUID_CHECK_MASK_FXSR  (1<<24)
%define CPUID_CHECK_MASK_SSE   (1<<25)
%define CPUID_CHECK_MASK_SSE2  (1<<26)
%define CPUID_CHECK_MASK_SS    (1<<27)
%define CPUID_CHECK_MASK_HTT   (1<<28)
%define CPUID_CHECK_MASK_TM    (1<<29)
%define CPUID_CHECK_MASK_IA61  (1<<30)
%define CPUID_CHECK_MASK_PBE   (1<<31)


; ----------------- Global function implementations -------------


; Check if CPUID instruction is available
cpu_cpuid_available:
	push ebp
	mov ebp, esp
	
	; Store real and the one we will modify
	pushfd
	pushfd

	; Modify the ID bit
	pop eax
	xor eax, 0x00200000
	push eax
	popfd

	pushfd
	pop eax	; Modified
	pop ebx	; Unmodified

	push ebx
	popfd	; Original


	xor eax, ebx
	and eax,0x00200000

	pop ebp	; Restore base pointer
	ret

; INPUT:
; - EAX = bitmask
; OUTPUT
; - EAX is non-zero if available
; - EAX is zero and ZF-flag is set if not available
cpu_has_id_set:
	push ebp
	mov ebp, esp
	push eax

	mov eax, 1
	cpuid
	pop eax
	and eax, edx

	pop ebx
	ret


cpu_supported:
	push ebp
	mov ebp, esp
	
	call cpu_cpuid_available
	jz .fail

	mov eax, CPUID_CHECK_MASK_APIC
	or eax, CPUID_CHECK_MASK_MSR
	call cpu_has_id_set
	jz .fail
	
	mov eax, 1
	pop ebp	; Restore base pointer
	ret

.fail:
	xor eax, eax
	pop ebp	; Restore base pointer
	ret


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


