; task.s
; Some of the code for doing software task switching

[BITS 32]

[GLOBAL return_fork]
[GLOBAL task_enter_usermode]

return_fork:
	mov eax, 0
	mov edx, [esp+0]
	mov ecx, [esp+4]
	mov esp, ecx
	jmp edx


; About iret instruction
; - Expects to find data in the following order:
;  - SS
;  - ESP
;  - EFLAGS
;  - CS
;  - EIP
; - If eflags is XOR-ed with 0x200, interrupts are enabled again

; INPUT:
; - Return EIP
task_enter_usermode:
	cli
	;mov ebx, [esp+4]	; Get EIP
	; TODO: Should have this as parameter, not hardcode it
	mov ebx, 0x40000000
	mov eax, (0x20 | 0x03)
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, esp
	push (0x20 | 0x03)	; SS
	push eax			; ESP
	pushf				; EFLAGS

	pop eax				; EFLAGS
	or eax, 0x200
	push eax

	push (0x18 | 0x03)	; SS
	push ebx			; EIP
	iret
