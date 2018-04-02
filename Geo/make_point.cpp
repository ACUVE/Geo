#include <array>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cmath>
#include <numeric>
#include <limits>
#include <climits>
#include <memory>
#include <iostream>
#include <thread>
#include <random>
#include <queue>
#include <stack>
#include <cassert>
#include "make_point.hpp"
#include "vector.hpp"

using Vector = kato::vectorf;


template< typename T >
constexpr
T const_sqrt_aux( T s, T x, T prev )
{
	return x != prev ? const_sqrt_aux( s, ( x + s / x ) / static_cast< T >( 2.0 ), x ) : x;
}
template< typename T >
constexpr
T const_sqrt( T s )
{
	return const_sqrt_aux< T >( s, s / static_cast< T >( 2.0 ), s );
}
template< typename T >
static
T hypot( T const x, T const y, T const z )
{
	return std::sqrt( x * x + y * y + z * z );
}
static
auto thread_num()
{
#if _DEBUG
	return 1u;
#else
	static std::atomic< unsigned int > num{ 0 };
	unsigned int ret = num.load();
	if( ret == 0 )
	{
		ret = std::thread::hardware_concurrency();
		if( ret == 0 ) ret = 1u;
		unsigned int exp = 0u;
		num.compare_exchange_strong( exp, ret );
	}
	return ret;
#endif
}

