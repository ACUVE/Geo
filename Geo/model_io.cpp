#include <fstream>
#include <tuple>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <cassert>

using namespace std::string_literals;

void load_ply( std::string const &filename, std::vector< float > &point, std::vector< unsigned int > &index )
{
	point.clear();
	index.clear();
	std::ifstream ifs( filename, std::ios::binary );
	if( !ifs.is_open() ) throw std::exception();
	int vertexline = 0, faceline = 0;
	enum class format{
		ASCII, BINARY_LE, UNKNOWN
	} read_format = format::UNKNOWN;
	std::string line;
	while( std::getline( ifs, line ) )
	{
		if( line.substr( 0, 7 ) == "format "s )
		{
			if( line.substr( 7, 6 ) == "ascii "s )
			{
				read_format = format::ASCII;
			}
			else if( line.substr( 7, 21 ) == "binary_little_endian "s )
			{
				read_format = format::BINARY_LE;
			}
		}
		else if( line.substr( 0, 15 ) == "element vertex "s )
		{
			vertexline = std::atoi( line.substr( 15 ).c_str() );
		}
		else if( line.substr( 0, 13 ) == "element face "s )
		{
			faceline = std::atoi( line.substr( 13 ).c_str() );
		}
		else if( line.substr( 0, 10 ) == "end_header"s )
		{
			break;
		}
	}
	if( read_format == format::UNKNOWN ) throw std::exception();
	if( vertexline == 0 || faceline == 0 ) throw std::exception();
	point.resize( vertexline * 3 );
	index.resize( faceline * 3 );
	switch( read_format )
	{
	case format::ASCII:
		{
			for( int i = 0; i < vertexline && std::getline( ifs, line ); ++i )
			{
				std::istringstream iss( line );
				auto const ind = i * 3;
				iss >> point[ ind + 0 ] >> point[ ind + 1 ] >> point[ ind + 2 ];
			}
			for( int i = 0; i < faceline && std::getline( ifs, line ); ++i )
			{
				std::istringstream iss( line );
				auto const ind = i * 3;
				int dummy;
				iss >> dummy >> index[ ind + 0 ] >> index[ ind + 1 ] >> index[ ind + 2 ];
				if( dummy != 3 )
				{
					point.clear();
					index.clear();
					return;
				}
			}
		}
		break;
	case format::BINARY_LE:
		{
			for( int i = 0; i < vertexline; ++i )
			{
				auto const ind = i * 3;
				ifs.read( reinterpret_cast< char * >( &point[ ind + 0 ] ), sizeof( float ) );
				ifs.read( reinterpret_cast< char * >( &point[ ind + 1 ] ), sizeof( float ) );
				ifs.read( reinterpret_cast< char * >( &point[ ind + 2 ] ), sizeof( float ) );
			}
			for( int i = 0; i < faceline; ++i )
			{
				auto const ind = i * 3;
				char dummy;
				ifs.read( &dummy, sizeof( char ) );
				if( dummy != 3 )
				{
					point.clear();
					index.clear();
					return;
				}
				ifs.read( reinterpret_cast< char * >( &index[ ind + 0 ] ), sizeof( float ) );
				ifs.read( reinterpret_cast< char * >( &index[ ind + 1 ] ), sizeof( float ) );
				ifs.read( reinterpret_cast< char * >( &index[ ind + 2 ] ), sizeof( float ) );
			}
		}
		break;
	}
}

std::tuple< std::vector< float >, std::vector< unsigned int > > load_ply( std::string const &filename )
{
	std::vector< float > vf;
	std::vector< unsigned int > uf;
	load_ply( filename, vf, uf );
	return std::make_tuple( std::move( vf ), std::move( uf ) );
}

void write_ply( std::string const &filename, std::vector< float > const &point, std::vector< unsigned int > const &index )
{
	std::ofstream ofs( filename );
	auto const num_vertex = std::size( point ) / 3;
	auto const num_face = std::size( index ) / 3;
	ofs <<
R"(ply
format ascii 1.0
comment VCGLIB generated
element vertex )" << std::size( point ) / 3 << R"(
property float x
property float y
property float z
element face )" << std::size( index ) / 3 << R"(
property list uchar int vertex_indices
end_header
)";
	for( auto i = 0u; i < num_vertex; ++i )
	{
		for( auto j = 0u; j < 3u; ++j )
		{
			if( j != 0 ) ofs << " ";
			ofs << point[ i * 3 + j ];
		}
		ofs << "\n";
	}
	for( auto i = 0u; i < num_face; ++i )
	{
		ofs << "3";
		for( auto j = 0u; j < 3u; ++j )
		{
			ofs << " " << index[ i * 3 + j ];
		}
		ofs << "\n";
	}
}

