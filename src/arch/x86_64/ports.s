[BITS 64]

[global outb]
[global outw]
[global inb]
[global inw]

outb:
	mov edx, edi
	mov eax, esi
	out dx, al
	ret

outw:
	mov edx, edi
	mov eax, esi
	out dx, ax
	ret

inb:
	mov edx, edi
	in al, dx
	ret

inw:
	mov edx, edi
	in ax, dx
	ret

