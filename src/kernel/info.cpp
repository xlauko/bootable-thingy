#include <kernel/info.hpp>
#include <kernel/panic.hpp>

namespace multiboot {

Status check( unsigned long magic, unsigned long addr ) {
    if ( magic != MULTIBOOT2_BOOTLOADER_MAGIC ) {
        fprintf( stderr, "invalid magic number" );
        return Status::failure;
    }

    if ( addr & 7 ) {
        fprintf( stderr, "unaligned mbi" );
        return Status::failure;
    }

    return Status::success;
}

information_item * info::begin() const {
    auto const first = reinterpret_cast< unsigned long >( list + 8 );
    return reinterpret_cast< information_item * >( first );
}

information_item * next( information_item * item ) {
    return reinterpret_cast< information_item * >( ( ( reinterpret_cast< unsigned long >( item ) )
                                                       + item->size + 7 ) & 0xfffffff8UL );
}

memory_info info::mem() const {
    memory_info mem_info;

    basic_memory_information * bmi = nullptr;

    yield( information_type::basic_memory, [&] ( const auto & item ) {
        bmi = reinterpret_cast< basic_memory_information * >( item );
    } );

    if ( bmi == nullptr ) {
        fprintf( stderr, "No memory information in multiboot tags." );
        panic();
    }

    mem_info.lower = bmi->lower;
    mem_info.upper = bmi->upper;

    return mem_info;
}

} // namespace multiboot