cunordered_map< std::tuple< unsigned int, unsigned int >, unsigned int > make_vmap( std::vector< unsigned int > const &index )
{
	cunordered_map< std::tuple< unsigned int, unsigned int >, unsigned int > vmap;
	for( auto i = 0u; i + 2 < std::size( index ); i += 3 )
	{
		auto const ii0 = index[ i + 0 ], ii1 = index[ i + 1 ], ii2 = index[ i + 2 ];
		vmap[ std::make_tuple( ii0, ii1 ) ] = ii2;
		vmap[ std::make_tuple( ii1, ii2 ) ] = ii0;
		vmap[ std::make_tuple( ii2, ii0 ) ] = ii1;
	}
	return std::move( vmap );
}
cunordered_map< std::tuple< unsigned int, unsigned int >, unsigned int > make_map_pindex_to_index( std::vector< unsigned int > const &index )
{
	cunordered_map< std::tuple< unsigned int, unsigned int >, unsigned int > map;
	for( auto i = 0u; i + 2 < std::size( index ); i += 3 )
	{
		auto const ii0 = index[ i + 0 ], ii1 = index[ i + 1 ], ii2 = index[ i + 2 ];
		map[ std::make_tuple( ii0, ii1 ) ] = i;
		map[ std::make_tuple( ii1, ii2 ) ] = i;
		map[ std::make_tuple( ii2, ii0 ) ] = i;
	}
	return std::move( map );
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
	point.assign( std::cbegin( regular_icosahedron_point ), std::cend( regular_icosahedron_point ) );
	index.assign( std::cbegin( regular_icosahedron_point_idxs ), std::cend( regular_icosahedron_point_idxs ) );
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
// [5]-Vertex Radius (12): 9*sqrt(65+22*sqrt(5))/38 ‾ 2.53092686862706152146
// [6]-Vertex Radius (20): 3*sqrt(3)/2              ‾ 2.5980762113533159403

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
	cunordered_map< std::tuple< Index_Type, Index_Type >, Index_Type > partmap;
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

// メモリを食う糞ソース
std::vector< unsigned int > make_claster_impl( unsigned int const point_num, unsigned int const max_num, unsigned int const sep_num, std::vector< std::vector< unsigned int * > > const &check_timing_list, std::vector< std::vector< unsigned int > > const &map, std::atomic< bool > &flag )
{
	std::random_device rd;
	std::mt19937_64 gen( rd() );
	// std::uniform_int_distribution< unsigned int > dist( 1, max_num );
	std::uniform_int_distribution< unsigned int > dist( 1, max_num );

	std::vector< unsigned int > ret_num( point_num );
	std::vector< unsigned int > tmp( sep_num );
	cunordered_set< std::vector< unsigned int > > set;
	while( !flag )
	{
		bool gen_ok = true;
		set.clear();
		for( auto i = 0u; i < point_num; ++i )
		{
			auto const first_num = dist( gen );
			auto const &mi = map[ i ];
			auto j = 0u;
			for( ; j < max_num; ++j )
			{
				auto const v = ret_num[ i ] = ((first_num + j - 1) % max_num) + 1;
				bool same = false;
				for( auto k = 0u; k < mi.size(); ++k )
				{
					if( mi[ k ] >= i ) break;
					if( same = ret_num[ mi[ k ] ] == v ) break;
				}
				if( !same ) break;
			}
			if( !(gen_ok = j != max_num) ) break;
			for( auto const v : check_timing_list[ i ] )
			{
				for( auto k = 0u; k < sep_num; ++k ) tmp[ k ] = ret_num[ v[ k ] ];
				auto it = set.find( tmp );
				if( it != set.end() )
				{
					gen_ok = false;
					break;
				}
				set.insert( tmp );
			}
			if( !gen_ok ) break;
		}
		if( !gen_ok ) continue;
		bool exp = false;
		if( !flag.compare_exchange_strong( exp, true ) ) break;
		return std::move( ret_num );
	}
	return {};
}
void make_claster( unsigned int const point_num, unsigned int const max_num, std::vector< unsigned int > const &index, std::vector< unsigned int > &num )
{
	auto const map = make_adjacency_matrix( index, point_num );
	auto const vmap = make_vmap( index );
	unsigned int argsepnum;
	std::vector< unsigned int > argindex;

	// さんかっけー
	/*
	{
		argsepnum = 3;
		for( auto i = 0u; i + 2 < index.size(); i += 3 )
		{
			argindex.insert( argindex.end(), { index[ i + 0 ], index[ i + 1 ], index[ i + 2 ] } );
			argindex.insert( argindex.end(), { index[ i + 1 ], index[ i + 2 ], index[ i + 0 ] } );
			argindex.insert( argindex.end(), { index[ i + 2 ], index[ i + 0 ], index[ i + 1 ] } );
		}
	}
	// */

	// ほしいやつ
	/*
	{
		argsepnum = 4;
		for( auto const &v : vmap )
		{
			auto const a = std::get< 0 >( v.first ), b = std::get< 1 >( v.first );
			if( a >= b ) continue;
			auto const iri = vmap[ std::make_tuple( a, b ) ], jri = vmap[ std::make_tuple( b, a ) ];
			argindex.insert( argindex.end(), { a, b, iri, jri } );
			argindex.insert( argindex.end(), { b, a, jri, iri } );
		}
	}
	// */

	// ほしいやつその２
	// /*
	{
		argsepnum = 6;
		for( auto i = 0u; i + 2 < index.size(); i += 3 )
		{
			auto const a = index[ i + 0 ], b = index[ i + 1 ], c = index[ i + 2 ];
			auto const u_it = vmap.find( std::make_tuple( b, a ) );
			auto const v_it = vmap.find( std::make_tuple( c, b ) );
			auto const w_it = vmap.find( std::make_tuple( a, c ) );
			if( u_it == vmap.end() || v_it == vmap.end() || w_it == vmap.end() ) continue;
			auto const u = u_it->second, v = v_it->second, w = w_it->second;
			if( u == c || v == a || w == b ) continue;	// 三角形が完全に重なっている
			
			argindex.insert( argindex.end(), { a, b, c, u, v, w } );
			argindex.insert( argindex.end(), { b, c, a, v, w, u } );
			argindex.insert( argindex.end(), { c, a, b, w, u, v } );
		}
	}
	// */

	std::vector< std::vector< unsigned int * > > check_timing_list( point_num );
	for( auto i = 0u; i + argsepnum - 1 < argindex.size(); i += argsepnum )
	{
		auto const m = *std::max_element( &argindex[ i ], &argindex[ i ] + argsepnum );
		check_timing_list[ m ].emplace_back( &argindex[ i ] );
	}

	std::atomic< bool > flag{ false };
	std::vector< std::thread > th;
	auto const tn = thread_num();
	for( auto i = 0u; i < tn; ++i )
	{
		th.emplace_back(
			[ & ]()
			{
				auto v = make_claster_impl( point_num, max_num, argsepnum, check_timing_list, map, flag );
				if( !std::empty(v) ) num = std::move( v );
			}
		);
	}
	for( auto &&t : th ) t.join();
}
std::vector< unsigned int > make_claster( unsigned int const point_num, unsigned int const max_num, std::vector< unsigned int > const &index )
{
	std::vector< unsigned int > u;
	make_claster( point_num, max_num, index, u );
	return std::move( u );
}

void make_texture( unsigned int const width, unsigned int const height, std::vector< unsigned int > const &num, std::vector< float > const &point, float const dot_size, float const claster_size_in_rad, bool const multi_thread, std::vector< std::uint8_t > &texture )
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
	texture.clear();
	texture.resize( width * height * 3, std::numeric_limits< std::uint8_t >::max() );
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
							texture[ index * 3 + 0 ] = 30;
							texture[ index * 3 + 1 ] = 30;
							texture[ index * 3 + 2 ] = 30;
							goto out;
						}
					}
				}
				out:;
			}
		}
	};
	auto const tn = thread_num();
	std::vector< std::thread > ths;
	ths.reserve( tn - 1 );
	auto const q = width / tn, r = width % tn;
	auto s = 0u, e = q + (0 < r);
	for( auto i = 0u; i < tn - 1; ++i, s = e, e += q + (i < r) )
	{
		ths.emplace_back( dosome, s, e );
	}
	dosome( s, e );
	if( !ths.empty() ) for( auto &&t : ths ) t.join();
}
std::vector< std::uint8_t > make_texture( unsigned int const width, unsigned int const height, std::vector< unsigned int > const &num, std::vector< float > const &point, float const dot_size, float const claster_size_in_rad, bool const multi_thread )
{
	std::vector< std::uint8_t > v;	
	make_texture( width, height, num, point, dot_size, claster_size_in_rad, multi_thread, v );
	return std::move( v );
}

