#pragma once

#include <boot/multiboot.h>

#include <cstdint>

namespace kernel {

    class Thingy {
    public:
        static constexpr int PAGE_SHIFT = 12;

        static int start( uint32_t boot_magic, uint32_t boot_addr );

        static constexpr uint32_t page_size() noexcept { return 4096; }

        static constexpr uint32_t addr_to_page(uintptr_t addr) noexcept {
            return addr >> PAGE_SHIFT;
        }

        static constexpr uintptr_t page_to_addr(uint32_t page) noexcept {
            return page << PAGE_SHIFT;
        }

    private:
        Thingy( Thingy& ) = delete;
        Thingy( Thingy&& ) = delete;
        Thingy& operator=( Thingy& ) = delete;
        Thingy& operator=( Thingy&& ) = delete;
        ~Thingy() = delete;
        Thingy() = delete;
    };

} // namespace kernel
