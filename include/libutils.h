#pragma once

#include <type_traits>
#include <stddef.h>

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
    using byte_type = adopt_cv_t< char, T >;
    return reinterpret_cast< T* >( reinterpret_cast< byte_type* >( ptr ) + offset );
}
