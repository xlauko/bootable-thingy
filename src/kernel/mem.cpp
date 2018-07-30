#include <kernel/mem.hpp>
#include <kernel/dt.hpp>
#include <kernel/panic.hpp>

#include <string.h>
#include <stdio.h>

extern "C" uint32_t kernel_end;

namespace kernel::mem {

    uint32_t * frames;

	constexpr size_t index_from_bit( size_t b ) { return b / (8 * 4 ); }
	constexpr size_t offset_from_bit( size_t b ) { return b % (8 * 4 ); }

    namespace {
        using paging::page_table;
        using paging::page_directory;

        struct cr3 {
            static page_table * get() {
                uint32_t cr3;
                asm volatile ("movl %%cr3, %%eax" : "=a" (cr3));
                return reinterpret_cast< page_table * >( cr3 );
            }

            static void set( page_directory * dir ) {
                auto addr = reinterpret_cast< uint32_t >( &dir->tables[0] );
                asm volatile ("movl %%eax, %%cr3" :: "a" (addr));
            }
        };

        struct cr0 {
            static uint32_t get() {
                uint32_t cr0;
                asm volatile ("movl %%cr0, %%eax" : "=a" (cr0));
                return cr0;
            }

            static void set( uint32_t val ) {
                asm volatile ("movl %%eax, %%cr0" :: "a" (val));
            }
        };
    }


	namespace paging {
		page_directory * kernel_page_dir;

        void switch_page_dir( page_directory * dir ) {
            cr3::set( dir );
            cr0::set( cr0::get() | 0x80000000 );
        }

        void page_fault_handler( uint32_t int_no ) {
            uint32_t faulting_address;
            asm volatile( "mov %%cr2, %0" : "=r"( faulting_address ) );

            fprintf( stderr, "Page fault at 0x%x\n", faulting_address );
            panic();
        }

		page_table * page_table::create() {
            page_table * tab = reinterpret_cast< page_table * >( kmalloc_aligned( sizeof( page_table ) ) );

            for ( size_t i = 0; i < page_table::size; i++ ) {
                tab->pages[ i ].present = 0;
                tab->pages[ i ].rw = 1;
            }

            return tab;
        }

		page_table * page_table::empty() {
            return reinterpret_cast< page_table * >( 0x00000002 );
        }

		page_directory * page_directory::create() {
            auto dir = reinterpret_cast< page_directory * >( kmalloc_aligned( sizeof( page_directory ) ) );

            for ( size_t i = 0; i < page_directory::size; i++ )
                dir->tables[ i ] = page_table::empty();

            return dir;
        }
	} // namespace paging

    void identity_map_page( page_directory * dir, uint32_t virt, uint32_t phys ) {
        short id = virt >> 22;

        auto tab = page_table::create();

        dir->tables[ id ] = reinterpret_cast< page_table * >(
            reinterpret_cast< uint32_t >( tab ) | 3 );

        for ( size_t i = 0; i < page_table::size; i++ ) {
            tab->pages[ i ].frame = phys >> 12; // TODO?
            tab->pages[ i ].present = 1;
            phys += 4096;
        }
    }

    void * allocator::alloc( size_t size ) {
    }

    void allocator::free( void * ptr ) {

    }

    heap::header * kheap = nullptr;

    uintptr_t placement_addr = reinterpret_cast< uintptr_t >( &kernel_end );

    void * fmalloc( size_t size ) {
        auto res = reinterpret_cast< void * >( placement_addr );
        memset( res, 0, size );
        placement_addr += size;
        return res;
    }

    void * kmalloc_aligned( size_t size ) {
        placement_addr &= 0xFFFFF000;
        placement_addr += 0x1000;

        auto res = placement_addr;
        placement_addr += size;
        return reinterpret_cast< void * >( res );
    }

    void heap::init() {
        kheap = reinterpret_cast< heap::header * >( fmalloc( heap::kernel_heap_size ) );
        kheap->magic = heap::magic;
        kheap->free = true;
        kheap->size = heap::kernel_heap_size - sizeof( heap::header ) - sizeof( heap::footer );
        kheap->magic2 = heap::magic2;

        auto footer = reinterpret_cast< heap::footer * >( reinterpret_cast< uintptr_t >( kheap )
                                                        + sizeof( heap::header ) + kheap->size );
        footer->magic = heap::magic;
        footer->size = heap::kernel_heap_end;
        footer->magic2 = heap::magic2;

        // TODO init user heap
    }

    void init( size_t mem_size ) {
        heap::init();

        paging::kernel_page_dir = page_directory::create();

        for ( size_t i = 0; i < 0xF0000000; i += 1024 * 4096 )
            identity_map_page( paging::kernel_page_dir, i, i );

        isrs::install_handler( 14, paging::page_fault_handler );

        paging::switch_page_dir( paging::kernel_page_dir );
    }
} // namespace kernel::mem
