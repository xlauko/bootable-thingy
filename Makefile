GRUB ?= $(HOME)/prog/grub/bin/
MKRESCUE = env PATH=$$PATH:$(GRUB) grub-mkrescue

TARGETS = kernel boot.img

.PHONY: $(TARGETS)

all: $(TARGETS)

kernel:
	$(MAKE) -C kernel

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
	rm -f boot.img
	$(MAKE) -C kernel clean
