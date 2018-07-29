#pragma once

#include <stdint.h>
#include <stddef.h>

namespace kernel {

    struct gdt_entry {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_middle;
        uint8_t access;
        uint8_t granularity;
        uint8_t base_high;
    } __attribute__((packed));

    struct gdt {
        uint16_t limit;
        uint32_t base;

        static constexpr size_t size = 5;

        template< size_t idx >
        void set( uint32_t base, uint32_t limit, uint8_t access, uint8_t gran );

        static void init();
    } __attribute__((packed));


    namespace dt {
        void init();
    }

} // namespace kernel
