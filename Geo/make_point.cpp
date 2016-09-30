#include <array>
#include <unordered_map>
#include <tuple>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cmath>
#include <numeric>
#include <limits>
#include <memory>
#include <iostream>
#include <thread>
#include "make_point.hpp"

namespace std
{
	template<>
	struct hash< std::tuple< unsigned int, unsigned int > >
	{
		typedef std::tuple< unsigned int, unsigned int > argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const
		{
			result_type const h1 = std::hash< unsigned int >{}( std::get< 0 >( s ) );
			result_type const h2 = std::hash< unsigned int >{}( std::get< 0 >( s ) );
			return h1 ^ (h2 << 1); // or use boost::hash_combine
		}
	};
}

template< typename T >
constexpr T const_sqrt_aux( T s, T x, T prev )
{
	return x != prev ? const_sqrt_aux( s, ( x + s / x ) / static_cast< T >( 2.0 ), x ) : x;
}
template< typename T >
constexpr T const_sqrt( T s )
{
	return const_sqrt_aux< T >( s, s / static_cast< T >( 2.0 ), s );
}
template< typename T >
static
T hypot( T const x, T const y, T const z )
{
	return std::sqrt( x * x + y * y + z * z );
}

// 参考文献: http://atali.jp/blog/2014/08/geodesicdome/

// 正20面体の基本情報：
//   面：20面，辺30本，頂点12個

// 円周率
constexpr float PI = 3.14159265358979f;
// 黄金比
constexpr float GR = (1 + const_sqrt( 5.0f )) / 2;

// 正20面体の座標
constexpr float regular_icosahedron_point[ 12 * 3 ] = 
{
	+1.0f,   +GR,  0.0f,
	-1.0f,   +GR,  0.0f,
	+1.0f,   -GR,  0.0f,
	-1.0f,   -GR,  0.0f,
	 0.0f, +1.0f,   +GR,
	 0.0f, -1.0f,   +GR,
	 0.0f, +1.0f,   -GR,
	 0.0f, -1.0f,   -GR,
	  +GR,  0.0f, +1.0f,
	  +GR,  0.0f, -1.0f,
	  -GR,  0.0f, +1.0f,
	  -GR,  0.0f, -1.0f,
};
constexpr unsigned int regular_icosahedron_point_idxs[ 20 * 3 ] =
{
	0, 1, 4,
	0, 4, 8,
	0, 8, 9,
	0, 9, 6,
	0, 6, 1,
	
	1, 10, 4,
	4, 5, 8,
	8, 2, 9,
	9, 7, 6,
	6, 11, 1,
	
	4, 10, 5,
	8, 5, 2,
	9, 2, 7,
	6, 7, 11,
	1, 11, 10,

	10, 3, 5,
	5, 3, 2,
	2, 3, 7,
	7, 3, 11,
	11, 3, 10
};
constexpr float regular_icosahedron_r = const_sqrt( 1.0f * 1.0f + GR * GR );

