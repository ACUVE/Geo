#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "compier_attribute.hpp"

namespace chash_detail
{

template< typename INT >
static
constexpr
INT rol3( INT val ){
	static_assert( std::is_unsigned< INT >::value, "Rotate Left only makes sense for unsigned types" );
	return (val << 3) | (val >> (sizeof( INT ) * CHAR_BIT - 3));
}

template< typename T >
struct chash
{
	typedef T argument_type;
	typedef std::size_t result_type;
	result_type operator()( argument_type const &s ) const
	{
		return std::hash< T >{}( s );
	}
};

template< typename... Type >
struct chash< std::tuple< Type... > >
{
	typedef std::tuple< Type... > argument_type;
	typedef std::size_t result_type;
	result_type operator()( argument_type const &s ) const
	{
		return hasher< 0u >( s );
	}

	template< std::size_t I >
	static
	FORCE_INLINE
	std::enable_if_t<
		(I < sizeof...( Type )),
		result_type
	> hasher( argument_type const &s )
	{
		return chash< std::decay_t< decltype( std::get< I >( s ) ) > >{}( std::get< I >( s ) ) ^ rol3( hasher< I + 1 >( s ) );
	}
	template< std::size_t I >
	static
	FORCE_INLINE
	std::enable_if_t<
		(I >= sizeof...( Type )),
		result_type
	> hasher( argument_type const &s )
	{
		return 0u;
	}
};

template<>
struct chash< std::vector< unsigned int > >
{
	typedef std::vector< unsigned int > argument_type;
	typedef std::size_t result_type;
	result_type operator()( argument_type const &v ) const
	{
		result_type h = 0u;
		for( auto &&i : v )
		{
			h = rol3( h ) ^ i;
		}
		return h;
	}
};

template< typename T, typename U >
using unordered_map = std::unordered_map< T, U, chash< T > >;
template< typename T, typename U >
using unordered_multimap = std::unordered_multimap< T, U, chash< T > >;
template< typename T >
using unordered_set = std::unordered_set< T, chash< T > >;
template< typename T >
using unordered_multiset = std::unordered_multiset< T, chash< T > >;
} // namespace chash

template< typename... T >
using chash = chash_detail::chash< T... >;

template< typename T, typename U >
using cunordered_map = std::unordered_map< T, U, chash< T > >;
template< typename T, typename U >
using cunordered_multimap = std::unordered_multimap< T, U, chash< T > >;
template< typename T >
using cunordered_set = std::unordered_set< T, chash< T > >;
template< typename T >
using cunordered_multiset = std::unordered_multiset< T, chash< T > >;
