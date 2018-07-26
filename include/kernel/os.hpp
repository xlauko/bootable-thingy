#pragma once

#include <kernel/dev.hpp>

namespace kernel {

    class Thingy {
    public:
        static void start( unsigned long magic, unsigned long addr ) noexcept;
        static void init_devices( dev::Serial *ser, dev::VGA *v ) noexcept;
        static void halt() noexcept;
    private:
        Thingy( Thingy& ) = delete;
        Thingy( Thingy&& ) = delete;
        Thingy& operator=( Thingy& ) = delete;
        Thingy& operator=( Thingy&& ) = delete;
        ~Thingy() = delete;
        Thingy() = delete;
    };

    namespace dev {
        static Serial * serial;
        static VGA * vga;
    } // namespace dev

} // namespace kernel
