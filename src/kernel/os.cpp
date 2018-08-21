#include <kernel/os.hpp>

using namespace kernel;
using namespace kernel::dev;

#include <kernel/info.hpp>
#include <kernel/utils.hpp>
#include <kernel/panic.hpp>
#include <kernel/mem.hpp>
#include <kernel/dt.hpp>
#include <kernel/user.hpp>
#include <kernel/syscall.hpp>

#include <multiboot2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t* video = reinterpret_cast< uint16_t * >( 0xB8000 );

unsigned int stack_ptr;

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

    mem::init( info );

    init_devices( &ser, &kvga );
    init_pdclib( &ser );

    syscall::init();

    user::executable program;

    *( (int *)0xffasdas ) = 10;
    /*info.yield( multiboot::information_type::module, [&program] ( const auto & item ) {
        using namespace mem::paging;

        auto mod = reinterpret_cast< multiboot::modules_information * >( item );
        if ( strcmp( mod->command, "program.data" ) == 0 ) {
            program.text.addr = reinterpret_cast< uint32_t >( nullptr ); // TODO alloc
            // memcpy module to memory
            program.text.size = ( mod->end - mod->start  + page::size - 1 ) / page::size;
        }

        if ( strcmp( mod->command, "program.text" ) == 0 ) {
            program.text.addr = reinterpret_cast< uint32_t >( nullptr ); // TODO alloc
            // memcpy module to memory
            program.text.size = ( mod->end - mod->start  + page::size - 1 ) / page::size;
        }
    } );*/

    puts( "\nInitialization of Thingy finished." );
    puts( "===============================================================================" );

    int ret = program.start();

    puts( "===============================================================================" );

    printf( "Program finished with value: %d\n", ret );

    // TODO page fault? *(reinterpret_cast< int * >( 0x1000 ) ) = 42;

    // TODO press key to irq
    /*irq::install_handler( 1, [] ( registers_t * ) {
        puts( "pressed key" );
        irq::pic::enable( 0 );
        irq::pic::disable( 1 );
    } );*/

    //irq::install_handler( 1, test_handler );
    //asm volatile( "int $33\n" );

    // Go go user space

    /*kvga << "You can write now:\n";
    while ( true ) {
        kvga << ser.read();
    }*/
}

void Thingy::init_devices( dev::Serial *ser, dev::VGA *_vga ) noexcept {
    serial = ser; ser->init();
    vga = _vga;
}

extern "C" void main( unsigned long magic, unsigned long addr, unsigned int esp ) {
    stack_ptr = esp;
    Thingy::start( magic, addr );
}

