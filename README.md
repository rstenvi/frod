# frod
A toy operating system

Simple toy operating system that is unfinished. 

# Building
Can be built with a GCC Cross-compiler and Bochs or Qemu for the emulation.

Steps:
- Bochs executable must be placed in $BOCHS
- grub-mkrescue must be placed in $GRUB\_RESCUE
- Looks for cross-compiler i586-elf-gcc in path
- make run-boch or make run-qemu

