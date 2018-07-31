#include <kernel/mem.hpp>
#include <kernel/dt.hpp>
#include <kernel/panic.hpp>

#include <string.h>
#include <stdio.h>

extern "C" uint32_t kernel_end;

namespace {

    template< typename T1, typename T2 >
    T1 add_offset_and_cast( T2 val, size_t offset ) {
        return reinterpret_cast< T1 >( reinterpret_cast< uintptr_t >( val ) + offset );
    }

} // anonymous namespace

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

    heap * kernel_heap = nullptr;

    void * allocator::alloc( size_t size ) {
        // TODO user land alloc
        return kernel_heap->malloc( size );
    }

    void allocator::free( void * ptr ) {
        // TODO user land free
        return kernel_heap->free( ptr );
    }


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
        kernel_heap = reinterpret_cast< heap * >( fmalloc( heap::kernel_heap_size ) );
        auto header = kernel_heap->get_header();
        header->magic = heap::magic;
        header->free = true;
        header->size = heap::kernel_heap_size - sizeof( heap::header ) - sizeof( heap::footer );
        header->magic2 = heap::magic2;

        auto footer = kernel_heap->get_footer();
        footer->magic = heap::magic;
        footer->size = heap::kernel_heap_end;
        footer->magic2 = heap::magic2;

        // TODO init user heap
    }

    size_t heap::size() { return get_header()->size; }

    size_t heap::total_size() { return size() + sizeof( header ) + sizeof( footer ); }

    heap::header * heap::get_header() {
        return reinterpret_cast< heap::header * >( this );
    }

    heap::footer * heap::get_footer() {
        return add_offset_and_cast< heap::footer * >( get_header(), sizeof( heap::header ) + size() );
    }

    namespace internal {
        heap * find_available_heap( heap * heap_ptr, size_t size ) {
            while ( !heap_ptr->can_fit( size ) || !heap_ptr->get_header()->free ) {
                auto header_ptr = heap_ptr->get_header();
                auto footer_ptr = heap_ptr->get_footer();

                if ( footer_ptr->size == heap::kernel_heap_end && !header_ptr->free ) {
                    fprintf( stderr, "Out of heap space.\n" );
                    panic();
                }

                if ( footer_ptr->size != header_ptr->size ) {
                    fprintf( stderr, "Heap size mismatch.\n" );
                    panic();
                }

                heap_ptr = heap_ptr->next();
            }

            return heap_ptr;
        }

        void split_heap( heap * heap_ptr, size_t size ) {
            auto footer_ptr = add_offset_and_cast< heap::footer * >( heap_ptr, sizeof( heap::header ) + size );

            footer_ptr->magic = heap::magic;
            footer_ptr->size = size;
            footer_ptr->magic2 = heap::magic2;

            auto header_ptr = heap_ptr->get_header();

            size_t rest = heap_ptr->size() - sizeof( heap::header ) - sizeof( heap::footer ) - size;
            header_ptr->size = size;

            auto heap_rest = add_offset_and_cast< heap * >( footer_ptr, sizeof( heap::footer ) );

            auto header_rest = heap_rest->get_header();
            header_rest->magic = heap::magic;
            header_rest->size = rest;
            header_rest->free = true;
            header_rest->magic2 = heap::magic2;

            auto footer_rest = heap_rest->get_footer();

            footer_rest->size = rest;

            if ( !heap_ptr->check() || !heap_rest->check() ) {
                fprintf( stderr, "Splitting of heap damaged metadata.\n" );
                panic();
            }
        }

        void * malloc( heap * heap_ptr, size_t size ) {
            auto space = find_available_heap( heap_ptr, size );
            space->get_header()->free = false;
            split_heap( space, size );
            return space->memory();
        }

        void free( heap * heap_ptr, void * addr ) {
            auto tofree = add_offset_and_cast< heap * >( addr, - sizeof( heap::header ) );

            if ( tofree == heap_ptr ) {
                heap_ptr->get_header()->free = true;
                return;
            }

            if ( !tofree->check() ) {
                fprintf( stderr, "Trying to free invalid heap object.\n" );
                panic();
            }

            auto prev_ptr = tofree->prev();

            if ( !prev_ptr->check() ) {
                fprintf( stderr, "Predecessor in heap is corrupted.\n" );
                panic();
            }

            prev_ptr->get_header()->size += tofree->total_size();
            prev_ptr->get_footer()->size = prev_ptr->size();
        }
    } // namespace internal;

    bool heap::check() {
        auto head = get_header();
        auto foot = get_footer();
        return head->magic == heap::magic && head->magic2 == heap::magic2 &&
               foot->magic == heap::magic && foot->magic2 == heap::magic2 &&
               head->size == foot->size;
    }

    void * heap::memory() {
        return add_offset_and_cast< void * >( get_header(), sizeof( header ) );
    }

    bool heap::can_fit( size_t size ) {
        return this->size() > size;
    }

    heap * heap::next() {
        return add_offset_and_cast< heap * >( get_footer(), sizeof( footer ) );
    }

    heap * heap::prev() {
        auto prev_footer = add_offset_and_cast< footer * >( this, -sizeof( footer ) );
        if ( prev_footer->magic != heap::magic && prev_footer->magic2 != heap::magic2 ) {
            fprintf( stderr, "Predecessor in heap is corrupted.\n" );
            panic();
        }

        return add_offset_and_cast< heap * >( this,
             - ( sizeof( header ) + sizeof( footer ) + prev_footer->size ) );
    }

    void * heap::malloc( size_t size ) {
        if ( kernel_heap == nullptr ) { // TODO remove for user land
            return fmalloc( size );
        }

        return internal::malloc( this, size );
    }


    void heap::free( void * ptr ) {
        if ( kernel_heap == nullptr ) { // TODO remove for user land
            return;
        }

        internal::free( this, ptr );
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