void make_dual( std::vector< float > const &point, std::vector< unsigned int > const &index, std::vector< float > &d_point, std::vector< std::vector< unsigned int > > &d_index )
{
	using Index_Type = std::decay_t< decltype( index[ 0 ] ) >;
	using Dual_Index_Type = std::decay_t< decltype( d_index[ 0 ] ) >;
	Index_Type const d_point_offset = static_cast< Index_Type >( std::size( point ) / 3 ), dpo3 = d_point_offset * 3;
	d_point = point;
	d_point.resize( dpo3 + std::size( index ) / 3 * 3, 0.0f );
	d_index.clear();
	d_index.resize( std::size( point ) / 3 );
	for( auto i = 0u; i + 2 < index.size(); i += 3 )
	{
		for( auto j = 0u; j < 3; ++j )
		{
			auto const ind = index[ i + j ] * 3;
			for( auto k = 0u; k < 3; ++k )
			{
				d_point[ dpo3 + i + k ] += point[ ind + k ] / 3;
			}
		}
	}
	auto vmap = make_vmap( index );
	auto pi2i = make_map_pindex_to_index( index );
	for( auto i = 0u; i < std::size( d_index ); ++i )
	{
		auto const f_it = std::find_if(
			vmap.begin(), vmap.end(),
			[ & ]( auto &v )
			{
				return std::get< 0 >( v.first ) == i;
			}
		);
		if( f_it == vmap.end() ) continue;
		auto it = f_it;
		Dual_Index_Type r{ i, d_point_offset + pi2i[ f_it->first ] / 3 };
		r.emplace_back( d_point_offset + pi2i[ std::make_tuple( i, f_it->second ) ] / 3 );
		for( it = vmap.find( std::make_tuple( i, it->second ) ); it != f_it && it != vmap.end(); it = vmap.find( std::make_tuple( i, it->second ) ) )
		{
			auto const addp = d_point_offset + pi2i[ std::make_tuple( i, it->second ) ] / 3;
			r.emplace_back( addp );
		}
		if( it == vmap.end() )
		{
			continue;
			/*
			it = f_it;
			for( it = vmap.find( std::make_tuple( it->second, i ) ); it != vmap.end(); it = vmap.find( std::make_tuple( it->second, i ) ) )
			{
				r.emplace( r.begin() + 1, d_point_offset + pi2i[ std::make_tuple( i , it->second ) ] / 3 );
			}
			*/
		}
		// if( std::size( r ) < 4 ) continue;
		d_index[ i ] = std::move( r );
	}
}
std::tuple< std::vector< float >, std::vector< std::vector< unsigned int > > > make_dual( std::vector< float > const &point, std::vector< unsigned int > const &index )
{
	std::vector< float > fv;
	std::vector< std::vector< unsigned int > > uvv;
	make_dual( point, index, fv, uvv );
	return std::make_tuple( std::move( fv ), std::move( uvv ) );
}

