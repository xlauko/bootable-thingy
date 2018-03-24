#include <hw/vga.hpp>

using namespace hw;

VGA::VGA() noexcept
    : row{ 0 }, column{ 0 }
{
    buffer = reinterpret_cast< char* >( 0xB8000 );
    clear();
}

void VGA::set_cursor( uint8_t x, uint8_t y ) noexcept {
    column = x; row = y;
}

void VGA::clear() noexcept {
    row = 0;
    column = 0;

	for ( int i = 0; i < width * height * 2; ++i )
		buffer[ i ] = 0;
}

void VGA::put( char c, uint8_t x, uint8_t y ) {
    const size_t i = ( row * width + column ) * 2;
    buffer[ i ] = c & 0xff;
    buffer[ i + 1 ] = 7; /* vga attribute */
    ++column;
}

void VGA::write( const char c ) noexcept {
    if ( c == '\n' || column >= width )
        column = 0, row++;
    if ( c == '\n' )
        return;
    if ( row >= height )
        row = 0;

    put( c, row, column );
}

void VGA::write( const char* str ) noexcept {
    for ( size_t i = 0; str[ i ]; ++i )
        this->write( str[ i ] );
}