void make_geodesic_dome_point( unsigned int const level, std::vector< float > &point, std::vector< unsigned int > &index )
{
	// levelが1上がる毎に面数は4倍になる
	unsigned int const mensuu = (20 << (2 * level));
	// 辺数 = 面数 * 3 / 2
	unsigned int const hensuu = mensuu * 3 / 2;
	// 点数 = 面数 / 2 + 2
	unsigned int const tensuu = mensuu / 2 + 2;
	point.reserve( tensuu * 3 );
	point.assign( std::cbegin( regular_icosahedron_point ), std::cend( regular_icosahedron_point ) );
	index.reserve( mensuu * 3 );
	index.assign( std::cbegin( regular_icosahedron_point_idxs ), std::cend( regular_icosahedron_point_idxs ) );
	if( level > 0 )
	{
		std::decay_t< decltype( index ) > index_tmp;
		index_tmp.reserve( index.size() );
		for( auto cl = 0u; cl < level; ++cl )
		{
			auto &o_i = cl % 2 == 0 ? index_tmp : index;
			auto &i_i = cl % 2 == 0 ? index : index_tmp;
			std::unordered_map< std::tuple< unsigned int, unsigned int >, unsigned int > m;
			auto getpos = [ & ]( unsigned int a, unsigned int b )
			{
				auto const ind = std::minmax( a, b );
				auto const ret_it = m.find( ind );
				if( ret_it != m.end() ) return ret_it->second;
				unsigned int const pind = static_cast< unsigned int >( point.size() / 3 );
				auto const *p1 = &point[ a * 3 ], *p2 = &point[ b * 3 ];
				auto const x = p1[ 0 ] + p2[ 0 ], y = p1[ 1 ] + p2[ 1 ], z = p1[ 2 ] + p2[ 2 ];
				auto const r = std::sqrt( x * x + y * y + z * z ) / regular_icosahedron_r;
				auto const addpoint = { x / r, y / r, z / r };
				std::copy( std::cbegin( addpoint ), std::cend( addpoint ), std::back_inserter( point ) );
				m[ ind ] = pind;
				return pind;
			};
			o_i.resize( 0 );
			for( auto i = 0u; i + 2 < i_i.size(); i += 3 )
			{
				auto const * const p = &i_i[ i ];
				auto const i01 = getpos( p[ 0 ], p[ 1 ] );
				auto const i12 = getpos( p[ 1 ], p[ 2 ] );
				auto const i20 = getpos( p[ 2 ], p[ 0 ] );
				auto const addindex = {
					p[ 0 ], i01, i20,
					i01, p[ 1 ], i12,
					i20, i12, p[ 2 ],
					i12, i20, i01,
				};
				std::copy( std::cbegin( addindex ), std::cend( addindex ), std::back_inserter( o_i ) );
			}
		}
		if( level % 2 != 0 )
		{
			index = std::move( index_tmp );
		}
	}
}
std::tuple< std::vector< float >, std::vector< unsigned int > > make_geodesic_dome_point( unsigned int const level )
{
	std::vector< float > f;
	std::vector< unsigned int > i;
	make_geodesic_dome_point( level, f, i );
	return std::make_tuple( std::move( f ) , std::move( i ) );
}

// Pentakis Dodecahedron
// 参考： http://dmccooey.com/polyhedra/PentakisDodecahedron.txt
// 参考： http://dmccooey.com/polyhedra/PentakisDodecahedron.html
// [5]-Vertex Radius (12): 9*sqrt(65+22*sqrt(5))/38 ~ 2.53092686862706152146
// [6]-Vertex Radius (20): 3*sqrt(3)/2              ~ 2.5980762113533159403

constexpr float C0 = 3 * (const_sqrt( 5.0f ) - 1) / 4;
constexpr float C1 = 9 * (9 + const_sqrt( 5.0f )) / 76;
constexpr float C2 = 9 * (7 + 5 * const_sqrt( 5.0f )) /76;
constexpr float C3 = 3 * (1 + const_sqrt( 5.0f )) / 4;

