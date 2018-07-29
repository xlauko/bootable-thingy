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
        .asciz  "halted"
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
