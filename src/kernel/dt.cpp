#include <kernel/dt.hpp>

#include <stdio.h>
#include <string.h>

#include <kernel/panic.hpp>
#include <kernel/ioport.hpp>
#include <kernel/mem.hpp>

using namespace kernel;

/* GDT function */

extern "C" int __gdt_flush( size_t size, void * gdt );
extern "C" void __tss_flush();

static size_t gdt_size = 6 * 2;
uint32_t gdt[ 12 ];

uint32_t tss[ 26 ];

void gdt_init() {

    gdt_size *= 4;
    --gdt_size;

    // Null descriptor
    gdt[ 0 ] = 0x0000'0000;
    gdt[ 1 ] = 0x0000'0000;

    // Kernel code descriptor
    gdt[ 2 ] = 0x0000'ffff;
    gdt[ 3 ] = 0x00cf'9a00;

    // Kernel data descriptor
    gdt[ 4 ] = 0x0000'ffff;
    gdt[ 5 ] = 0x00cf'9200;

    // User code descriptor
    gdt[ 6 ] = 0x0000'ffff;
    gdt[ 7 ] = 0x00cf'fa00;

    // User data descriptor
    gdt[ 8 ] = 0x0000'ffff;
    gdt[ 9 ] = 0x00cf'f200;

    memset( tss, 0, sizeof( uint32_t ) * 26 );

    tss[ 1 ] = 0x0;// ESP0
    tss[ 2 ] = 0x10; // SS0
    tss[ 19 ] = 0x0b;
    tss[ 18 ] = tss[ 20 ] = tss[ 21 ] = tss[ 22 ] = tss[ 23 ] = 0x13;
    tss[ 25 ] = 0x0068'0000; // IOPB

    // TSS descriptor
    auto tssi = reinterpret_cast< uint32_t >( tss );
    gdt[ 10 ] = 0x0000'0068;
    gdt[ 11 ] = 0x0000'e900;
    gdt[ 10 ] |= tssi << 16;
    gdt[ 11 ] |= ( tssi >> 16 ) & 0xFF;
    gdt[ 11 ] |= tssi & 0xFF00'0000;

    __gdt_flush( gdt_size, gdt );
    __tss_flush();
}

/* IDT functions */

extern "C" {
    idt::item idtable[ idt::size ];
    idt idt_ptr;
}

extern "C" void __idt_flush();

extern "C" {
    void isr0( registers_t * );
    void isr1( registers_t * );
    void isr2( registers_t * );
    void isr3( registers_t * );
    void isr4( registers_t * );
    void isr5( registers_t * );
    void isr6( registers_t * );
    void isr7( registers_t * );
    void isr8( registers_t * );
    void isr9( registers_t * );
    void isr10( registers_t * );
    void isr11( registers_t * );
    void isr12( registers_t * );
    void isr13( registers_t * );
    void isr14( registers_t * );
    void isr15( registers_t * );
    void isr16( registers_t * );
    void isr17( registers_t * );
    void isr18( registers_t * );
    void isr19( registers_t * );
    void isr20( registers_t * );
    void isr21( registers_t * );
    void isr22( registers_t * );
    void isr23( registers_t * );
    void isr24( registers_t * );
    void isr25( registers_t * );
    void isr26( registers_t * );
    void isr27( registers_t * );
    void isr28( registers_t * );
    void isr29( registers_t * );
    void isr30( registers_t * );
    void isr31( registers_t * );
}

extern "C" {
    void irq0( registers_t * );
    void irq1( registers_t * );
    void irq2( registers_t * );
    void irq3( registers_t * );
    void irq4( registers_t * );
    void irq5( registers_t * );
    void irq6( registers_t * );
    void irq7( registers_t * );
    void irq8( registers_t * );
    void irq9( registers_t * );
    void irq10( registers_t * );
    void irq11( registers_t * );
    void irq12( registers_t * );
    void irq13( registers_t * );
    void irq14( registers_t * );
    void irq15( registers_t * );
}

template< size_t idx >
void idt::set( irq::handler handler, uint16_t selector, uint8_t flags ) {
    auto base = reinterpret_cast< uint32_t >( handler );
    auto &item = idtable[ idx ];
    item.base_low  = (base & 0xFFFF);
    item.base_high = (base >> 16) & 0xFFFF;
    item.selector  = selector;
    item.zero      = 0;
    // We must uncomment the OR below when we get to using user-mode.
    // It sets the interrupt gate's privilege level to 3.
    item.flags     = flags /* | 0x60 */;

}

void idt::init() {
    idt_ptr.limit = ( sizeof( idt::item ) * idt::size ) - 1;
    idt_ptr.base = reinterpret_cast< uint32_t >( idtable );

    memset( &idtable, 0, sizeof( idt::item ) * idt::size );

    __idt_flush();
}

