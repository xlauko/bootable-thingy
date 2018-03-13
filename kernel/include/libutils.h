#pragma once

#include <type_traits>
#include <cstddef>

extern "C" {

/*  The number of columns. */
#define COLUMNS                 80
/*  The number of lines. */
#define LINES                   24
/*  The attribute of an character. */
#define ATTRIBUTE               7

/*  Variables. */
/*  Save the X position. */
static int xpos;
/*  Save the Y position. */
static int ypos;

/*  Point to the video memory. */
static volatile unsigned char *video = (unsigned char*) 0xB8000;

void clear (void);
void itoa (char *buf, int base, int d);
void putchar( int c );
void puts( const char *str );
void printf (const char *format, ...);

}

template < typename dst, typename src >
struct adopt_const {
    using type = std::conditional_t< std::is_const_v< src >, std::add_const_t< dst >, dst >;
};

template < typename dst, typename src >
using adopt_const_t = typename adopt_const< dst, src >::type;

template < typename dst, typename src>
struct adopt_volatile {
    using type = std::conditional_t< std::is_volatile_v< src >, std::add_volatile_t< dst >, dst >;
};

template < typename dst, typename src>
using adopt_volatile_t = typename adopt_volatile< dst, src >::type;

template < typename dst, typename src >
struct adopt_cv {
    using type = adopt_const_t< adopt_volatile_t< dst, src >, src >;
};

template < typename dst, typename src >
using adopt_cv_t = typename adopt_cv< dst, src >::type;

template < typename T>
inline constexpr T* add_offset( T* ptr, unsigned long offset ) noexcept {
    using byte_type = adopt_cv_t< std::byte, T >;
    return reinterpret_cast< T* >( reinterpret_cast< byte_type* >( ptr ) + offset );
}
