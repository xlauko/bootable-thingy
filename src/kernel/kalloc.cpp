#include <kernel/kalloc.hpp>

using namespace kernel;

extern "C" uint32_t kernel_end;

namespace kernel {
    uintptr_t placement_addr = reinterpret_cast< uintptr_t >( &kernel_end );

    namespace detail {
        uintptr_t kalloc( size_t size, bool align = false ) {
            placement_addr &= 0xFFFFF000;
            placement_addr += 0x1000;

            auto res = placement_addr;
            placement_addr += size;
            return res;
        }
    }

    uintptr_t kalloc_aligned( size_t size ) {
        return detail::kalloc( size, true );
    }

} // namespace kernel
