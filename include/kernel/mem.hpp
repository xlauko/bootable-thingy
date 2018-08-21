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

    void set_kernel_stack( uintptr_t stack );

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
            size_t num;
        };

        page_entry & get_page( virt::address_t addr );

    } // namespace paging

    inline phys::address_t virt_2_phys( virt::address_t addr ) {
        return ( paging::get_page( addr ).raw & ~0xfff ) | ( addr & 0xfff );
    }

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
        void unmap( virt::address_t virt );

        static void init( frame_allocator * allocator );

        static constexpr uint32_t kernel_flags = 0x103;
        static constexpr uint32_t user_flags = 0x07;

        frame_allocator * allocator;
    };

    extern page_allocator palloc;

    void * kmalloc_page_aligned( size_t size );

    struct allocator {
        void * alloc( size_t size, bool user = false );
        void * realloc( void * ptr, size_t size, bool user = false );
        void free( void * ptr );

        struct alignas( 8 ) node {
            static constexpr uint32_t magic = 0xDEADBEEF;

            struct metadata_header {
                uint32_t magic_begin;
                size_t size;
                node * next;
                bool free;
                bool user;
                uint32_t magic_end;
            };

            struct metadata_footer {
                uint32_t magic_begin;
                size_t size;
                uint32_t magic_end;
            };

            metadata_header __header;

            metadata_header & header() { return __header; }

            metadata_footer & footer() {
                return *( reinterpret_cast< metadata_footer * >(
                reinterpret_cast< uintptr_t >( this ) + sizeof( metadata_header ) +__header.size ) );
            }

            node * next() {
                return reinterpret_cast< node * >(
                       reinterpret_cast< uintptr_t >( this ) + sizeof( metadata_header )
                                                             + sizeof( metadata_footer )
                                                             + __header.size );
            }

            bool check() {
                return __header.magic_begin == magic && __header.magic_end == magic;
            }

            bool fit( size_t size, bool user );

            void * data() {
                return reinterpret_cast< void * >(
                    reinterpret_cast< uintptr_t >( this ) + sizeof( metadata_header )
                );
            }

        };

        node * freelist = nullptr;
    };

    extern allocator _allocator;

    void init( const multiboot::info & info );

} // namespace kernel::mem