static
Vector calc_single_dcm_cluster_normal( std::vector< float > const &dual_point, std::vector< unsigned int > const &single_dual_index )
{
	auto const dual_index_size = std::size( single_dual_index );
	if( dual_index_size == 0 ) return Vector{};
	Vector const center( &dual_point[ single_dual_index[ 0 ] * 3 ] );
	if( dual_index_size == 1 ) return center;
	Vector sum( 0.0f, 0.0f, 0.0f );
	for( auto i = 1u; i < dual_index_size - 1; ++i )
	{
		Vector const v1( &dual_point[ single_dual_index[ i ] * 3 ] ), v2( &dual_point[ single_dual_index[ i + 1 ] * 3 ] );
		sum += v1.cross_product( v2 );
	}
	sum = sum.normarize();
	return sum;
}

void calc_dcm_cluster_normal( std::vector< float > const &dual_point, std::vector< std::vector< unsigned int > > const &dual_index, std::vector< float > &dcm_normal )
{
	auto const num_of_point = std::size( dual_index );
	dcm_normal.clear();
	dcm_normal.reserve( num_of_point * 3 );
	for( auto i = 0u; i < dual_index.size(); ++i )
	{
		auto const &nv = calc_single_dcm_cluster_normal( dual_point, dual_index[ i ] );
		dcm_normal.insert( dcm_normal.end(), { nv.x, nv.y, nv.z } );
	}
}
std::vector< float > calc_dcm_cluster_normal( std::vector< float > const &dual_point, std::vector< std::vector< unsigned int > > const &dual_index )
{
	std::vector< float > dcm_normal;
	calc_dcm_cluster_normal( dual_point, dual_index, dcm_normal );
	return std::move( dcm_normal );
}

static
std::tuple< Vector, Vector > calc_uv_axes( Vector const normal )
{
	Vector const kariup( 0.0f, 0.0f, 1.0f ); // must be normarized
	Vector const up = normal != kariup ? (kariup - normal.inner_product( kariup ) * normal).normarize() : Vector( 1.0f, 0.0f, 0.0f ); // v座標軸
	Vector const right = up.cross_product( normal ); // u座標軸相当
	return std::make_tuple( right, up );
}

