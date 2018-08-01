#include <kernel/dt.hpp>

#include <stdio.h>
#include <string.h>

#include <kernel/panic.hpp>
#include <kernel/ioport.hpp>

using namespace kernel;

/* GDT function */

extern "C" void __gdt_flush( void * gdt );

extern "C" {
    gdt::item gdtable[ gdt::size ];
    gdt gdt_ptr;
}

template< size_t idx >
void gdt::set( uint32_t base, uint32_t limit, uint8_t access, uint8_t gran ) {
    auto &item = gdtable[ idx ];

    item.base_low =     (base & 0xFFFF);
    item.base_middle =  (base >> 16) & 0xFF;
    item.base_high =    (base >> 24) & 0xFF;
    item.limit_low =    (limit & 0xFFFF);
    item.granularity =  (limit >> 16) & 0X0F;
    item.granularity |= (gran & 0xF0);
    item.access = access;
}

void gdt::init() {
    gdt_ptr.limit = ( sizeof( gdt::item ) * gdt::size ) - 1;
    gdt_ptr.base = reinterpret_cast< uint32_t >( &gdtable );

    gdt_ptr.set< 0 >( 0, 0, 0, 0 );                 // null segment
    gdt_ptr.set< 1 >( 0, 0xFFFFFFFF, 0x9A, 0xCF );  // code segment
    gdt_ptr.set< 2 >( 0, 0xFFFFFFFF, 0x92, 0xCF );  // data segment
    gdt_ptr.set< 3 >( 0, 0xFFFFFFFF, 0xFA, 0xCF );  // user segment
    gdt_ptr.set< 4 >( 0, 0xFFFFFFFF, 0xF2, 0xCF );  // user data
     __gdt_flush( &gdt_ptr );
}

/* IDT functions */

extern "C" {
    idt::item idtable[ idt::size ];
    idt idt_ptr;
}

extern "C" void __idt_flush();

extern "C" {
    void isr0( unsigned int );
    void isr1( unsigned int );
    void isr2( unsigned int );
    void isr3( unsigned int );
    void isr4( unsigned int );
    void isr5( unsigned int );
    void isr6( unsigned int );
    void isr7( unsigned int );
    void isr8( unsigned int );
    void isr9( unsigned int );
    void isr10( unsigned int );
    void isr11( unsigned int );
    void isr12( unsigned int );
    void isr13( unsigned int );
    void isr14( unsigned int );
    void isr15( unsigned int );
    void isr16( unsigned int );
    void isr17( unsigned int );
    void isr18( unsigned int );
    void isr19( unsigned int );
    void isr20( unsigned int );
    void isr21( unsigned int );
    void isr22( unsigned int );
    void isr23( unsigned int );
    void isr24( unsigned int );
    void isr25( unsigned int );
    void isr26( unsigned int );
    void isr27( unsigned int );
    void isr28( unsigned int );
    void isr29( unsigned int );
    void isr30( unsigned int );
    void isr31( unsigned int );
}

extern "C" {
    void irq0( unsigned int );
    void irq1( unsigned int );
    void irq2( unsigned int );
    void irq3( unsigned int );
    void irq4( unsigned int );
    void irq5( unsigned int );
    void irq6( unsigned int );
    void irq7( unsigned int );
    void irq8( unsigned int );
    void irq9( unsigned int );
    void irq10( unsigned int );
    void irq11( unsigned int );
    void irq12( unsigned int );
    void irq13( unsigned int );
    void irq14( unsigned int );
    void irq15( unsigned int );
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

    idt_ptr.set< 0 >( isr0 );
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

extern "C" void isr_default_handler( unsigned int_no ) {
    if ( int_no == 8 ) {
        fprintf( stderr, "%s\n", exception_messages[ int_no ] );
		panic();
    }

    if ( int_no >= 32 ) {
    	panic();
	}

    if ( auto handler = isrs_handlers[ int_no ] ) {
        handler( int_no );
    } else {
        fprintf( stderr, "Unhandled exception: [%d] %s\n", int_no, exception_messages[ int_no ] );
    	panic();
	}
}

extern "C" void irq_default_handler( unsigned int_no ) {
    if ( int_no < 32 || int_no > 47 ) {
    	panic();
    }

    // Send an EOI (end of interrupt) signal to the PICs.
    // If this interrupt involved the slave.
    if ( int_no >= 40 ) {
        // Send reset signal to slave.
        dev::outb( 0xA0, 0x20 );
    }
    // Send reset signal to master. (As well as slave, if necessary).
    dev::outb( 0x20, 0x20 );

    if ( auto handler = irq_handlers[ int_no - 32 ] ) {
        handler( int_no );
    }
}
namespace kernel::dt {

    void init() {
        gdt::init();
        idt::init();
    }

} // namespace dt;
