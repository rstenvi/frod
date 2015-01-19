

# The following variables configures the system.
# A change here will nake take effect unless everything is rebuilt (do make
# clean).


# Decides the architecture, possible values are:
# - x86
# - x86_64 does NOT compile
ARCH=x86
#ARCH=x86_64



# Decides if the kernel built is a test or a real kernel
# Should be "-D TEST_KERNEL" if we are running a test, nothing if we are not.
#TEST=-D TEST_KERNEL
TEST=



# Decides how much information is printed, possible values are:
# - 0 - Print all debug info + extra info to Bochs serial console
# - 1 - Print all debug info to console, but NOT Bochs serial console
# - 2 - Print low-level information about what is happening
# - 3 - Print high-level information about what is happening
# - 4 - Print warning messages
# - 5 - Print non-fatal error messages
# - 6 - Print info messages for user
# - 7 - Print fatal error messages
# One value includes all higher values.
# A value of 0 requires that the emulator is Bochs, useful for debugging.
# Maximum value should be 6 for a functioning system
# 3 is an ok middle ground 
DEBUG=3



# The remaining variables should NOT be changed

LDFLAGS=-T linker-$(ARCH).ld
LDFLAGS2=-nostdlib -ffreestanding -lgcc -z max-page-size=0x1000

CFLAGS=-g -nostdlib -ffreestanding -O2 -Wall -Wextra -I$(INCLUDE) $(TEST) -D DEBUG=$(DEBUG) -D $(ARCH)
NASM=nasm
AS=nasm


ifeq ($(ARCH),x86)
	ASFLAGS=-felf
	CC=i586-elf-gcc
	LD=i586-elf-gcc
	CFLAGS+=-m32
else ifeq ($(ARCH),x86_64)
	ASFLAGS=-felf64
	CC=x86_64-elf-gcc
	LD=x86_64-elf-gcc
	CFLAGS+=-m64 -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow
endif
