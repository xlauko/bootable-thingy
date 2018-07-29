#include <kernel/dt.hpp>

#include <string.h>

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

    __idt_flush();
}

namespace kernel::dt {

    void init() {
        gdt::init();
        idt::init();
    }

} // namespace dt;