void calc_dual_uv( std::vector< float > const &d_point, std::vector< std::vector< unsigned int > > const &d_index, std::vector< std::vector< float > > &d_uv )
{
	d_uv.clear();
	d_uv.reserve( std::size( d_index ) );
	for( auto i = 0u; i < std::size( d_index ); ++i )
	{
		auto const &dii = d_index[ i ];
		std::vector< float > tuv;
		if( !dii.empty() )
		{
			Vector const center( &d_point[ dii[ 0 ] * 3 ] );
			Vector const normal = calc_single_dcm_cluster_normal( d_point, dii );
			auto const uv = calc_uv_axes( normal );
			
			for( auto j = 0u; j < std::size( dii ); ++j )
			{
				Vector const cvec( &d_point[ dii[ j ] * 3 ] );
				auto const vc = cvec - center;
				auto const u = vc.inner_product( std::get< 0 >( uv ) ), v = vc.inner_product( std::get< 1 >( uv ) );
				tuv.emplace_back( u );
				tuv.emplace_back( v );
			}
		}
		d_uv.emplace_back( std::move( tuv ) );
	}
}
std::vector< std::vector< float > > calc_dual_uv( std::vector< float > const &d_point, std::vector< std::vector< unsigned int > > const &d_index )
{
	std::vector< std::vector< float > > fvv;
	calc_dual_uv( d_point, d_index, fvv );
	return std::move( fvv );
}

void calc_high_resolution_object_uv( std::vector< float > const &point, std::vector< unsigned int > const &index, std::vector< float > const &hires_point, std::vector< unsigned int > const &hires_index, std::vector< unsigned int > &hires_nearest_point_index, std::vector< float > &hires_uv )
{
	auto const hires_index_size = std::size( hires_index );
	auto const hires_triangle_num = hires_index_size / 3;
	// index毎にuvを与える（point毎ではない）
	hires_uv.resize( hires_index_size * 2 );
	// 三角形毎にpointのindexを与える（特に近くない時はstd::numeric_limits< unsigned int >::max()）
	hires_nearest_point_index.resize( hires_triangle_num );
	// 双対を得ておく
	auto dual = make_dual( point, index );
	auto &d_point = std::get< 0 >( dual );
	auto &d_index = std::get< 1 >( dual );
	d_index.erase( std::remove_if( d_index.begin(), d_index.end(), []( auto const &v ){ return v.empty(); } ), d_index.end() );
	auto const d_index_size = std::size( d_index );
	// それぞれの点について，uv座標軸を取得
	std::vector< std::tuple< Vector, Vector > > uv_axes;
	std::vector< Vector > normal_vector;
	normal_vector.reserve( d_index_size );
	uv_axes.reserve( d_index_size );
	for( auto i = 0u; i < d_index_size; ++i )
	{
		auto const normal = calc_single_dcm_cluster_normal( d_point, d_index[ i ] );
		normal_vector.emplace_back( normal );
		uv_axes.emplace_back( calc_uv_axes( normal ) );
	}

	///
	// hiresの法線ベクトルを計算
	auto hires_dual = make_dual( hires_point, hires_index );
	auto &d_hires_point = std::get< 0 >( hires_dual );
	auto &d_hires_index = std::get< 1 >( hires_dual );
	std::vector< Vector > hires_normal_vector;
	hires_normal_vector.reserve( hires_point.size() / 3 * 3 );
	for( auto i = 0u; i < d_hires_index.size(); ++i )
	{
		Vector const ret = calc_single_dcm_cluster_normal( d_hires_point, d_hires_index[ i ] );
		hires_normal_vector.emplace_back( ret );
	}
	///

	// hires側の点の全てについて，一番近い点を計算する
	auto const hires_point_num = std::size( hires_point ) / 3;
	constexpr auto NONE_INDEX = std::numeric_limits< unsigned int >::max();
	std::vector< unsigned int > nearest_point_d_index_index( hires_point_num, NONE_INDEX );
	for( auto i = 0u; i < hires_point_num; ++i )
	{
		auto min_distance = std::numeric_limits< float >::max();
		Vector const hp( &hires_point[ i * 3 ] );
		for( auto j = 0u; j < d_index_size; ++j )
		{
			if( hires_normal_vector[ i ].inner_product( normal_vector[ j ] ) < 0 ) continue;
			Vector const p( &d_point[ d_index[ j ][ 0 ] * 3 ] );
			auto const distance = (p - hp).length();
			if( distance < min_distance )
			{
				min_distance = distance;
				nearest_point_d_index_index[ i ] = j;
			}
		}
	}
	// それぞれの点でUV計算
	for( auto i = 0u; i < hires_triangle_num; ++i )
	{
		auto const fi = i * 3;
		unsigned int const np[] = { nearest_point_d_index_index[ hires_index[ fi + 0 ] ], nearest_point_d_index_index[ hires_index[ fi + 1 ] ], nearest_point_d_index_index[ hires_index[ fi + 2 ] ] };
		if( np[ 0 ] != np[ 1 ] || np[ 1 ] != np[ 2 ] )
		{
			hires_nearest_point_index[ i ] = NONE_INDEX;
			std::fill_n( &hires_uv[ fi * 2 ], 2 * 3, 0.0f );
		}
		else
		{
			auto const np0_di = np[ 0 ];
			auto const np0 = d_index[ np0_di ][ 0 ];
			hires_nearest_point_index[ i ] = np0;
			Vector const p( &point[ np0 * 3 ] );
			for( auto j = 0u; j < 3; ++j )
			{
				Vector const hp( &hires_point[ hires_index[ fi + j ] * 3 ] );
				auto const d = hp - p;
				hires_uv[ (fi + j) * 2 + 0 ] = d.inner_product( std::get< 0 >( uv_axes[ np0_di ] ) );
				hires_uv[ (fi + j) * 2 + 1 ] = d.inner_product( std::get< 1 >( uv_axes[ np0_di ] ) );
			}
		}
	}
}
std::tuple< std::vector< unsigned int >, std::vector< float > > calc_high_resolution_object_uv( std::vector< float > const &point, std::vector< unsigned int > const &index, std::vector< float > const &hires_point, std::vector< unsigned int > const &hires_index )
{
	std::vector< unsigned int > u;
	std::vector< float > f;
	calc_high_resolution_object_uv( point, index, hires_point, hires_index, u, f );
	return std::make_tuple( std::move( u ), std::move( f ) );
}

