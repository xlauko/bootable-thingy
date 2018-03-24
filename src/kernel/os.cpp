#include <kernel/os.hpp>

using namespace kernel;

void Thingy::start( uint32_t boot_magic, uint32_t boot_addr ) {

}

extern "C" int main( unsigned long magic, unsigned long addr ) {
    Thingy::start( magic, addr );
}
