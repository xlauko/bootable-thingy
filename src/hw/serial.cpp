#include <hw/serial.hpp>

using namespace hw;

constexpr uint16_t Serial::PORTS[];

void Serial::init(uint16_t port_) {
	outb(port_ + 1, 0x00);    // Disable all interrupts
  	outb(port_ + 3, 0x80);    // Enable DLAB (set baud rate divisor)
  	outb(port_ + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
  	outb(port_ + 1, 0x00);    //                  (hi byte)
  	outb(port_ + 3, 0x03);    // 8 bits, no parity, one stop bit
  	outb(port_ + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
  	outb(port_ + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

Serial::Serial( int port )
	: _port( port < 5 ? PORTS[port-1] : 0 )
{
  	static bool initialized = false;
  	if ( !initialized ) {
    	initialized = true;
    	init();
  	}
}

char Serial::read() {
	return inb( _port );
}

void Serial::write( char c ) {
	while ( is_transmit_empty() == 0 );
	outb( _port, c );
}

int Serial::received() {
	return inb( _port + 5 ) & 1;
}

int Serial::is_transmit_empty() {
	return inb( _port + 5 ) & 0x20;
}

void Serial::eot() {
	outb( PORTS[0], 0x4 );
}

