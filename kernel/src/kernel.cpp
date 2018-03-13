#include "multiboot2.h"
#include "libutils.h"
#include "pgtable.h"

using MBTag = struct multiboot_tag;
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
}

extern "C" {
    int inb( int port );
    void outb( int port, int val );
}

#define PORT 0x3F8

void init_serial() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received() {
   return inb(PORT + 5) & 1;
}

char read_serial() {
   while (serial_received() == 0);

   return inb(PORT);
}

int is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}

void write_serial(char a) {
   while (is_transmit_empty() == 0);

   outb(PORT,a);
}

void main( unsigned long magic, unsigned long addr )
{
    clear();

    if ( magic != MULTIBOOT2_BOOTLOADER_MAGIC ) {
        puts( "invalid magic number :-(" );
        return;
    }

    if ( addr & 7 ) {
        puts( "unaligned mbi :-(" );
        return;
    }

    print_tags( addr );

    init_serial();
    init_page_dir();

    printf( "You can write now:\n" );

    while (true) {
        putchar( read_serial() );
    }
}
