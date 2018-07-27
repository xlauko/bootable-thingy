#pragma once

#include <type_traits>
#include <cstdint>
#include <stdlib.h>

using byte = uint8_t;

/* Type traits */
// requires operator==
struct Eq { };

template< typename Trait, typename Ret, typename T >
using HasTrait = typename std::enable_if< std::is_base_of< Trait, T >::value, Ret >::type;

template< typename T >
using IsEq = HasTrait< Eq, bool, T >;

template< typename T >
IsEq< T > operator!=( const T &a, const T &b ) { return !(a == b); }

// requires operator <
struct Ord : Eq { };

template< typename T >
using IsOrd = HasTrait< Ord, bool, T >;

template< typename T >
IsOrd< T > operator>( const T &a, const T &b ) { return b < a; }

template< typename T >
IsOrd< T > operator>=( const T &a, const T &b ) { return !(a < b); }

template< typename T >
IsOrd< T > operator<=( const T &a, const T &b ) { return !(b < a); }

template< typename T >
IsOrd< T > operator==( const T &a, const T &b ) {
    return !(a < b) && !(b < a);
}

template< typename T >
auto operator<( const T &a, const T& b )
    -> decltype( a.as_tuple(), IsOrd< T >() )
{
    return a.as_tuple() < b.as_tuple();
}

using Comparable = Ord;

struct Unit : Ord {
    bool operator<( Unit ) const { return false; }
    bool operator==( Unit ) const { return true; }
};

/* typesafe ofset computation */

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

/* Status */

enum class Status {
    success,
    failure
};
