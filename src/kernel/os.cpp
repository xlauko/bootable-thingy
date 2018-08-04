#include <kernel/os.hpp>

using namespace kernel;
using namespace kernel::dev;

#include <kernel/info.hpp>
#include <kernel/utils.hpp>
#include <kernel/panic.hpp>
#include <kernel/mem.hpp>
#include <kernel/dt.hpp>

#include <multiboot2.h>
#include <stdio.h>
#include <stdlib.h>

static uint16_t* video = reinterpret_cast< uint16_t * >( 0xB8000 );

namespace kernel {
    void init_pdclib( dev::Serial * ser );
}

void test_handler( registers_t * regs ) {
    printf( "in irq handler %d\n", regs->int_no );
}

void Thingy::start( unsigned long magic, unsigned long addr ) noexcept {
    Serial ser{ Serial::Port::one };
    VGA kvga{ video };

    if ( multiboot::check( magic, addr ) == Status::failure )
        panic();

    auto info = multiboot::info( addr );

    kernel::dt::init();

    auto mem_info = info.mem();
    mem::init( mem_info.upper + mem_info.lower );

    init_devices( &ser, &kvga );
    init_pdclib( &ser );

    irq::install_handler( 1, test_handler );
    puts( "Initialization of Thingy finished." );
    asm volatile( "int $33\n" );
    // info.print();

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