void write_ply_with_texture( std::string const &filename, std::string const &texture_filename, std::vector< float > const &point, std::vector< unsigned int > const &index, std::vector< float > const &index_uv )
{
	assert( std::size( index ) * 2 == std::size( index_uv ) );
	std::ofstream ofs( filename );
	auto const num_vertex = std::size( point ) / 3;
	auto const num_face = std::size( index ) / 3;
	ofs <<
R"(ply
format ascii 1.0
comment VCGLIB generated
comment TextureFile )" << texture_filename << R"(
element vertex )" << num_vertex << R"(
property float x
property float y
property float z
element face )" << num_face << R"(
property list uchar int vertex_indices
property list uchar float texcoord
end_header
)";
	for( auto i = 0u; i < num_vertex; ++i )
	{
		for( auto j = 0u; j < 3u; ++j )
		{
			if( j != 0 ) ofs << " ";
			ofs << point[ i * 3 + j ];
		}
		ofs << "\n";
	}
	for( auto i = 0u; i < num_face; ++i )
	{
		ofs << "3";
		for( auto j = 0u; j < 3u; ++j )
		{
			ofs << " " << index[ i * 3 + j ];
		}
		ofs << " 6";
		for( auto j = 0u; j < 6u; ++j )
		{
			ofs << " " << index_uv[ i * 6 + j ];
		}
		ofs << "\n";
	}
}

void load_num( std::string const &filename, std::vector< unsigned int > &num )
{
	num.clear();
	std::ifstream ifs( filename );
	if( !ifs.is_open() ) throw std::exception();
	while( true )
	{
		unsigned int n;
		ifs >> n;
		if( ifs.eof() || ifs.bad() ) break;
		num.emplace_back( n );
	}
}
std::vector< unsigned int > load_num( std::string const &filename )
{
	std::vector< unsigned int > n;
	load_num( filename, n );
	return std::move( n );
}

void write_num( std::string const &filename, std::vector< unsigned int > const &num )
{
	std::ofstream ofs( filename );
	for( auto &n : num )
	{
		ofs << n << "\n";
	}
	ofs << std::flush;
}

void load_wow_data( std::string const &filename, unsigned int &point_cols, unsigned int &point_rows, std::vector< float > &point, std::vector< unsigned int > &num )
{
	point.clear(); num.clear();

	std::ifstream ifs( filename );
	if( !ifs.is_open() ) throw std::runtime_error( "load_wow_data: file open error: filename=" + filename );
	
	// 面倒くさいので一個しか対応していない
	unsigned int num_of_marker;
	ifs >> num_of_marker;
	if( num_of_marker != 1 ) throw std::runtime_error( "load_wow_data: load_wow_data support only one marker: filename=" + filename );
	ifs >> point_cols >> point_rows;
	auto const size_point = point_cols * point_rows;
	point.reserve( size_point * 3 ); num.reserve( size_point );
	for( unsigned int i = 0; i < size_point; ++i )
	{
		unsigned int tn;
		float tx, ty, tz;
		ifs >> tn >> tx >> ty >> tz;
		point.insert( point.end(), { tx, ty, tz } );
		num.emplace_back( tn );
	}
	if( !ifs ) throw std::runtime_error( "load_wow_data: error eof: filename=" + filename );
}
std::tuple< unsigned int, unsigned int, std::vector< float >, std::vector< unsigned int > > load_wow_data( std::string const &filename )
{
	unsigned int point_cols, point_rows;
	std::vector< float > point;
	std::vector< unsigned int > num;
	load_wow_data( filename, point_cols, point_rows, point, num );
	return std::make_tuple( std::move( point_cols ), std::move( point_rows ), std::move( point ), std::move( num ) );
}