constexpr float regular_pentakis_dodecahedron_point[] =
{
	 0.0,   C0,   C3,
	 0.0,   C0,  -C3,
	 0.0,  -C0,   C3,
	 0.0,  -C0,  -C3,
	  C3,  0.0,   C0,
	  C3,  0.0,  -C0,
	 -C3,  0.0,   C0,
	 -C3,  0.0,  -C0,
	  C0,   C3,  0.0,
	  C0,  -C3,  0.0,
	 -C0,   C3,  0.0,
	 -C0,  -C3,  0.0,
	  C1,  0.0,   C2,
	  C1,  0.0,  -C2,
	 -C1,  0.0,   C2,
	 -C1,  0.0,  -C2,
	  C2,   C1,  0.0,
	  C2,  -C1,  0.0,
	 -C2,   C1,  0.0,
	 -C2,  -C1,  0.0,
	 0.0,   C2,   C1,
	 0.0,   C2,  -C1,
	 0.0,  -C2,   C1,
	 0.0,  -C2,  -C1,
	 1.5,  1.5,  1.5,
	 1.5,  1.5, -1.5,
	 1.5, -1.5,  1.5,
	 1.5, -1.5, -1.5,
	-1.5,  1.5,  1.5,
	-1.5,  1.5, -1.5,
	-1.5, -1.5,  1.5,
	-1.5, -1.5, -1.5,
};
constexpr unsigned int regular_pentakis_dodecahedron_point_idxs[] =
{
	12,  0,  2,
	12,  2, 26,
	12, 26,  4,
	12,  4, 24,
	12, 24,  0,
	13,  3,  1,
	13,  1, 25,
	13, 25,  5,
	13,  5, 27,
	13, 27,  3,
	14,  2,  0,
	14,  0, 28,
	14, 28,  6,
	14,  6, 30,
	14, 30,  2,
	15,  1,  3,
	15,  3, 31,
	15, 31,  7,
	15,  7, 29,
	15, 29,  1,
	16,  4,  5,
	16,  5, 25,
	16, 25,  8,
	16,  8, 24,
	16, 24,  4,
	17,  5,  4,
	17,  4, 26,
	17, 26,  9,
	17,  9, 27,
	17, 27,  5,
	18,  7,  6,
	18,  6, 28,
	18, 28, 10,
	18, 10, 29,
	18, 29,  7,
	19,  6,  7,
	19,  7, 31,
	19, 31, 11,
	19, 11, 30,
	19, 30,  6,
	20,  8, 10,
	20, 10, 28,
	20, 28,  0,
	20,  0, 24,
	20, 24,  8,
	21, 10,  8,
	21,  8, 25,
	21, 25,  1,
	21,  1, 29,
	21, 29, 10,
	22, 11,  9,
	22,  9, 26,
	22, 26,  2,
	22,  2, 30,
	22, 30, 11,
	23,  9, 11,
	23, 11, 31,
	23, 31,  3,
	23,  3, 27,
	23, 27,  9,	
};
void make_regular_pentakis_dodecahedron_point( unsigned int const level, std::vector< float > &point, std::vector< unsigned int > &index )
{
	point.assign( std::cbegin( regular_pentakis_dodecahedron_point ), std::cend( regular_pentakis_dodecahedron_point ) );
	index.assign( std::cbegin( regular_pentakis_dodecahedron_point_idxs ), std::cend( regular_pentakis_dodecahedron_point_idxs ) );

	for( auto i = 0u; i + 2 < std::size( point ); i += 3 )
	{
		auto const r = hypot( point[ i + 0 ], point[ i + 1 ], point[ i + 2 ] );
		point[ i + 0 ] /= r, point[ i + 1 ] /= r, point[ i + 2 ] /= r;
	}
	if( level > 0 )
	{
		partition_polygon_on_ball( level, point, index );
	}
}
std::tuple< std::vector< float >, std::vector< unsigned int > > make_regular_pentakis_dodecahedron_point( unsigned int const level )
{
	std::vector< float > f;
	std::vector< unsigned int > i;
	make_regular_pentakis_dodecahedron_point( level, f, i );
	return std::make_tuple( std::move( f ), std::move( i ) );
}

