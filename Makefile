GRUB ?= $(HOME)/prog/grub/bin/
MKRESCUE = env PATH=$$PATH:$(GRUB) grub-mkrescue

TARGETS = os

SRC = $(wildcard src/**/*.cpp)
ASM = $(wildcard src/**/*.S)
INCLUDE = $(wildcard include/**/*.hpp) $(wildcard include/**/*.h)

.PHONY: $(TARGETS)

LD = g++
CC = clang

CFLAGS = -ffreestanding -nostdlib -std=c++17 -static -fno-stack-protector -m32 -fno-PIC
LDFLAGS = -Wl,-melf_i386

all: $(TARGETS)

os: $(SRC) $(ASM) $(INCLUDE)
	$(LD) -o $@.out -Iinclude -T linkscript $(CFLAGS) $(LDFLAGS) $(SRC) $(ASM)

%.o: %.S
	$(CC) -o %$< -c $(CFLAGS) $@

%.o: %.cpp
	$(CC) -o %$< -Iinclude -c $(CFLAGS) $@

os.img: os
	mkdir -p _boot/boot/grub
	cp os.out _boot
	cp boot/grub.cfg _boot/boot/grub/
	cp -r src/* _boot
	$(MKRESCUE) -o $@ _boot
	rm -rf _boot

test: os.img
	qemu-system-i386 -serial stdio -cdrom os.img

clean:
	rm -f *.o
	rm -f os.out
	rm -f os.img
