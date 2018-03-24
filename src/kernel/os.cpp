#include <kernel/os.hpp>
#include <hw/serial.hpp>
#include <hw/vga.hpp>

using namespace kernel;

int Thingy::start( uint32_t boot_magic, uint32_t boot_addr ) {
    auto vga = hw::VGA();
    vga.write( "hello vga" );
    return 0;
    //auto& com = hw::Serial::port< 1 >();
    /*if ( boot_magic != MULTIBOOT2_BOOTLOADER_MAGIC ) {
        com.print( "invalid magic number\n" );
        return;
    }

    if ( boot_addr & 7 ) {
        com.print( "unaligned mbi\n" );
        return;
    }*/

}

extern "C" int boot( uint32_t boot_magic, uint32_t boot_addr ) {
    return Thingy::start( boot_magic, boot_addr );
}
