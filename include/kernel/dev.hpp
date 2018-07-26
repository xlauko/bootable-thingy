#pragma once

#include <type_traits>
#include <cstdint>
#include <stddef.h>

namespace kernel::dev {

    enum class Status {
        success,
        failure
    };

    struct Serial {
        enum class Port : uint16_t {
            one = 0x3f8,
            two = 0x2f8,
            three = 0x3e8,
            four = 0x2e8
        };

        Serial( Port port )
            : _port( static_cast< std::underlying_type_t< Port > >( port ) )
        {}

        Status init();
        char read();

        Status putchar( char c );
        std::size_t print( const char * str, std::size_t len );

		int received();
		int is_transmit_empty();

        void eot();

    private:
        int _port;
    };

    struct VGA {
        enum Color {
            BLACK         = 0,
            BLUE          = 1,
            GREEN         = 2,
            CYAN          = 3,
            RED           = 4,
            MAGENTA       = 5,
            BROWN         = 6,
            LIGHT_GREY    = 7,
            DARK_GREY     = 8,
            LIGHT_BLUE    = 9,
            LIGHT_GREEN   = 10,
            LIGHT_CYAN    = 11,
            LIGHT_RED     = 12,
            LIGHT_MAGENTA = 13,
            LIGHT_BROWN   = 14,
            WHITE         = 15,
        };

        explicit VGA( uint16_t * buffer ) noexcept
            : row( 0 ), column( 0 ), color( make_color( LIGHT_GREEN, BLACK ) ), buffer( buffer )
        {};

        static const int width { 80 };
        static const int height { 24 };

        constexpr static uint8_t make_color( const Color fg, const Color bg ) noexcept {
            return fg | bg << 4;
        }

        void set_color( Color c ) noexcept { color = c; }

        void clear() noexcept;

        void init() noexcept;

        void set_cursor( uint8_t x, uint8_t y ) noexcept;

        friend VGA& operator<<( VGA &vga, const char *str );
        friend VGA& operator<<( VGA &vga, char c );
    private:
        void write( char c ) noexcept;
        void write( const char* data ) noexcept;
        void put( char c, uint8_t x, uint8_t y );

        size_t row;
        size_t column;
        uint8_t color;
        uint16_t *buffer;
    };

    inline VGA& operator<<( VGA &vga, const char *str ) {
        vga.write( str );
        return vga;
    }

    inline VGA& operator<<( VGA &vga, char c ) {
        vga.write( c );
        return vga;
    }

} // namespace kernel::dev
