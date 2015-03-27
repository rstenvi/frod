

all: kernel
	cp src/kernel iso/boot
	cp src/arch/x86/bootap.bin iso/
	$(GRUB_RESCUE) -o image.iso iso/

kernel:
	make -C ./src

run-bochs: all
	$(BOCHS) -q -f .bochsrc.txt

# SMP does not work with more than 1 CPU on Qemu
run-qemu: all
	qemu-system-i386 -cdrom image.iso -smp 1
#	qemu-system-x86_64 -cdrom image.iso

doc:
	make -C ./doc

clean:
	make -C ./src clean
	-rm -f bochsout.txt iso/boot/kernel image.iso

fclean:
	make -C ./src clean
	make -C ./doc fclean
	-rm -f bochsout.txt iso/boot/kernel image.iso

.PHONY: all clean fclean kernel doc run-qemu run-bochs
