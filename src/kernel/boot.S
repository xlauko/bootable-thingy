#define ASM_FILE        1
#define STACK_SIZE      0x4000
#include "multiboot2.h"

.text

.global  start, _start

start:
_start:
        jmp     multiboot_entry

        .align  8 /* required multiboot header alignment */
multiboot_header:
        .long   MULTIBOOT2_HEADER_MAGIC
        .long   MULTIBOOT_ARCHITECTURE_I386
        .long   multiboot_header_end - multiboot_header /* length */
        .long   -( MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 \
                   + ( multiboot_header_end - multiboot_header ) )

        .align 8 /* each tag must be aligned separately */
        .short MULTIBOOT_HEADER_TAG_END /* tag id */
        .short 0                        /* flags */
        .long 8                         /* tag size */
multiboot_header_end:

multiboot_entry:
        movl    $(stack + STACK_SIZE), %esp /* stack pointer */
        pushl   $0
        popf                                /* reset EFLAGS */

        pushl   %ebx /* multiboot information structure */
        pushl   %eax /* multiboot magic value */

        cli          /* disable interrupts */
        call    main /* let's C */

        pushl   $halt_message
        call    puts
        hlt

halt_message:
        .asciz  "thingy halted"
        .comm   stack, STACK_SIZE /* the stack */

.global __gdt_flush, __flush
.extern gdt_ptr

__gdt_flush:
    lgdt [gdt_ptr]
    mov %ax, 0x10
    mov %ds, %ax
    mov %es, %ax
    mov %fs, %ax
    mov %gs, %ax
    mov %ss, %ax
    jmp  $0x08, $(__flush)
__flush:
    ret

.global  __idt_flush
.extern idt_ptr

__idt_flush:
    lidt [idt_ptr]
    ret

.macro ISR_NOERR num:req
    .global isr\num
    isr\num:
		cli
		pusha
		mov $\num, %eax
		push %eax
		jmp __isr_default_handler_wrapper
.endm

.macro ISR_ERR num:req
    .global isr\num
    isr\num:
        cli
		pusha
		mov $\num, %eax
		push %eax
        jmp __isr_default_handler_wrapper
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

.extern isr_default_handler
.global __isr_default_handler_wrapper
__isr_default_handler_wrapper:
	call isr_default_handler
	pop %eax

	popa
	sti
	iret

