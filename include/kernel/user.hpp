#pragma once

#include <stdint.h>

namespace kernel {
    namespace user {

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
