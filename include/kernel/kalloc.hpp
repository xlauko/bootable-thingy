#pragma once

#include <stdint.h>
#include <stddef.h>

namespace kernel {

    extern uintptr_t placement_addr;

    uintptr_t kalloc( size_t size );
    uintptr_t kalloc_aligned( size_t size );
}
