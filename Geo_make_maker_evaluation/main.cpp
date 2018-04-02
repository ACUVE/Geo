#include <string>
#include <cstdlib>
#include <atomic>
#include <thread>
#include <iostream>
#include <algorithm>
#include "make_point.hpp"
#include "model_io.hpp"

static
std::string to_local_path( std::string filepath )
{
#if _WIN32
	std::string::size_type pos = 0u;
	while( true )
	{
		pos = filepath.find( '/', pos );
		if( pos == std::string::npos ) break;
		filepath.replace( pos, 1, 1, '\\' );
		pos += 1;
	}
#endif
	return std::move( filepath );
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

template< typename Rep, typename Period >
static
void make_claster_for_eval( unsigned int const point_num, unsigned int const max_num, std::vector< unsigned int > const &index, std::chrono::duration< Rep, Period > const &duration, std::vector< unsigned int > &num, unsigned long long int &count )
{
	// 大体make_pointのmake_clusterからコピーしてきた
	auto const map = make_adjacency_matrix( index, point_num );
	auto const vmap = make_vmap( index );
	unsigned int argsepnum;
	std::vector< unsigned int > argindex;

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

	std::vector< std::vector< unsigned int * > > check_timing_list( point_num );
	for( auto i = 0u; i + argsepnum - 1 < argindex.size(); i += argsepnum )
	{
		auto const m = *std::max_element( &argindex[ i ], &argindex[ i ] + argsepnum );
		check_timing_list[ m ].emplace_back( &argindex[ i ] );
	}

	std::vector< std::thread > th;
	std::atomic< unsigned long long int > count_num{ 0u };
	auto const tn = thread_num();
	std::atomic< bool > flags[ 100 ];
	if( tn > std::size( flags ) ) throw std::exception( "make_claster_for_eval: flags' size must be enlarged" );
	for( auto &f : flags ) f = false;
	std::atomic< bool > realend_flag = false;
	auto const now = std::chrono::steady_clock::now();
	for( auto i = 0u; i < tn; ++i )
	{
		th.emplace_back(
			[ &, i ]()
			{
				while( !realend_flag )
				{
					flags[ i ] = false;
					auto v = make_claster_impl( point_num, max_num, argsepnum, check_timing_list, map, flags[ i ] );
					if( !std::empty(v) )
					{
						if( count_num.fetch_add( 1u ) == 0u )
						{
							num = std::move( v );
						}
					}
				}
			}
		);
	}
	std::this_thread::sleep_until( now + duration );
	count = count_num.load();
	realend_flag = true;
	for( auto &f : flags ) f = true;
	for( auto &t : th ) t.join();
}
template< typename Rep, typename Period >
static
std::tuple< std::vector< unsigned int >, unsigned long long int > make_claster_for_eval( unsigned int const point_num, unsigned int const max_num, std::vector< unsigned int > const &index, std::chrono::duration< Rep, Period > const &duration )
{
	std::vector< unsigned int > u;
	unsigned long long int c;
	make_claster_for_eval( point_num, max_num, index, duration, u, c );
	return std::make_tuple( std::move( u ), std::move( c ) );
}

int main( int argc, char **argv )
{
	using namespace std::string_literals;
	using namespace std::chrono_literals;

	if( argc < 3 ) return 1;

	// auto const &to_data = R"(../../exp_data/)"s;
	// auto const &filename = to_local_path( to_data + R"(Lau_150_normalized.ply)"s );
	auto const &filename = to_local_path( argv[ 1 ] );
	auto const &max_num = static_cast< unsigned int >( std::atoi( argv[ 2 ] ) );

	auto const &ply = load_ply( filename );
	auto const &point = std::get< 0 >( ply );
	auto const &index = std::get< 1 >( ply );

	// ここで待ち時間を調整する
	// auto const wait_duration = 1s;
	auto const wait_duration = 30min;	// 論文用（30分）

	auto const &t = make_claster_for_eval( static_cast< unsigned int >( std::size( point ) / 3u ), max_num, index, wait_duration );
	auto const &num = std::get< std::vector< unsigned int > >( t );
	auto const &find_num = std::get< unsigned long long int >( t );

	std::cout << find_num << std::endl; 
	if( !std::empty( num ) )
	{
		write_num( filename + "." + std::to_string( max_num ) + ".clu", num );
	}
}