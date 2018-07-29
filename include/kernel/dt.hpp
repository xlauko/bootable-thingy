#pragma once

#include <stdint.h>
#include <stddef.h>

#define PACKED __attribute__((packed))

namespace kernel {

    struct gdt {

        struct item {
            uint16_t limit_low;
            uint16_t base_low;
            uint8_t base_middle;
            uint8_t access;
            uint8_t granularity;
            uint8_t base_high;
        } PACKED;

        uint16_t limit;
        uint32_t base;

        static constexpr size_t size = 5;

        template< size_t idx >
        void set( uint32_t base, uint32_t limit, uint8_t access, uint8_t gran );

        static void init();
    } PACKED;


    struct idt {

        struct item {
            uint16_t base_low;
            uint16_t selector;
            uint8_t zero;
            uint8_t flags;
            uint16_t base_high;
        } PACKED;

        uint16_t limit;
        uint32_t base;

        static constexpr size_t size = 256;

        template< size_t idx >
        void set( uint32_t isr, uint16_t selector, uint8_t flags );

        static void init();
    } PACKED;

    namespace dt {
        void init();
    }

} // namespace kernel