const char *exception_messages[] = {
    "Division by zero",
    "Debug",
    "Non-maskable interrupt",
    "Breakpoint",
    "Detected overflow",
    "Out-of-bounds",
    "Invalid opcode",
    "No coprocessor",
    "Double fault",
    "Coprocessor segment overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Unknown interrupt",
    "Coprocessor fault",
    "Alignment check",
    "Machine check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

extern "C" {
	irq::handler isrs_handlers[ isrs::num_of_handlers ] = { nullptr };

    irq::handler irq_handlers[ irq::num_of_handlers ] = { nullptr };
}

namespace kernel::isrs {
	void install_handler( unsigned isrs, irq::handler handler ) {
		isrs_handlers[ isrs ] = handler;
	}

	void uninstall_handler( unsigned isrs ) {
		isrs_handlers[ isrs ] = nullptr;
	}

	void init() {
        idt_ptr.set< 1 >( isr1 );
        idt_ptr.set< 2 >( isr2 );
        idt_ptr.set< 3 >( isr3 );
        idt_ptr.set< 4 >( isr4 );
        idt_ptr.set< 5 >( isr5 );
        idt_ptr.set< 6 >( isr6 );
        idt_ptr.set< 7 >( isr7 );
        idt_ptr.set< 8 >( isr8 );
        idt_ptr.set< 9 >( isr9 );
        idt_ptr.set< 10 >( isr10 );
        idt_ptr.set< 11 >( isr11 );
        idt_ptr.set< 12 >( isr12 );
        idt_ptr.set< 13 >( isr13 );
        idt_ptr.set< 14 >( isr14 );
        idt_ptr.set< 15 >( isr15 );
        idt_ptr.set< 16 >( isr16 );
        idt_ptr.set< 17 >( isr17 );
        idt_ptr.set< 18 >( isr18 );
        idt_ptr.set< 19 >( isr19 );
        idt_ptr.set< 20 >( isr20 );
        idt_ptr.set< 21 >( isr21 );
        idt_ptr.set< 22 >( isr22 );
        idt_ptr.set< 23 >( isr23 );
        idt_ptr.set< 24 >( isr24 );
        idt_ptr.set< 25 >( isr25 );
        idt_ptr.set< 26 >( isr26 );
        idt_ptr.set< 27 >( isr27 );
        idt_ptr.set< 28 >( isr28 );
        idt_ptr.set< 29 >( isr29 );
        idt_ptr.set< 30 >( isr30 );
        idt_ptr.set< 31 >( isr31 );
    }
}

namespace kernel::irq {
	void install_handler( unsigned irq, irq::handler handler ) {
		irq_handlers[ irq ] = handler;
	}

	void uninstall_handler( unsigned irq ) {
		irq_handlers[ irq ] = nullptr;
	}

	void remap() {
        dev::outb( 0x20, 0x11 );
        dev::outb( 0xA0, 0x11 );
        dev::outb( 0x21, 0x20 );
        dev::outb( 0xA1, 0x28 );
        dev::outb( 0x21, 0x04 );
        dev::outb( 0xA1, 0x02 );
        dev::outb( 0x21, 0x01 );
        dev::outb( 0xA1, 0x01 );
        dev::outb( 0x21, 0x0 );
        dev::outb( 0xA1, 0x0 );
    };

    void init() {
        irq::remap();

        idt_ptr.set< 32 >( irq0 );
        idt_ptr.set< 33 >( irq1 );
        idt_ptr.set< 34 >( irq2 );
        idt_ptr.set< 35 >( irq3 );
        idt_ptr.set< 36 >( irq4 );
        idt_ptr.set< 37 >( irq5 );
        idt_ptr.set< 38 >( irq6 );
        idt_ptr.set< 39 >( irq7 );
        idt_ptr.set< 40 >( irq8 );
        idt_ptr.set< 41 >( irq9 );
        idt_ptr.set< 42 >( irq10 );
        idt_ptr.set< 43 >( irq11 );
        idt_ptr.set< 44 >( irq12 );
        idt_ptr.set< 45 >( irq13 );
        idt_ptr.set< 46 >( irq14 );
        idt_ptr.set< 47 >( irq15 );
    }

}

extern "C" void isr_default_handler( registers_t * regs ) {
    if ( regs->int_no == 8 ) {
        fprintf( stderr, "%s\n", exception_messages[ regs->int_no ] );
		panic();
    }

    if ( regs->int_no >= 32 ) {
    	panic();
	}

    if ( auto handler = isrs_handlers[ regs->int_no ] ) {
        handler( regs );
    } else {
        fprintf( stderr, "Unhandled exception: [%d] %s\n", regs->int_no, exception_messages[ regs->int_no ] );
    	panic();
	}
}

extern "C" void irq_default_handler( registers_t *regs ) {
    if ( regs->int_no < 32 || regs->int_no > 47 ) {
    	panic();
    }

    // Send an EOI (end of interrupt) signal to the PICs.
    // If this interrupt involved the slave.
    if ( regs->int_no >= 40 ) {
        // Send reset signal to slave.
        dev::outb( 0xA0, 0x20 );
    }
    // Send reset signal to master. (As well as slave, if necessary).
    dev::outb( 0x20, 0x20 );

    if ( auto handler = irq_handlers[ regs->int_no - 32 ] ) {
        handler( regs );
    }
}
namespace kernel::dt {

    void init() {
        gdt_init();
        idt::init();
        isrs::init();
        irq::init();
    }

} // namespace dt;
