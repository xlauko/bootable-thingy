#pragma once

#include <cstddef>
#include <cstdint>

namespace hw {
    class VGA {
    public:
        explicit VGA() noexcept;

        static const int width { 80 };
        static const int height { 25 };

        void write( const char c ) noexcept;
        void write( const char* data ) noexcept;

        void clear() noexcept;

        void set_cursor( uint8_t x, uint8_t y ) noexcept;
        // static VGA& get() { static VGA vga; return vga; }
    private:

        void put( char c, uint8_t x, uint8_t y );

        size_t row;
        size_t column;
        char *buffer;
    };

} // namespace hw