static
void make_cluster_texture_sub_vector( unsigned int const width_step, unsigned int const cluster_size_width, unsigned int const cluster_size_height, unsigned int const num, float const key_point_size, float const r, std::uint8_t *p )
{
	if( num < 1 ) return;
	std::vector< std::tuple< float, float > > circle_center;
	circle_center.reserve( num );
	switch( num )
	{
	case 1: circle_center.emplace_back( std::make_tuple( 0.0f, 0.0f ) ); break;
	case 2: circle_center.emplace_back( std::make_tuple( -r / 2, 0.0f ) ); circle_center.emplace_back( std::make_tuple( r / 2, 0.0f ) ); break;
	default:
		{
			circle_center.emplace_back( std::make_tuple( 0.0f, 0.0f ) );
			auto const dt = 2 * PI / (num - 1);
			for( auto i = 0u; i < num - 1; ++i )
			{
				auto const theta = dt * i;
				auto const st = std::sin( theta ), ct = std::cos( theta );
				circle_center.emplace_back( std::make_tuple( r * ct, r * st ) );
			}
		}
		break;
	}
	for( auto i = 0u; i < cluster_size_width; ++i )
	{
		float const x = (static_cast< float >( i ) - 1) / (cluster_size_width - 3) * 2 - 1.0f;
		for( auto j = 0u; j < cluster_size_height; ++j )
		{
			float const y = (static_cast< float >( j ) - 1) / (cluster_size_height - 3) * 2 - 1.0f;
			for( auto &&c : circle_center )
			{
				auto const d = std::hypot( x - std::get< 0 >( c ), y - std::get< 1 >( c ) );
				if( d < key_point_size )
				{
					auto const ind = (i + j * width_step) * 3;
					p[ ind + 0 ] = p[ ind + 1 ] = p[ ind + 2 ] = 0;
				}
			}
		}
	}
}

