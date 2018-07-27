#include <kernel/os.hpp>

using namespace kernel;
using namespace kernel::dev;

#include <kernel/info.hpp>
#include <kernel/utils.hpp>

#include <multiboot2.h>
#include <stdio.h>

static uint16_t* video = reinterpret_cast< uint16_t * >( 0xB8000 );

namespace kernel {
    void init_pdclib( dev::Serial * ser );
}

void Thingy::start( unsigned long magic, unsigned long addr ) noexcept {
    Serial ser{ Serial::Port::one };
    VGA kvga{ video };

    init_devices( &ser, &kvga );
    init_pdclib( &ser );

    puts( "Initialization finished." );
    if ( multiboot::check( magic, addr ) == Status::failure )
        return;

    auto info = multiboot::info( addr );
    info.print();

    kvga << "You can write now:\n";
    while ( true ) {
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

