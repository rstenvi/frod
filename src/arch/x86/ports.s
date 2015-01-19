; ports.s
; Deals with sending/receiving data to/from ports

[BITS 32]

[global outb]
[global outw]
[global inb]
[global inw]

outb:
	mov dx, [esp+4]
	mov al, [esp+8]
	out dx, al
	ret

outw:
	mov dx, [esp+4]
	mov ax, [esp+8]
	out dx, ax
	ret

inb:
	mov dx, [esp+4]
	in al, dx
	ret

inw:
	mov dx, [esp+4]
	in ax, dx
	ret