void make_cluster_texture( unsigned int const width, unsigned int const height, unsigned int const num, float const key_point_size, float const r, std::vector< std::uint8_t > &texture )
{
	texture.clear();
	texture.resize( width * height * 3, std::numeric_limits< std::decay_t< decltype( texture[ 0 ] ) > >::max() );
	make_cluster_texture_sub_vector( width, width, height, num, key_point_size, r, &texture[ 0 ]);
}
std::vector< std::uint8_t > make_cluster_texture( unsigned int const width, unsigned int const height, unsigned int const num, float const key_point_size, float const r )
{
	std::vector< std::uint8_t > v;
	make_cluster_texture( width, height, num, key_point_size, r, v );
	return std::move( v );
}

void
make_high_resolution_object_texture_and_uv_for_ply(
	unsigned int const cluster_size,
	float const key_point_size,
	float const r,
	std::vector< unsigned int > const &num,
	std::vector< unsigned int > const &hires_nearest_point_index,
	std::vector< float > const &hires_uv,
	unsigned int &width,
	unsigned int &height,
	std::vector< float > &texture_uv,
	std::vector< std::uint8_t > &texture
)
{
	assert( hires_nearest_point_index.size() * 6 == hires_uv.size() );
	// テクスチャ生成
	auto const numminmax = std::minmax_element( num.begin(), num.end() );
	auto const nummin = *std::get< 0 >( numminmax ), nummax = *std::get< 1 >( numminmax );
	auto const nonzero = nummin != 0;
	auto const c = nummax - nummin + 1 + nonzero;
	// auto const cluster_margin_size = (cluster_size + 1) / 2;
	auto const cluster_margin_size = cluster_size * 1;
	// w * h * 3 がオーバーフローすることがあるので，std::size_t
	std::size_t const w = width = (cluster_size + cluster_margin_size) * c + cluster_margin_size;
	std::size_t const h = height = cluster_size + cluster_margin_size * 2;
	texture.clear();
	texture.resize( w * h * 3, std::numeric_limits< std::decay_t< decltype( texture[ 0 ] ) > >::max() );
	auto left_offset = cluster_margin_size;
	if( nonzero )
	{
		make_cluster_texture_sub_vector( static_cast< unsigned int >( w ), cluster_size, cluster_size, 0, key_point_size, r, &texture[ (left_offset + cluster_margin_size * w) * 3 ] );
		left_offset += cluster_margin_size + cluster_size;
	}
	for( auto i = nummin; i <= nummax; ++i )
	{
		make_cluster_texture_sub_vector( static_cast< unsigned int >( w ), cluster_size, cluster_size, i, key_point_size, r, &texture[ (left_offset + cluster_margin_size * w) * 3 ] );
		left_offset += cluster_margin_size + cluster_size;
	}
	// UV座標計算
	auto const hires_triangle_num = std::size( hires_nearest_point_index );
	auto const cluster_size_plus_cluster_margin_size = cluster_size + cluster_margin_size;
	texture_uv.resize( std::size( hires_uv ) );
	for( auto i = 0u; i < hires_triangle_num; ++i )
	{
		auto const npi = hires_nearest_point_index[ i ];
		auto const tri_off = i * 3;
		if( npi == std::numeric_limits< std::decay_t< decltype( hires_nearest_point_index[ 0 ] ) > >::max() )
		{
			for( auto j = 0u; j < 3u; ++j )
			{
				auto const offset = (tri_off + j) * 2;
				texture_uv[ offset + 0 ] = ( cluster_margin_size + static_cast< float >( cluster_size ) / 2 ) / w;
				texture_uv[ offset + 1 ] = 0.5f;
			}
		}
		else
		{
			auto const n = num[ npi ];
			for( auto j = 0u; j < 3u; ++j )
			{
				auto const offset = (tri_off + j) * 2;
				texture_uv[ offset + 0 ] = static_cast< float >( (n - nummin + nonzero + 1) * cluster_size_plus_cluster_margin_size - cluster_size ) / w + hires_uv[ offset + 0 ] * cluster_size / w;
				texture_uv[ offset + 1 ] = hires_uv[ offset + 1 ] * cluster_size / h + static_cast< float >( cluster_margin_size ) / h;
			}
		}
	}
}
std::tuple< unsigned int, unsigned int, std::vector< float >, std::vector< std::uint8_t > >
make_high_resolution_object_texture_and_uv_for_ply(
	unsigned int const cluster_size,
	float const key_point_size,
	float const r,
	std::vector< unsigned int > const &num,
	std::vector< unsigned int > const &hires_nearest_point_index,
	std::vector< float > const &hires_uv
)
{
	unsigned int w, h;
	std::vector< float > f;
	std::vector< std::uint8_t > u;
	make_high_resolution_object_texture_and_uv_for_ply( cluster_size, key_point_size, r, num, hires_nearest_point_index, hires_uv, w, h, f, u );
	return std::make_tuple( w, h, std::move( f ), std::move( u ) );
}