// pointの中身は半径1の球面上の点に限る
void partition_polygon_on_ball( unsigned int const level, std::vector< float > &point, std::vector< unsigned int > &index )
{
	if( level <= 0 ) return;
	using Index_Type = std::decay_t< decltype( index[ 0 ] ) >;
	using Coord_Type = std::decay_t< decltype( point[ 0 ] ) >;
	std::unordered_map< std::tuple< Index_Type, Index_Type >, Index_Type > partmap;
	std::vector< Index_Type > retindex;
	auto get_great_circle_func = [ & ]( Index_Type const from, Index_Type const to )
	{
		auto const from_point_index = from * 3;
		auto const * const v1 = &point[ from_point_index ], * const v2 = &point[ to * 3 ];
		auto const dot = v1[ 0 ] * v2[ 0 ] + v1[ 1 ] * v2[ 1 ] + v1[ 2 ] * v2[ 2 ];
		Coord_Type const v3nn[] = { v2[ 0 ] - dot * v1[ 0 ], v2[ 1 ] - dot * v1[ 1 ], v2[ 2 ] - dot * v1[ 2 ] };
		auto const v3nnlen = hypot( v3nn[ 0 ], v3nn[ 1 ], v3nn[ 2 ] );
		std::array< Coord_Type, 3 > const v3 = { v3nn[ 0 ] / v3nnlen, v3nn[ 1 ] / v3nnlen, v3nn[ 2 ] / v3nnlen };
		auto const v1tov2rad = std::acos( dot );
		return [ from_point_index, v3, v1tov2rad, &point ]( Coord_Type const ratio )
		{
			// pointの中身のメモリ上の位置が移動するので from_point_index で位置を保持
			auto const * const v1 = &point[ from_point_index ];
			auto const theta = v1tov2rad * ratio;
			auto const thetasin = std::sin( theta ), thetacos = std::cos( theta );
			std::array< Coord_Type, 3 > ret = { v1[ 0 ] * thetacos + v3[ 0 ] * thetasin, v1[ 1 ] * thetacos + v3[ 1 ] * thetasin, v1[ 2 ] * thetacos + v3[ 2 ] * thetasin };
			return ret;
		};
	};
	auto add_great_circle_point = [ & ]( Index_Type const a, Index_Type const b, Index_Type const num )
	{
		auto const retindex = static_cast< Index_Type >( std::size( point ) / 3 );
		auto const gc = get_great_circle_func( a, b );
		auto const pd = 1 / static_cast< Coord_Type >( num + 1 );
		for( auto i = 1u; i - 1 < num; ++i ){
			auto const arr = gc( i * pd );
			point.insert( point.end(), std::cbegin( arr ), std::cend( arr ) );
		}
		return retindex;
	};
	auto getindex = [ & ]( Index_Type const a, Index_Type const b )
	{
		auto const minmax = std::minmax( a, b );
		auto const d = std::get< 0 >( minmax ) == a ? true : false;
		auto it = partmap.find( minmax );
		if( it != partmap.end() ) return std::make_tuple( it->second, d );
		auto const ret = add_great_circle_point( std::get< 0 >( minmax ), std::get< 1 >( minmax ), level );
		partmap[ minmax ] = ret;
		return std::make_tuple( ret, d );
	};
	for( auto i = 0u; i + 2 < std::size( index ); i += 3 )
	{
		auto const ia = index[ i + 0 ], ib = index[ i + 1 ], ic = index[ i + 2 ];
		auto const ab = getindex( ia, ib ), ac = getindex( ia, ic ), bc = getindex( ib, ic );
		auto const abf = std::get< 1 >( ab ), acf = std::get< 1 >( ac ), bcf = std::get< 1 >( bc );
		auto const abi = std::get< 0 >( ab ) + ( abf ? 0 : level - 1 ), aci = std::get< 0 >( ac ) + ( acf ? 0 : level - 1 ), bci = std::get< 0 >( bc ) + ( bcf ? 0 : level - 1 );
		#define CONV_INDEX( abcv, ind ) ( ( abcv ## f ) ? ( abcv ## i ) + ( ind ) : ( abcv ## i ) - ( ind ) )
		retindex.insert( retindex.end(), { ia, CONV_INDEX( ab, 0 ), CONV_INDEX( ac, 0 ) } );
		Index_Type crr_index_first = 0u;
		for( auto j = 1u; j < level; ++j )
		{
			auto const next_index_first = add_great_circle_point( CONV_INDEX( ab, j ), CONV_INDEX( ac, j ), j );
			retindex.insert( retindex.end(), { CONV_INDEX( ab, j - 1 ), CONV_INDEX( ab, j ), next_index_first } );
			retindex.insert( retindex.end(), { CONV_INDEX( ab, j - 1 ), next_index_first, (j == 1) ? CONV_INDEX( ac, j - 1 ) : crr_index_first } );
			for( auto k = 1u ; k < j; ++k )
			{
				retindex.insert( retindex.end(), { crr_index_first + k - 1, next_index_first + k - 1, next_index_first + k } );
				retindex.insert( retindex.end(), { crr_index_first + k - 1, next_index_first + k, (k == j - 1) ? CONV_INDEX( ac, j - 1 ) : crr_index_first + k } );
			}
			retindex.insert( retindex.end(), { CONV_INDEX( ac, j - 1 ), next_index_first + j - 1, CONV_INDEX( ac, j ) } );
			crr_index_first = next_index_first;
		}
		retindex.insert( retindex.end(), { CONV_INDEX( ab, level - 1 ), ib, CONV_INDEX( bc, 0 ) } );
		retindex.insert( retindex.end(), { CONV_INDEX( ab, level - 1 ), CONV_INDEX( bc, 0 ), (level == 1) ? CONV_INDEX( ac, 0 ) : crr_index_first } );
		for( auto k = 1u; k < level; ++k )
		{
			retindex.insert( retindex.end(), { crr_index_first + k - 1, CONV_INDEX( bc, k - 1 ), CONV_INDEX( bc, k ) } );
			retindex.insert( retindex.end(), { crr_index_first + k - 1, CONV_INDEX( bc, k ), (k == level - 1) ? CONV_INDEX( ac, level - 1 ) : crr_index_first + k } );
		}
		retindex.insert( retindex.end(), { CONV_INDEX( ac, level - 1 ), CONV_INDEX( bc, level - 1 ), ic } );
		#undef CONV_INDEX
	}
	index = std::move( retindex );
}

