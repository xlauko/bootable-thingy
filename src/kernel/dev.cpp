#include <kernel/dev.hpp>

#include <kernel/ioport.hpp>


using namespace kernel::dev;

Status Serial::init() {
	outb( _port + 1, 0x00);    // Disable all interrupts
  	outb( _port + 3, 0x80);    // Enable DLAB (set baud rate divisor)
  	outb( _port + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
  	outb( _port + 1, 0x00);    //                  (hi byte)
  	outb( _port + 3, 0x03);    // 8 bits, no parity, one stop bit
  	outb( _port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
  	outb( _port + 4, 0x0B);    // IRQs enabled, RTS/DSR set
  	return Status::success;
}

char Serial::read() {
    while ( received() == 0 );

	return inb( _port );
}

Status Serial::putchar( char c ) {
	while ( is_transmit_empty() == 0 );
	outb( _port, c );

	return Status::success;
}

std::size_t Serial::print( const char * str, std::size_t len ) {
    std::size_t written = 0;

    for ( std::size_t i = 0; i < len; ++i ) {
        auto result = putchar( str[ i ] );
        if ( result == Status::failure )
            break;
        written++;
    }
    return written;
}

int Serial::received() {
	return inb( _port + 5 ) & 1;
}

int Serial::is_transmit_empty() {
	return inb( _port + 5 ) & 0x20;
}

void Serial::eot() {
	outb( _port, 0x4 );
}

constexpr inline uint16_t make_entry( char c, uint8_t color ) noexcept {
    uint16_t c16 = c;
    uint16_t color16 = color;
    return c16 | color16 << 8;
}

void VGA::set_cursor( uint8_t x, uint8_t y ) noexcept {
    column = x; row = y;
}

void VGA::clear() noexcept {
	for ( int i = 0; i < width * height * 2; ++i )
		buffer[ i ] = 0;

	row = 0;
   	column = 0;
}

void VGA::put( char c, uint8_t x, uint8_t y ) {
    const size_t i = ( row * width + column ) * 2;
    buffer[ i ] = make_entry( c, color );
    ++column;
}

void VGA::write( char c ) noexcept {
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

void VGA::init() noexcept {
    clear();
}

