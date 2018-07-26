LD = gcc
CC = clang

CFLAGS = -ffreestanding -nostdlib -std=c++17 -static -fno-stack-protector -m32 -fno-PIC	\
		 -fno-rtti -fno-exceptions -g

LDFLAGS = -Wl,-melf_i386

GRUB ?= $(HOME)/prog/grub/bin/
MKRESCUE = env PATH=$$PATH:$(GRUB) grub-mkrescue

ISO = thingy.img

TARGETS = kernel $(ISO)

.PHONY: $(TARGETS)

CPP = $(wildcard src/**/*.cpp)
ASM = $(wildcard src/**/*.S)

INCLUDE = $(wildcard include/**/*.h)

all: $(TARGETS)

kernel: $(CPP) $(ASM) $(INCLUDE)
	$(LD) -o $@.out -Iinclude -T linkscript $(CFLAGS) $(LDFLAGS) $(CPP) $(ASM)

%.o: %.S
	$(CC) -o $< -c $(CFLAGS) $@

%.o: %.cpp
	$(CC) -o $< -Iinclude -c $(CFLAGS) $@

clean:
	rm -f *.o
	rm -f kernel.out
	rm -f $(ISO)

$(ISO): kernel
	mkdir -p _boot/boot/grub
	cp kernel.out _boot
	cp grub.cfg _boot/boot/grub/
	cp -r data/ _boot/
	$(MKRESCUE) -o $@ _boot
	#rm -rf _boot

test: $(ISO)
	qemu-system-i386 -serial stdio -cdrom $(ISO)
