; isr_asm.s
; All our low-level interrupt code, just a bunch of functions that calls the
; C-functions and cleans up the stack.
; Heavily inspired by: http://www.jamesmolloy.co.uk/tutorial_html/


[BITS 32]


[EXTERN isr_handler]
[EXTERN irq_handler]
[EXTERN intr_handler]


[EXTERN process_init]

[GLOBAL call_process_init]
[GLOBAL trap_ret]

%macro ISR_NOERRCODE 1
	[GLOBAL isr%1]
	isr%1:
		push byte 0
		push byte %1
		jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
	[GLOBAL isr%1]
	isr%1:
		push byte %1
		jmp isr_common_stub
%endmacro

%macro IRQ 2
	global irq%1
		irq%1:
		push dword 0
		push dword %2
		jmp irq_common_stub
%endmacro

%macro INTR_NOERRCODE 1
	[GLOBAL intr%1]
	intr%1:
		push byte 0
		push byte %1
		jmp intr_common_stub
%endmacro




; TODO: Use this stub for all the interrupts
%macro STUB 2
	%1:
		; Push all the general purpuse registers, it's reverse of the order given in
		; Registers in arch.h
		pusha

		xor eax, eax
		mov ax, ds
		push eax

		mov ax, es
		push eax

		mov ax, fs
		push eax

		mov ax, gs
		push eax

		; When we start changing to user mode, I need to make sure we can change to
		; kernel and back to user mode when we leave
		mov ax, 0x10	; load the kernel data segment descriptor
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax

		mov eax, esp
		push eax

		call %2

		add esp, 4

	.ret_%1:
		mov eax, 0x10
		pop ebx	; gs
		mov gs, ax

		pop ebx	; fs
		mov fs, ax

		pop ebx	; es
		mov es, ax

		pop ebx	; ds
		mov ds, ax

		popa		; Pop the stack in reverse order as pusha

		; Clean up from the error code and the interrupt number
		add esp, 8

		iret
%endmacro




ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31


IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47


INTR_NOERRCODE 63
INTR_NOERRCODE 64


; Syscall
ISR_NOERRCODE 128


STUB intr_common_stub, intr_handler
STUB isr_common_stub,  isr_handler
STUB irq_common_stub,  irq_handler



call_process_init:
	pushfd
	mov ax, cs
	push eax
	push 0x0		; EIP
	pushad
	mov ax, ds
	push eax
	call process_init
	pop ebx
	popad
	add esp, 0x10
	ret

