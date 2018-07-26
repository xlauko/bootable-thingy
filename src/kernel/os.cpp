#include <kernel/os.hpp>

using namespace kernel;
using namespace kernel::dev;

#include <multiboot2.h>
#include <libutils.h>

/*using MBTag = struct multiboot_tag;
using MBString = struct multiboot_tag_string;
using MBModule = struct multiboot_tag_module;
using MBMemInfo = struct multiboot_tag_basic_meminfo;
using MBBootDev = struct multiboot_tag_bootdev;

struct Module {
    Module( MBModule * tag ) : _tag( tag ) {}

    char* begin() const {
        return reinterpret_cast< char* >( _tag->mod_start );
    }

    char* end() const {
        return reinterpret_cast< char* >( _tag->mod_end );
    }

    void dump() const {
        for ( auto it = begin(); it != end(); ++it )
	        putchar( *it );
    }

    MBModule *_tag;
};

void print_tags( unsigned long addr ) {
    auto tag = reinterpret_cast< MBTag* >( addr );
    tag = add_offset( tag, 8 ); // skip initial tag

    while ( tag->type != MULTIBOOT_TAG_TYPE_END ) {
		switch ( tag->type ) {
			case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
                auto m = reinterpret_cast< MBString* >( tag );
            	printf ("Boot loader name = %s\n", m->string );
				break;
            }
            case MULTIBOOT_TAG_TYPE_MODULE: {
                auto m = reinterpret_cast< MBModule* >( tag );
            	printf ("Module at 0x%x-0x%x. Content:\n", m->mod_start, m->mod_end);
                Module( m ).dump();
            	printf ("End of module.\n\n");
                break;
            }
			case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
                auto m = reinterpret_cast< MBMemInfo* >( tag );
            	printf ("mem_lower = %uKB, mem_upper = %uKB\n", m->mem_lower, m->mem_upper);
         		break;
            }
		}

        auto offset = ( tag->size + 7 ) & ~7;
	    tag = add_offset( tag, offset );
    }
    putchar('\n');
}*/

static uint16_t* video = reinterpret_cast< uint16_t * >( 0xB8000 );

void Thingy::start( unsigned long magic, unsigned long addr ) noexcept {
    Serial ser{ Serial::Port::one };
    VGA kvga{ video };

    init_devices( &ser, &kvga );

    if ( magic != MULTIBOOT2_BOOTLOADER_MAGIC ) {
        kvga << "invalid magic number :-(";
        return;
    }

    if ( addr & 7 ) {
        kvga << "unaligned mbi :-(";
        return;
    }

    //print_tags( addr );

    kvga << "You can write now:\n";

    while (true) {
        kvga << ser.read();
    }
}

void Thingy::init_devices( dev::Serial *ser, dev::VGA *_vga ) noexcept {
    serial = ser; ser->init();
    vga = _vga;
}

extern "C" void main( unsigned long magic, unsigned long addr ) {
    Thingy::start( magic, addr );
}

