LD = gcc
CC = clang

CFLAGS = -ffreestanding -nostdlib -std=c++17 -static -fno-stack-protector -m32 -fno-PIC
LDFLAGS = -Wl,-melf_i386

GRUB ?= $(HOME)/prog/grub/bin/
MKRESCUE = env PATH=$$PATH:$(GRUB) grub-mkrescue

CPP = $(wildcard src/*.cpp)
ASM = $(wildcard src/*.S)

INCLUDE = $(wildcard include/*.h)

kernel: $(CPP) $(ASM) $(INCLUDE)
	$(LD) -o $@.out -Iinclude -T linkscript $(CFLAGS) $(LDFLAGS) $(CPP) $(ASM)

%.o: %.S
	$(CC) -o $< -c $(CFLAGS) $@

%.o: %.cpp
	$(CC) -o $< -Iinclude -c $(CFLAGS) $@

clean:
	rm -f *.o
	rm -f kernel.out
