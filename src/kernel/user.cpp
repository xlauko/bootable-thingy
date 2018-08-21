#include <kernel/user.hpp>
#include <kernel/mem.hpp>
#include <kernel/dt.hpp>

#include <stdio.h>

extern "C" int __jump_to_userland( void *code, void * stack );

namespace kernel {
    namespace user {

        uint32_t stack[ stack_size ];

        int executable::start() const {
            puts( "\nStarting user program.\n" );

            // map pages
            auto flags = mem::page_allocator::user_flags;

            auto code_addr = mem::palloc.find_space( text.size, true );
            for ( size_t i = 0; i < text.size; ++i ) {
                auto addr = text.addr + i * mem::paging::page::size;
                mem::palloc.map( addr, code_addr, flags );
            }

            auto data_addr = mem::palloc.find_space( data.size, true );
            for ( size_t i = 0; i < data.size; ++i ) {
                auto addr = data.addr + i * mem::paging::page::size;
                mem::palloc.map( addr, data.addr, flags );
            }

            return __jump_to_userland( ( void * )mem::virt_2_phys( code_addr ), stack );
        }

    } // namespace user
} // namespace kernel
