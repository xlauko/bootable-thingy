#include <kernel/dt.hpp>

using namespace kernel;

extern "C" void __gdt_flush( void * gdt );

extern "C" {
    gdt_entry table[ gdt::size ];
    gdt gdt_ptr;
}

template< size_t idx >
void gdt::set( uint32_t base, uint32_t limit, uint8_t access, uint8_t gran ) {
    auto &item = table[ idx ];

    item.base_low =     (base & 0xFFFF);
    item.base_middle =  (base >> 16) & 0xFF;
    item.base_high =    (base >> 24) & 0xFF;
    item.limit_low =    (limit & 0xFFFF);
    item.granularity =  (limit >> 16) & 0X0F;
    item.granularity |= (gran & 0xF0);
    item.access = access;
}

void gdt::init() {
    gdt_ptr.limit = ( sizeof(gdt_entry) * gdt::size ) - 1;
    gdt_ptr.base = reinterpret_cast< uint32_t >( &table );

    gdt_ptr.set< 0 >( 0, 0, 0, 0 );                 // null segment
    gdt_ptr.set< 1 >( 0, 0xFFFFFFFF, 0x9A, 0xCF );  // code segment
    gdt_ptr.set< 2 >( 0, 0xFFFFFFFF, 0x92, 0xCF );  // data segment
    gdt_ptr.set< 3 >( 0, 0xFFFFFFFF, 0xFA, 0xCF );  // user segment
    gdt_ptr.set< 4 >( 0, 0xFFFFFFFF, 0xF2, 0xCF );  // user data
     __gdt_flush( &gdt_ptr );
}

namespace kernel::dt {

    void init() {
        gdt::init();
    }

} // namespace dt;
