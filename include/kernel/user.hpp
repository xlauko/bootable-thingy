#pragma once

#include <stdint.h>
#include <stddef.h>

namespace kernel {
    namespace user {

        static constexpr size_t stack_size = 0x4000;

        struct executable {
            struct section {
                uint32_t addr;
                uint32_t size;
            };

            section data;
            section text;

            int start() const;
        };

    } // namespace user
} // namespace kernel
