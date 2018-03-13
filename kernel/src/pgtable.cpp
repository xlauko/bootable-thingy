#include "pgtable.h"
#include "libutils.h"

#include <algorithm>

const size_t page_dir_size = 1024;
const size_t page_table_size = 1024;
const size_t page_size = 4096;


uint32_t page_dir[ page_dir_size ] __attribute__((aligned( page_size )));

uint32_t first_page_table[ page_table_size ] __attribute__((aligned( page_size )));

extern "C" {
    void load_page_dir( uint32_t* );
    void enable_paging();

    extern void kernel_physical_start( void );
    extern void kernel_physical_end( void );
    extern void kernel_virtual_start( void );
    extern void kernel_virtual_end( void );
}



void id_paging( uintptr_t first_pt, uintptr_t from, int size ) {
    from = from & 0xfffff000;
    while ( size > 0 ) {
        first_pt = from | 3;
        first_pt++;

        from += page_size;
        size -= page_size;
    }
}

void init_page_dir() {
    std::fill_n( page_dir, page_dir_size, 0x00000002 );

    for ( size_t i = 0; i < page_table_size; ++i )
        first_page_table[ i ] = (i * 0x1000) | 3;

    page_dir[ 0 ] = reinterpret_cast< uint32_t >( first_page_table ) | 3;

    load_page_dir( page_dir );
    enable_paging();

    puts( "paging enabled" );

    uintptr_t vaddr = reinterpret_cast< uintptr_t >( &kernel_virtual_start );
    id_paging( reinterpret_cast<uintptr_t>( first_page_table ),
               vaddr, page_table_size * page_size );

    puts( "identity map kernel" );
}
