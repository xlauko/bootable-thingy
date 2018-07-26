GRUB ?= $(HOME)/prog/grub/bin/
MKRESCUE = env PATH=$$PATH:$(GRUB) grub-mkrescue

ISO = thingy.img
KERNEL := thingy.bin
$(KERNEL): PLATFORM = kernel

LIBC = lib/pdclib/kernel_pdclib.a

TARGETS = $(KERNEL) $(ISO) $(LIBC)

.PHONY: $(TARGETS)

CPP = $(wildcard src/**/*.cpp)
ASM = $(wildcard src/**/*.S)

LD = gcc
CC = clang

LDFLAGS = -Wl,-melf_i386
INCLUDE = lib/pdclib/includes lib/pdclib/internals lib/pdclib/opt/nothread	\
	    lib/pdclib/platform/$(PLATFORM)/includes							\
	    lib/pdclib/platform/$(PLATFORM)/internals							\
	    include

FLAGS = $(foreach i, $(INCLUDE), -I$i)

CFLAGS += -std=c++17 -ffreestanding -nostdlib -static -fno-stack-protector -m32 \
		  -fno-PIC -fno-rtti -fno-exceptions $(FLAGS) -D_PDCLIB_BUILD -g


all: $(TARGETS)

$(KERNEL): $(CPP) $(ASM) $(LIBC)
	$(LD) -o $@ -T linkscript $(CFLAGS) $(LDFLAGS) $(CPP) $(ASM) $(LIBC)

$(LIBC):
	$(MAKE) -C lib/pdclib kernel

%.o: %.S
	$(CC) -o $< -c $(CFLAGS) $@

%.o: %.cpp
	$(CC) -o $< -Iinclude -c $(CFLAGS) $@

clean:
	rm -f *.o
	rm -f $(KERNEL)
	rm -f $(ISO)
	$(MAKE) -C lib/pdclib clean

$(ISO): $(KERNEL)
	mkdir -p _boot/boot/grub
	cp $(KERNEL) _boot
	cp grub.cfg _boot/boot/grub/
	cp -r data/ _boot/
	$(MKRESCUE) -o $@ _boot
	rm -rf _boot

test: $(ISO)
	qemu-system-i386 -serial stdio -cdrom $(ISO)
