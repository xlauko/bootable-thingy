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

        page_table * get_table( virt::address_t addr ) {
            return kernel_page_dir->tables[ ( addr >> 20 ) & ~0x3 ];
        }

        size_t page_idx( virt::address_t addr ) {
            return ( addr >> 12 ) & (( 1 << 12 ) - 1 );
        }

        page_entry & get_page( virt::address_t addr ) {
            return get_table( addr )->pages[ page_idx( addr ) ];
        }

        size_t offset( virt::address_t addr ) {
            return addr & ~0xfff;
        }

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

    phys::address_t virt_2_phys( virt::address_t addr ) {
        return ( paging::get_page( addr ).raw & ~0xfff ) | ( addr & 0xfff );
    }

    void identity_map_page( page_directory * dir, virt::address_t virt, phys::address_t phys ) {
        short id = virt >> 22;

        auto tab = page_table::create();
        dir->tables[ id ] = reinterpret_cast< page_table * >(
            reinterpret_cast< uint32_t >( tab ) | 0x3 );

        for ( size_t i = 0; i < page_table::size; i++ ) {
            tab->pages[ i ].frame = phys >> 12;
            tab->pages[ i ].present = 1;
            phys += 4096;
        }
    }

    frame_allocator falloc;

    void * kmalloc_page_aligned( size_t size ) {
        if ( size > paging::page::size )
            panic();
        return reinterpret_cast< void * >( falloc.alloc().addr );
    }

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

    void page_allocator::map( phys::address_t phys, virt::address_t virt, uint32_t flags ) {
        using namespace paging;

        auto tab = virt >> 22;
        auto entry = reinterpret_cast< page_entry * >( tab );
        if ( !entry->present ) {
            auto table = page_table::create();
            kernel_page_dir->tables[ virt >> 22 ] = reinterpret_cast< page_table * >(
                reinterpret_cast< uint32_t >( table ) | flags );
        }

        get_page( virt ).raw = phys | flags;
    }

    paging::page page_allocator::alloc( size_t num, bool user ) {
        using namespace paging;
        auto addr = find_space( num, user );

        for ( int i = 0; i < num; ++i )
            if ( user )
                map( falloc.alloc().addr, addr + i * page::size, user_flags );
            else
                map( falloc.alloc().addr, addr + i * page::size, kernel_flags );

        return { addr, num };
    }

    void page_allocator::unmap( virt::address_t addr ) {
        reinterpret_cast< paging::page_entry * >( paging::get_table( addr ) )->present = 0;
    }

    void page_allocator::free( paging::page page ) {
        for ( int i = 0; i < page.num; ++i ) {
            auto virt = page.addr + i * paging::page::size;
            auto phys = virt_2_phys( virt );
            unmap( virt );
            falloc.free( { phys, 1 } );
        }
    }

    virt::address_t page_allocator::skip_used_pages( virt::address_t addr ) {
        using namespace paging;

        while ( true ) {
            auto tab = get_table( addr );
            if ( reinterpret_cast< page_entry * >( tab )->present ) {
                for ( int i = page_idx( addr ); i < page_table::size; ++i ) {
                    auto page = tab->pages[ i ];
                    if ( !page.present )
                        return addr + i * page::size;
                }

                addr = ( addr + page_table::size * page::size );
            } else {
                return addr;
            }
        }
    }

    size_t page_allocator::unused_space_from_addr( virt::address_t addr, bool user, size_t bound ) {
        using namespace paging;
        size_t free_pages = 0;

        while ( true ) {
            if ( free_pages > bound )
                return free_pages;
            auto tab = get_table( addr );
            if ( !reinterpret_cast< page_entry * >( tab )->present ) {
                free_pages += page_table::size;
                addr += page_table::size * page::size;
            } else {
                for ( int i = page_idx( addr ); i < page_table::size; ++i ) {
                    auto page = tab->pages[ i ];
                    if ( !page.present && page.user == user )
                        free_pages++;
                    else
                        return free_pages;
                }

                addr += ( page_table::size - page_idx( addr ) ) * page::size;
            }
        }

        return free_pages;
    }

    virt::address_t page_allocator::find_space( size_t num, bool user ) {
        using namespace paging;

        virt::address_t addr = 0;
        size_t free_pages = 0;

        while ( free_pages < num ) {
            addr += free_pages * page::size;
            addr = skip_used_pages( addr );
            free_pages = unused_space_from_addr( addr, user, num );
        }

        return addr;
    }

    allocator _allocator;

    bool allocator::node::fit( size_t size, bool user ) {
        return size <= __header.size && __header.free && __header.user == user;
    }

    void * allocator::alloc( size_t size, bool user ) {
        if ( size == 0 )
            return nullptr;

        node * curr = freelist;
        while ( curr && curr->check() && !curr->fit( size, user ) )
            curr = curr->next();

        constexpr size_t metadata_size = sizeof( node::metadata_header ) + sizeof( node::metadata_footer );

        if ( !curr ) {
            size_t alloc_pages = (size + metadata_size + paging::page::size - 1 ) / paging::page::size;
            auto page = palloc.alloc( alloc_pages, user );

            size_t available_memory = alloc_pages * paging::page::size - metadata_size;

            curr = reinterpret_cast< node * >( virt_2_phys( page.addr ) );
            curr->header().magic_begin = node::magic;
            curr->header().size = available_memory;
            curr->header().next = freelist;
            curr->header().free = false;
            curr->header().user = user;
            curr->header().magic_end = node::magic;

            curr->footer().magic_begin = node::magic;
            curr->footer().size = available_memory;
            curr->footer().magic_end = node::magic;
            freelist = curr;
        }

        size_t available_memory = curr->header().size;
        if ( available_memory > size + metadata_size + 8 ) {
            size_t free_space = available_memory - metadata_size - size;

            curr->header().size = size;

            auto &footer = curr->footer();
            footer.magic_begin = node::magic;
            footer.size = size;
            footer.magic_end = node::magic;

            node * next = reinterpret_cast< node * >(
                          reinterpret_cast< uintptr_t >( &curr->footer() ) + sizeof( node::metadata_footer ) );

            next->header().magic_begin = node::magic;
            next->header().size = free_space;
            next->header().next = curr->header().next;
            next->header().free = true;
            next->header().user = curr->header().user;
            next->header().magic_end = node::magic;

            next->footer().size = available_memory;

            curr->header().next = next;
        }

        return curr->data();
    }

    void * allocator::realloc( void * ptr, size_t size, bool user ) {
        auto node = reinterpret_cast< struct node * >(
                    reinterpret_cast< uintptr_t >( ptr ) - sizeof ( node::metadata_header ) );

        if ( size < node->header().size )
            return ptr;

        auto place = alloc( size, user );
        memcpy( place, ptr, node->header().size );

        free( ptr );

        return place;
    }

    void allocator::free( void * ptr ) {
        if ( ptr == nullptr ) return;

        auto curr = reinterpret_cast< node * >( (uintptr_t)ptr - sizeof( node::metadata_header ) );
        curr->header().free = true;
    }

    void init( const multiboot::info & info ) {
        using namespace paging;
        frame_allocator::init( info );

        paging::kernel_page_dir = page_directory::create();

        for ( size_t i = 0; i < num_of_pages; i += page_table::size * page::size )
            identity_map_page( paging::kernel_page_dir, i, i );

        page_allocator::init( &falloc );

        isrs::install_handler( 14, paging::page_fault_handler );

        paging::switch_page_dir( paging::kernel_page_dir );
    }
} // namespace kernel::mem
