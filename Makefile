GRUB ?= $(HOME)/prog/grub/bin/
MKRESCUE = env PATH=$$PATH:$(GRUB) grub-mkrescue

TARGETS = kernel

.PHONY: $(TARGETS)

all: $(TARGETS)

kernel:
	cd ./kernel ; make kernel

boot.img: kernel
	mkdir -p _boot/boot/grub
	cp kernel/kernel.out _boot
	cp kernel/grub.cfg _boot/boot/grub/
	cp -r kernel/* _boot
	$(MKRESCUE) -o $@ _boot
	rm -rf _boot

test: boot.img
	qemu-system-i386 -serial stdio -cdrom boot.img

clean:
	rm -f *.o
	rm boot.img
	cd ./kernel; make clean
