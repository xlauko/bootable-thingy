#include <kernel/dt.hpp>

#include <stdio.h>
#include <string.h>

#include <kernel/panic.hpp>

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
    void isr0();
    void isr1();
    void isr2();
    void isr3();
    void isr4();
    void isr5();
    void isr6();
    void isr7();
    void isr8();
    void isr9();
    void isr10();
    void isr11();
    void isr12();
    void isr13();
    void isr14();
    void isr15();
    void isr16();
    void isr17();
    void isr18();
    void isr19();
    void isr20();
    void isr21();
    void isr22();
    void isr23();
    void isr24();
    void isr25();
    void isr26();
    void isr27();
    void isr28();
    void isr29();
    void isr30();
    void isr31();
}

template< size_t idx >
void idt::set( uint32_t base, uint16_t selector, uint8_t flags ) {
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

    idt_ptr.set< 0 >( reinterpret_cast< uint32_t >( isr0 ) );
    idt_ptr.set< 1 >( reinterpret_cast< uint32_t >( isr1 ) );
    idt_ptr.set< 2 >( reinterpret_cast< uint32_t >( isr2 ) );
    idt_ptr.set< 3 >( reinterpret_cast< uint32_t >( isr3 ) );
    idt_ptr.set< 4 >( reinterpret_cast< uint32_t >( isr4 ) );
    idt_ptr.set< 5 >( reinterpret_cast< uint32_t >( isr5 ) );
    idt_ptr.set< 6 >( reinterpret_cast< uint32_t >( isr6 ) );
    idt_ptr.set< 7 >( reinterpret_cast< uint32_t >( isr7 ) );
    idt_ptr.set< 8 >( reinterpret_cast< uint32_t >( isr8 ) );
    idt_ptr.set< 9 >( reinterpret_cast< uint32_t >( isr9 ) );
    idt_ptr.set< 10 >( reinterpret_cast< uint32_t >( isr10 ) );
    idt_ptr.set< 11 >( reinterpret_cast< uint32_t >( isr11 ) );
    idt_ptr.set< 12 >( reinterpret_cast< uint32_t >( isr12 ) );
    idt_ptr.set< 13 >( reinterpret_cast< uint32_t >( isr13 ) );
    idt_ptr.set< 14 >( reinterpret_cast< uint32_t >( isr14 ) );
    idt_ptr.set< 15 >( reinterpret_cast< uint32_t >( isr15 ) );
    idt_ptr.set< 16 >( reinterpret_cast< uint32_t >( isr16 ) );
    idt_ptr.set< 17 >( reinterpret_cast< uint32_t >( isr17 ) );
    idt_ptr.set< 18 >( reinterpret_cast< uint32_t >( isr18 ) );
    idt_ptr.set< 19 >( reinterpret_cast< uint32_t >( isr19 ) );
    idt_ptr.set< 20 >( reinterpret_cast< uint32_t >( isr20 ) );
    idt_ptr.set< 21 >( reinterpret_cast< uint32_t >( isr21 ) );
    idt_ptr.set< 22 >( reinterpret_cast< uint32_t >( isr22 ) );
    idt_ptr.set< 23 >( reinterpret_cast< uint32_t >( isr23 ) );
    idt_ptr.set< 24 >( reinterpret_cast< uint32_t >( isr24 ) );
    idt_ptr.set< 25 >( reinterpret_cast< uint32_t >( isr25 ) );
    idt_ptr.set< 26 >( reinterpret_cast< uint32_t >( isr26 ) );
    idt_ptr.set< 27 >( reinterpret_cast< uint32_t >( isr27 ) );
    idt_ptr.set< 28 >( reinterpret_cast< uint32_t >( isr28 ) );
    idt_ptr.set< 29 >( reinterpret_cast< uint32_t >( isr29 ) );
    idt_ptr.set< 30 >( reinterpret_cast< uint32_t >( isr30 ) );
    idt_ptr.set< 31 >( reinterpret_cast< uint32_t >( isr31 ) );

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
}

namespace kernel::isrs {
	void install_handler( unsigned isrs, irq::handler handler ) {
		isrs_handlers[ isrs ] = handler;
	}

	void uninstall_handler( unsigned isrs ) {
		isrs_handlers[ isrs ] = nullptr;
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

namespace kernel::dt {

    void init() {
        gdt::init();
        idt::init();
    }

} // namespace dt;
