#pragma once

#include <boot/multiboot.h>

#include <cstdint>

namespace kernel {

    class Thingy {
    public:
        static void start( uint32_t boot_magic, uint32_t boot_addr );
    private:
        Thingy( Thingy& ) = delete;
        Thingy( Thingy&& ) = delete;
        Thingy& operator=( Thingy& ) = delete;
        Thingy& operator=( Thingy&& ) = delete;
        ~Thingy() = delete;
        Thingy() = delete;
    };

} // namespace kernel
