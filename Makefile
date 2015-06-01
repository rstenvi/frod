

all: kernel
	cp src/kernel iso/boot
	cp src/arch/x86/bootap.bin iso/
	cp src/tools/sc2.bin iso/
	$(GRUB_RESCUE) -o image.iso iso/

kernel:
	make -C ./src

run-bochs: all
	$(BOCHS) -q -f .bochsrc.txt

# SMP does not work with more than 1 CPU on Qemu
# Keyboard doesn't work either
run-qemu: all
	qemu-system-i386 -cdrom image.iso -smp 1
#	qemu-system-x86_64 -cdrom image.iso

doc:
	make -C ./doc

clean:
	make -C ./src clean
	-rm -f bochsout.txt iso/boot/kernel image.iso serial.txt

fclean: clean
	make -C ./doc fclean

.PHONY: all clean fclean kernel doc run-qemu run-bochs