void make_adjacency_matrix( std::vector< unsigned int > const &index, unsigned int const max_index, std::vector< std::vector< unsigned int > > &am )
{
	for( auto &v : am ) v.clear();
	am.resize( max_index );
	auto add = [ & ]( unsigned int a, unsigned int b )
	{
		auto &ama = am[ a ];
		auto it = std::lower_bound( ama.begin(), ama.end(), b );
		if( it != ama.end() && *it == b ) return;
		ama.insert( it, b );
	};
	auto add_op = [ & ]( unsigned int a, unsigned int b )
	{
		add( a, b ); add( b, a );
	};
	auto const index_size = std::size( index );
	for( auto i = 0u; i + 2 < index_size; i += 3 )
	{
		add_op( index[ i + 0 ], index[ i + 1 ] );
		add_op( index[ i + 1 ], index[ i + 2 ] );
		add_op( index[ i + 2 ], index[ i + 0 ] );
	}
}
std::vector< std::vector< unsigned int > > make_adjacency_matrix( std::vector< unsigned int > const &index, unsigned int const max_index )
{
	std::vector< std::vector< unsigned int > > am;
	make_adjacency_matrix( index, max_index, am );
	return std::move( am );
}

void calc_normal( std::vector< float > const &point, std::vector< unsigned int > const &index, std::vector< float > &normal )
{
	auto const num_of_point = std::size( point ) / 3;
	normal.clear();
	normal.reserve( num_of_point * 3 );
	std::vector< bool > flag( num_of_point, false );
	std::vector< kato::vectorf > tmp_normal( num_of_point, kato::vectorf( 0.0f, 0.0f, 0.0f ) );
	auto const index_size = std::size( index );
	for( auto i = 0u; i + 2 < index_size; i += 3 )
	{
		kato::vectorf const p1( &point[ index[ i + 0 ] * 3 ] ), p2( &point[ index[ i + 1 ] * 3 ] ), p3( &point[ index[ i + 2 ] * 3 ] );
		auto const n = (p1 - p2).cross_product( p1 - p3 ).normarize();
		for( auto i : { index[ i + 0 ], index[ i + 1 ], index[ i + 2 ] } )
		{
			flag[ i ] = true;
			tmp_normal[ i ] += n;
		}
	}
	for( auto i = 0u; i < num_of_point; ++i )
	{
		auto const n = flag[ i ] ? tmp_normal[ i ].normarize() : kato::vectorf( 0.0f, 0.0f, 0.0f );
		normal.insert( normal.end(), { n.x, n.y, n.z });
	}
}
std::vector< float > calc_normal( std::vector< float > const &point, std::vector< unsigned int > const &index )
{
	std::vector< float > normal;
	calc_normal( point, index, normal );
	return std::move( normal );
}

/*
void delaunay_triangulation( std::vector< float > const &point, std::vector< unsigned int > &index, std::vector< unsigned int > * const pouttriangle )
{
	// TODO:
}
*/
