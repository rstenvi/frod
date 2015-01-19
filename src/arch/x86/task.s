; task.s
; Some of the code for doing software task switching

[BITS 32]

[GLOBAL return_fork]

return_fork:
	mov eax, 0
	mov edx, [esp+0]
	mov ecx, [esp+4]
	mov esp, ecx
	jmp edx

