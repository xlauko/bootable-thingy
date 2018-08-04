#pragma once

#include <stdint.h>
#include <stddef.h>

#define PACKED __attribute__((packed))



namespace kernel {
    typedef struct registers{
        uint32_t ds;
        uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
        uint32_t int_no, err_code;
        uint32_t eip, cs, eflags, useresp, ss;
    } registers_t;

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


	namespace irq {
		static constexpr size_t num_of_handlers = 16;

		using handler = void (*) ( registers_t * );

		void install_handler( unsigned irq, irq::handler handler );
		void uninstall_handler( unsigned irq );

		void remap();

		void inti();
	}

	namespace isrs {
		static constexpr size_t num_of_handlers = 32;

		void install_handler( unsigned isrs, irq::handler handler );
		void uninstall_handler( unsigned isrs );
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