void euclidean_to_theta_phi_of_poler_for_draw( std::vector< float > const &point, std::vector< unsigned int > const &index, std::vector< float > &poler_theta_phi, std::vector< unsigned int > &poler_index, std::vector< unsigned int > *poler_point_to_point )
{
	using Index_Type = std::decay_t< decltype( poler_index[ 0 ] ) >;
	using Coord_Type = std::decay_t< decltype( poler_theta_phi[ 0 ] ) >;
	poler_theta_phi.resize( std::size( point ) / 3 * 2 );
	poler_theta_phi.reserve( std::size( poler_theta_phi ) * 3 );
	if( poler_point_to_point )
	{
		poler_point_to_point->resize( std::size( poler_theta_phi ) / 2 );
		poler_point_to_point->reserve( std::size( *poler_point_to_point ) * 3 );
		std::iota( std::begin( *poler_point_to_point ), std::end( *poler_point_to_point ), 0u );
	}
	poler_index = index;
	poler_index.reserve( std::size( poler_index ) * 2 );
	auto const index_size = std::size( index );
	for( auto i = 0u, j = 0u; i + 1 < poler_theta_phi.size(); i += 2, j += 3 )
	{
		poler_theta_phi[ i + 0 ] = std::atan2( point[ j + 1 ], point[ j + 0 ] );
		poler_theta_phi[ i + 1 ] = std::atan2( point[ j + 2 ], std::hypot( point[ j + 0 ], point[ j + 1 ] ) );
	}
	std::unordered_map< Index_Type, Index_Type > plus_hantai_index, minus_hantai_index;
	auto get_offset_index = [ & ]( Index_Type const index, Coord_Type const diff, std::unordered_map< Index_Type, Index_Type > &hantai_index )
	{
		auto const it = hantai_index.find( index );
		if( it != hantai_index.end() ) return it->second;
		auto const ret = static_cast< Index_Type >( std::size( poler_theta_phi ) / 2 );
		auto const ptp = &poler_theta_phi[ index * 2 ];
		poler_theta_phi.push_back( ptp[ 0 ] + diff );
		poler_theta_phi.push_back( ptp[ 1 ] );
		hantai_index[ index ] = ret;
		if( poler_point_to_point )
		{
			poler_point_to_point->push_back( index );
		}
		return ret;
	};
	auto get_plus_hantai_index = [ & ]( Index_Type const index )
	{
		return get_offset_index( index, 2 * PI, plus_hantai_index );
	};
	auto get_minus_hantai_index = [ & ]( Index_Type const index )
	{
		return get_offset_index( index, -2 * PI, minus_hantai_index );
	};
	for( auto i = 0u; i + 2 < index_size; i += 3 )
	{
		auto const p1 = &poler_theta_phi[ poler_index[ i + 0 ] * 2 ], p2 = &poler_theta_phi[ poler_index[ i + 1 ] * 2 ], p3 = &poler_theta_phi[ poler_index[ i + 2 ] * 2 ];
		auto const v1x = p2[ 0 ] - p1[ 0 ], v1y = p2[ 1 ] - p1[ 1 ], v2x = p3[ 0 ] - p2[ 0 ], v2y = p3[ 1 ] - p2[ 1 ];
		auto const cross = v1x * v2y - v1y * v2x;
		if( cross >= 0 ) continue;
		// if( cross <= 0 ) continue;
		auto const pa = { p1, p2, p3 };
		auto const minmax = std::minmax_element( std::cbegin( pa ), std::cend( pa ), []( auto const &a, auto const &b ){ return a[ 0 ] < b[ 0 ]; } );
		auto const minp = *minmax.first, maxp = *minmax.second;
		auto const midp = *std::find_if( std::cbegin( pa ), std::cend( pa ), [ & ]( auto const &v ){ return minp != v && maxp != v; } );
		auto const dmaxmid = maxp[ 0 ] - midp[ 0 ], dmidmin = midp[ 0 ] - minp[ 0 ];
		if( dmaxmid > dmidmin  )
		{
			// minの方がmidに近い
			for( auto j = 0u; j < 3; ++j )
			{
				auto const idx = poler_index[ i + j ];
				if( &poler_theta_phi[ idx * 2 ] == maxp )
				{
					poler_index.push_back( idx );
					poler_index[ i + j ] = get_minus_hantai_index( idx );
				}
				else
				{
					poler_index.push_back( get_plus_hantai_index( idx ) );
				}
			}
		}
		else
		{
			// maxの方がmidに近い
			for( auto j = 0u; j < 3; ++j )
			{
				auto const idx = poler_index[ i + j ];
				if( &poler_theta_phi[ idx * 2 ] == minp )
				{
					poler_index.push_back( idx );
					poler_index[ i + j ] = get_plus_hantai_index( idx );
				}
				else
				{
					poler_index.push_back( get_minus_hantai_index( idx ) );
				}
			}
		}
	}
	poler_theta_phi.shrink_to_fit();
	poler_index.shrink_to_fit();
	if( poler_point_to_point ) poler_point_to_point->shrink_to_fit();
}
std::tuple< std::vector< float >, std::vector< unsigned int > > euclidean_to_theta_phi_of_poler_for_draw( std::vector< float > const &point, std::vector< unsigned int > const &index, std::vector< unsigned int > *poler_point_to_point )
{
	std::vector< float > f;
	std::vector< unsigned int > u;
	euclidean_to_theta_phi_of_poler_for_draw( point, index, f, u, poler_point_to_point );
	return std::make_tuple( std::move( f ), std::move( u ) );
}

