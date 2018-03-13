#include "mm.h"
#include "libutils.h"

#include <algorithm>

const size_t page_dir_size = 1024;
const size_t page_table_size = 1024;

uint32_t page_dir[ page_dir_size ] __attribute__((aligned(4096)));

uint32_t first_page_table[ page_table_size ] __attribute__((aligned(4096)));

extern "C" {
    void load_page_dir( uint32_t* );
    void enable_paging();
}

void init_page_dir() {
    std::fill_n( page_dir, page_dir_size, 0x00000002 );

    for ( size_t i = 0; i < page_table_size; ++i )
        first_page_table[ i ] = (i * 0x1000) | 3;

    page_dir[ 0 ] = reinterpret_cast< uint32_t >( first_page_table ) | 3;

    load_page_dir( page_dir );
    enable_paging();

    puts( "paging enabled" );
}
