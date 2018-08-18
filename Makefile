GRUB ?= $(HOME)/prog/grub/bin/
MKRESCUE = env PATH=$$PATH:$(GRUB) grub-mkrescue

ISO = thingy.img
KERNEL := thingy.bin
$(KERNEL): PLATFORM = kernel


USER := user
$(USER): PLATFORM = user

LIBC = lib/pdclib/kernel_pdclib.a

TARGETS = $(KERNEL) $(ISO) $(LIBC) $(USER)

.PHONY: $(TARGETS) all clean

CPP = $(wildcard src/**/*.cpp)
ASM = $(wildcard src/**/*.S)

OBJ := $(CPP:.cpp=.o) $(ASM:.S=.o)

LD  = gcc
CC  = clang
CXX = clang++

LDFLAGS = -Wl,-melf_i386
INCLUDE = lib/pdclib/includes lib/pdclib/internals lib/pdclib/opt/nothread	\
	      lib/pdclib/platform/$(PLATFORM)/includes							\
	      lib/pdclib/platform/$(PLATFORM)/internals							\
	      include

IFLAGS = $(foreach i, $(INCLUDE), -I$i)

FLAGS += -ffreestanding -nostdlib -static -fno-stack-protector -m32 \
		 -fno-PIC -fno-pie $(IFLAGS) -D_PDCLIB_BUILD -g -mno-sse

CFLAGS += $(FLAGS) -std=c11
CXXFLAGS += $(FLAGS) -std=c++17 -fno-rtti -fno-exceptions

all: $(TARGETS)

$(KERNEL): $(OBJ) $(LIBC)
	$(LD) -o $@ -n -T linkscript $(CXXFLAGS) $(CFLAGS) -O2 -lgcc $(LDFLAGS) $(OBJ) $(LIBC)

$(LIBC):
	$(MAKE) -C lib/pdclib kernel

%.o: %.S
	$(CC) -o $@ -c $(CFLAGS) $<

%.o: %.cpp
	$(CC) -o $@ -Iinclude -c $(CXXFLAGS) $<

clean:
	rm -f src/**/*.o
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

debug: $(ISO)
	qemu-system-i386 -S -s -serial mon:stdio -cdrom $(ISO)

$(USER): program.text.bin program.data.bin

program.text.bin: program.elf
	objcopy -I elf32-i386 -O binary -j .text -S $< $@

program.data.bin: program.elf
	objcopy -I elf32-i386 -O binary -j .data -S $< $@

# compiler-rt libc
program.elf: data/program.o
	$(LD) -o $@ -n -T data/linkscript $(CFLAGS) -O2 -lgcc $(LDFLAGS) $^

data/program.o: data/program.c
	$(CC) -o $@ -c $(CFLAGS) $<
