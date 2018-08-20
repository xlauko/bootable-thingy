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

    struct frame_allocator {
        phys::address_t alloc();
        phys::address_t alloc( size_t num_of_frames );

        void skip_allocated_frames();

        void free( phys::address_t );

        static void init( const multiboot::info & info );

        size_t last = 0;
    };

    extern frame_allocator falloc;

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

            static constexpr size_t index( phys::address_t addr ) {
                return addr / size;
            }
        };
    } // namespace paging


    struct heap {

        static constexpr uint32_t magic = 0x04206969;
        static constexpr uint32_t magic2 = 0xDEADBEEF;

        static constexpr size_t kernel_heap_size = 0xFFFFF;
        static constexpr uint32_t kernel_heap_end = 0xFFFFDEAD;

        struct header {
            uint32_t magic;
            bool free;
            uint32_t size;
            uint32_t magic2;
        };

        struct footer {
            uint32_t magic;
            uint32_t size;
            uint32_t magic2;
        };

        void * malloc( size_t size );
        void free( void * ptr );

        void * umalloc( size_t size );
        void ufree( void * ptr );

        static void init();

        header * get_header();
        footer * get_footer();

        size_t size();
        size_t total_size();

        bool can_fit( size_t size );

        heap * next();
        heap * prev();

        void * memory();

        bool check();
    };

    void * fmalloc( size_t size );
    void * kmalloc_aligned( size_t size );

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