std::vector< std::uint8_t > make_texture( unsigned int const width, unsigned int const height, std::vector< unsigned int > const &num, std::vector< float > const &point, float const dot_size, float const claster_size_in_rad, bool const multi_thread )
{
	if( std::size( num ) != std::size( point ) / 3 )
	{
		throw std::logic_error( __func__ );
	}
	using Point_Type = std::decay_t< decltype( point[ 0 ] ) >;

	std::vector< std::vector< std::array< Point_Type, 3 > > > dotp( std::size( point ) / 3);
	auto const cssin = std::sin( claster_size_in_rad ), cscos = std::cos( claster_size_in_rad );
	for( auto i = 0u, j = 0u; i < std::size( dotp ); ++i, j += 3 )
	{
		// ドットクラスタに含まれる点の数
		auto const numi = num[ i ];
		if( numi == 0 ) continue;
		auto &dotpi = dotp[ i ];
		dotpi.resize( numi );
		auto const r = hypot( point[ j + 0 ], point[ j + 1 ], point[ j + 2 ] );
		// ドットクラスターの位置ベクトル
		auto const x = point[ j + 0 ] / r, y = point[ j + 1 ] / r, z = point[ j + 2 ] / r;
		if( numi == 1 )
		{
			// ドットクラスターのドットが1つの場合
			dotpi[ 0 ][ 0 ] = x, dotpi[ 0 ][ 1 ] = y, dotpi[ 0 ][ 2 ] = z;
		}
		else
		{
			// ドットクラスターの極座標
			// auto const theta = std::asin( z ), phi = std::atan2( y, x );
			auto const theta = std::atan2( y, x ), phi = std::asin( z );
			auto const theta_sin = std::sin( theta ), theta_cos = std::cos( theta );
			auto const phi_sin = std::sin( phi ), phi_cos = std::cos( phi );
			// ドットクラスターの位置で円に接し，かつ，だいたい(0,0,1)の方向を向いているベクトル
			auto const upx = -theta_cos * phi_sin, upy = -theta_sin * phi_sin, upz = phi_cos;
			// 上の2つのベクトルに直交するベクトル
			auto const hix = y * upz - z * upy, hiy = z * upx - x * upz, hiz = x * upy - y * upx;
			Point_Type diff_gamma = 2 * PI / numi;
			for( auto j = 0u; j < numi; ++j )
			{
				auto const gamma = j * diff_gamma;
				auto const gamma_sin = std::sin( gamma ), gamma_cos = std::cos( gamma );
				// ドットクラスター内の1つの点の方向ベクトル
				auto const dox = gamma_cos * upx + gamma_sin * hix, doy = gamma_cos * upy + gamma_sin * hiy, doz = gamma_cos * upz + gamma_sin * hiz;
				// ドットクラスター内の1つの点の位置ベクトル
				auto const dx = cscos * x + cssin * dox, dy = cscos * y + cssin * doy, dz = cscos * z + cssin * doz;
				dotpi[ j ][ 0 ] = dx, dotpi[ j ][ 1 ] = dy, dotpi[ j ][ 2 ] = dz;
			}
		}
	}
	std::vector< std::uint8_t > ret( width * height * 3, std::numeric_limits< std::uint8_t >::max() );
	auto const dosome = [ & ]( unsigned int const s, unsigned int const e )
	{
		for( auto i = s; i < e; ++i )
		{
			for( auto j = 0u; j < height; ++j )
			{
				auto const theta = ( static_cast< Point_Type >( i ) - width / 2 ) / width * 2 * PI;
				auto const phi = ( static_cast< Point_Type >( j ) - height / 2) / height * PI;
				auto const theta_sin = std::sin( theta ), theta_cos = std::cos( theta );
				auto const phi_sin = std::sin( phi ), phi_cos = std::cos( phi );
				auto const x = theta_cos * phi_cos, y = theta_sin * phi_cos, z = phi_sin;
				for( auto &&v : dotp )
				{
					for( auto &&p : v )
					{
						if( hypot( p[ 0 ] - x, p[ 1 ] - y, p[ 2 ] - z ) < dot_size )
						{
							auto const index = j * width + i;
							ret[ index * 3 + 0 ] /= 2;
							ret[ index * 3 + 1 ] /= 2;
							ret[ index * 3 + 2 ] /= 2;
							// goto out;
						}
					}
				}
				out:;
			}
		}
	};
	auto const hwc = std::thread::hardware_concurrency();
	auto const cpunum = multi_thread ? (hwc <= 0 ? 1u : hwc) : 1u;
	std::vector< std::thread > ths;
	ths.reserve( cpunum - 1 );
	auto const q = width / cpunum, r = width % cpunum;
	auto s = 0u, e = q + (0 < r);
	for( auto i = 0u; i < cpunum - 1; ++i, s = e, e += q + (i < r) )
	{
		ths.emplace_back( dosome, s, e );
	}
	dosome( s, e );
	if( !ths.empty() ) for( auto &&t : ths ) t.join();
	return std::move( ret );
}