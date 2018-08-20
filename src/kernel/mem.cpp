#include <kernel/mem.hpp>
#include <kernel/dt.hpp>
#include <kernel/panic.hpp>
#include <kernel/dev.hpp>

#include <string.h>
#include <stdio.h>

extern "C" char __kernel_start;
extern "C" char __kernel_end;

namespace {

    template< typename T1, typename T2 >
    T1 add_offset_and_cast( T2 val, size_t offset ) {
        return reinterpret_cast< T1 >( reinterpret_cast< uintptr_t >( val ) + offset );
    }

} // anonymous namespace

namespace kernel::mem {

    frame_allocator falloc;

    namespace {
        static constexpr size_t num_of_pages = 0x4000000;

        struct frame_bitmap {

            static constexpr size_t size = num_of_pages / 8;

            size_t offset( size_t idx ) {
                return 7 - ( idx % 8 );
            }

            bool get( size_t idx ) {
                return bitmap[ idx / 8 ] & ( 1 << offset( idx ) );
            }

            void set( size_t idx ) {
                bitmap[ idx / 8 ] = bitmap[ idx / 8 ] | ( 1 << offset( idx ) );
            }

            void reset( size_t idx ) {
                bitmap[ idx / 8 ] = bitmap[ idx / 8 ] & ~( 1 << offset( idx ) );
            }

            char bitmap [ size ];
        };

        frame_bitmap fbitmap;
    }

    void frame_allocator::init( const multiboot::info & info ) {
        using namespace mem::paging;
        memset( fbitmap.bitmap, 0, frame_bitmap::size );

        auto kernel_start = reinterpret_cast< uint32_t >( &__kernel_start );
        auto kernel_end = reinterpret_cast< uint32_t >( &__kernel_end );

        for ( size_t addr = kernel_start; addr < kernel_end; addr += page::size ) {
            fbitmap.set( page::index( addr ) );
        }

        info.yield( multiboot::information_type::memory_map, [&] ( const auto & item ) {
            auto mmap = reinterpret_cast< multiboot_tag_mmap * >( item );

            auto next_entry = [mmap] ( const auto & entry ) {
                return reinterpret_cast< multiboot_memory_map_t * >(
                       reinterpret_cast< uintptr_t >( entry ) + mmap->entry_size );
            };

            auto is_end = [mmap] ( const auto & entry ) {
                auto mmap_end = reinterpret_cast< uintptr_t >( mmap ) + mmap->size;
                return reinterpret_cast< uintptr_t >( entry ) >= mmap_end;
            };

            for ( auto entry = mmap->entries; !is_end( entry ); entry = next_entry( entry ) ) {
                if ( entry->type != MULTIBOOT_MEMORY_AVAILABLE ) {
                    size_t size = entry->addr + entry->len;
                    for ( size_t addr = entry->addr; addr < size; addr += page::size )
                        fbitmap.set( page::index( addr ) );
                }
            }

        } );

        uintptr_t video_addr = 0xB8000;
        size_t video_size = dev::VGA::width * dev::VGA::height * 2;
        for ( auto fb = video_addr; fb < video_addr + video_size; fb += page::size )
            fbitmap.set( page::index( fb ) );

        info.yield( multiboot::information_type::module, [] ( const auto & item ) {
            using namespace mem::paging;
            auto mod = reinterpret_cast< multiboot::modules_information * >( item );
            for ( size_t addr = mod->start; addr < mod->end; addr += page::size )
                fbitmap.set( page::index( addr ) );
        } );
    }

    void frame_allocator::skip_allocated_frames() {
        while ( fbitmap.get( last ) )
            last = ( last + 1 ) % num_of_pages;
    }

    frame_allocator::frame frame_allocator::alloc() {
        return alloc( 1 );
    }

    frame_allocator::frame frame_allocator::alloc( size_t num_of_frames ) {
        bool available = false;

        while ( !available ) {
            skip_allocated_frames();

            available = true;
            for ( int i = 0; i < num_of_frames; ++i ) {
                if ( fbitmap.get( ( last + i ) % num_of_pages ) ) {
                    available = false;
                    last = last + i;
                    break;
                }
            }
        }

        phys::address_t addr = paging::page::size * last;

        for ( int i = 0; i < num_of_frames; ++i )
            fbitmap.set( last + i );
        skip_allocated_frames();

        return { addr, num_of_frames };
    }

    void frame_allocator::free( frame_allocator::frame frame ) {
        for ( int i = 0; i < frame.size; ++i ) {
            if ( !fbitmap.get( paging::page::index( frame.addr ) + i ) )
                panic();
            fbitmap.reset( paging::page::index( frame.addr ) + i );
        }
    }

    page_allocator palloc;

    void page_allocator::init( frame_allocator * allocator ) {
        palloc.allocator = allocator;
    }

    paging::page page_allocator::alloc( size_t num, bool user ) {
        find_space( num );
    }

    void page_allocator::free( paging::page page ) {

    }

    paging::page page_allocator::find_space( size_t num ) {

    }

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

        void page_fault_handler( registers_t * ) {
            uint32_t faulting_address;
            asm volatile( "mov %%cr2, %0" : "=r"( faulting_address ) );

            fprintf( stderr, "Page fault at 0x%x\n", faulting_address );
            panic();
        }

		page_table * page_table::create() {
            page_table * tab = reinterpret_cast< page_table * >( kmalloc_page_aligned( sizeof( page_table ) ) );

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
            auto dir = reinterpret_cast< page_directory * >( kmalloc_page_aligned( sizeof( page_directory ) ) );

            for ( size_t i = 0; i < page_directory::size; i++ )
                dir->tables[ i ] = page_table::empty();

            return dir;
        }
	} // namespace paging

    void identity_map_page( page_directory * dir, virt::address_t virt, phys::address_t phys ) {
        short id = virt >> 22;

        auto tab = page_table::create();

        dir->tables[ id ] = reinterpret_cast< page_table * >(
            reinterpret_cast< uint32_t >( tab ) | 3 );

        for ( size_t i = 0; i < page_table::size; i++ ) {
            tab->pages[ i ].frame = phys >> 12;
            tab->pages[ i ].present = 1;
            phys += 4096;
        }
    }

    void * kmalloc_page_aligned( size_t size ) {
        if ( size > paging::page::size )
            panic();
        return reinterpret_cast< void * >( falloc.alloc().addr );
    }

    template<>
    void * allocator< kernel_allocator >::alloc( size_t size ) {
//        return kernel_heap->malloc( size );
    }

    template<>
    void allocator< kernel_allocator >::free( void * ptr ) {
//        return kernel_heap->free( ptr );
    }

    template<>
    void * allocator< user_allocator >::alloc( size_t size ) {
//        return user_heap->malloc( size );
    }

    template<>
    void allocator< user_allocator >::free( void * ptr ) {
//        return user_heap->free( ptr );
    }

    void init( const multiboot::info & info ) {
        frame_allocator::init( info );

        paging::kernel_page_dir = page_directory::create();

        for ( size_t i = 0; i < num_of_pages; i += 1024 * 4096 )
            identity_map_page( paging::kernel_page_dir, i, i );

        page_allocator::init( &falloc );

        /*heap::init();



        isrs::install_handler( 14, paging::page_fault_handler );

        paging::switch_page_dir( paging::kernel_page_dir );*/
    }
} // namespace kernel::mem
