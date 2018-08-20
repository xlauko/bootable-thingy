#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include <kernel/info.hpp>

#define ALIGNED __attribute__((aligned(4096)))

namespace kernel::mem {

    namespace phys {
	    using address_t = uint32_t;
    }

    namespace virt {
        using address_t = uint32_t;
    }

    namespace paging {

        struct page_entry {
            union alignas( 4 ) {
                uint32_t raw;
                struct {
					uint32_t present    : 1;   // Page present in memory
					uint32_t rw         : 1;   // Read-only if clear, readwrite if set
					uint32_t user       : 1;   // Supervisor level only if clear
					uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
					uint32_t dirty      : 1;   // Has the page been written to since last refresh?
					uint32_t unused     : 7;   // Amalgamation of unused and reserved bits
					uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
				};
            };
        };

		struct page_table {
			static constexpr size_t size = 1024;
			page_entry pages[ size ];

			static page_table * create();
			static page_table * empty();
		};


		struct page_directory {
			static constexpr size_t size = 1024;
			page_table * tables[ size ];

			static page_directory * create();
		};

		struct page {
            static constexpr size_t size = 4096;

            static constexpr size_t index( virt::address_t addr ) {
                return addr / size;
            }

            virt::address_t addr;
        };

    } // namespace paging

    struct frame_allocator {
        struct frame {
            phys::address_t addr;
            size_t size;
        };

        frame alloc();
        frame alloc( size_t num );

        void skip_allocated_frames();

        void free( frame );

        static void init( const multiboot::info & info );

        size_t last = 0;
    };

    extern frame_allocator falloc;

    struct page_allocator {

        paging::page alloc( size_t num, bool user = false );
        void free( paging::page page );

        virt::address_t skip_used_pages( virt::address_t addr );
        size_t unused_space_from_addr( virt::address_t virt, bool user, size_t bound );
        virt::address_t find_space( size_t num, bool user );

        void map( virt::address_t virt, phys::address_t phys, uint32_t flags );

        static void init( frame_allocator * allocator );

        static constexpr uint32_t kernel_flags = 0x103;
        static constexpr uint32_t user_flags = 0x07;

        frame_allocator * allocator;
    };

    extern page_allocator palloc;

    void * kmalloc_page_aligned( size_t size );

    struct kernel_allocator {};
    struct user_allocator {};

    template< typename __tag >
    struct allocator {
        using tag = __tag;

        static void * alloc( size_t size );
        static void free( void * ptr );
    };

    void init( const multiboot::info & info );

} // namespace kernel::mem
