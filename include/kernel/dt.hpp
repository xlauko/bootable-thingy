#pragma once

#include <stdint.h>
#include <stddef.h>

#include <kernel/ioport.hpp>

#define PACKED __attribute__((packed))

namespace kernel {

    typedef struct registers{
        uint32_t ds;
        uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
        uint32_t int_no, err_code;
        uint32_t eip, cs, eflags, useresp, ss;
    } registers_t;

    struct gdt_entry_t {
       uint16_t limit_low;           // The lower 16 bits of the limit.
       uint16_t base_low;            // The lower 16 bits of the base.
       uint8_t  base_middle;         // The next 8 bits of the base.
       uint8_t  access;              // Access flags, determine what ring this segment can be used in.
       uint8_t  granularity;
       uint8_t  base_high;           // The last 8 bits of the base.
    } __attribute__((packed));

    struct gdt_ptr_t {
        uint16_t limit;               // The upper 16 bits of all selector limits.
        uint32_t base;                // The address of the first gdt_entry_t struct.
    } __attribute__((packed));

	namespace irq {
		static constexpr size_t num_of_handlers = 16;

		using handler = void (*) ( registers_t * );

		void install_handler( unsigned irq, irq::handler handler );
		void uninstall_handler( unsigned irq );

		void remap();

		void inti();

		namespace pic {
            static constexpr uint8_t PORT_DATA[ 2 ] = { 0x21, 0xA1 };

            static void enable( uint8_t id ) {
                bool pic = id >= 8;
                uint8_t mask = dev::inb( PORT_DATA[ pic ] );
                dev::outb( PORT_DATA[ pic ], mask & ~( 1 << ( id % 8 ) ) );
            }

            static void disable( uint8_t id ) {
                bool pic = id >= 8;
                uint8_t mask = dev::inb( PORT_DATA[ pic ] );
                dev::outb( PORT_DATA[ pic ], mask | ( 1 << ( id % 8 ) ) );
            }
        }
	}

	namespace isrs {
		static constexpr size_t num_of_handlers = 32;

		void install_handler( unsigned isrs, irq::handler handler );
		void uninstall_handler( unsigned isrs );

		void inti();
	}

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
        void set( irq::handler handler, uint16_t selector = 0x08, uint8_t flags = 0x8E );

        static void init();
    } PACKED;

    namespace dt {
        void init();
    }

} // namespace kernel
