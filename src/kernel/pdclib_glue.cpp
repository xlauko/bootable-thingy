#include <stdio.h>
#include <_PDCLIB_glue.h>

#include <kernel/os.hpp>
#include <kernel/mem.hpp>

extern _PDCLIB_fileops_t _PDCLIB_fileops;

extern FILE * stdin;
extern FILE * stdout;
extern FILE * stderr;

extern "C" void _PDCLIB_Exit( int status ) {
    asm volatile ("hlt");
}

extern "C" void free( void * ptr ) noexcept {
    return kernel::mem::_allocator.free( ptr );
}

extern "C" void * malloc( size_t size ) noexcept {
    return kernel::mem::_allocator.alloc( size );
}

extern "C" void * realloc( void * ptr, size_t size ) noexcept {
    return kernel::mem::_allocator.realloc( ptr, size );
}

static bool readf( _PDCLIB_fd_t self, void * buff, size_t length, size_t * numBytesRead ) {
    auto in = static_cast< kernel::dev::Serial * >( self.pointer );
    auto buf = reinterpret_cast< char * >( buff );

    int idx = 0;
    while ( idx < length ) {
        char c = in->read();
        buf[ idx ] = c;
        idx++;
    }

    *numBytesRead = idx;
    return true;
}

static bool writef( _PDCLIB_fd_t self, const void * buff, size_t len, size_t * written ) {
    auto out = static_cast< kernel::dev::Serial * >( self.pointer );
    auto str = static_cast< const char * >( buff );
    *written = out->print( str, len );
    return true;
}

static bool seekf( _PDCLIB_fd_t self, int_fast64_t offset, int whence, int_fast64_t* newPos ) {
    return false;
}

static void closef( _PDCLIB_fd_t self ) { }

namespace kernel {

    void init_pdclib( dev::Serial * ser ) {
        _PDCLIB_fileops.read = readf;
        _PDCLIB_fileops.write = writef;
        _PDCLIB_fileops.seek = seekf;
        _PDCLIB_fileops.close = closef;

        stdin->handle.pointer = ser;
        stdout->handle.pointer = ser;
        stderr->handle.pointer = ser;
    }

}
