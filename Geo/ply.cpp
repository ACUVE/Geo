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
	std::ifstream ifs( filename );
	int vertexline = 0, faceline = 0;
	std::string line;
	while( std::getline( ifs, line ) )
	{
		if( line.substr( 0, 15 ) == "element vertex "s )
		{
			vertexline = std::atoi( line.substr( 15 ).c_str() );
		}
		else if( line.substr( 0, 13 ) == "element face "s )
		{
			faceline = std::atoi( line.substr( 13 ).c_str() );
		}
		else if( line == "end_header"s )
		{
			break;
		}
	}
	
	if( vertexline == 0 || faceline == 0 ) return;
	point.resize( vertexline * 3 );
	index.resize( faceline * 3 );
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

std::tuple< std::vector< float >, std::vector< unsigned int > > load_ply( std::string const &filename )
{
	std::vector< float > vf;
	std::vector< unsigned int > uf;
	load_ply( filename, vf, uf );
	return std::make_tuple( std::move( vf ), std::move( uf ) );
}

void write_ply_with_texture( std::string const &filename, std::string const &texture_filename, std::vector< float > &point, std::vector< unsigned int > const &index, std::vector< float > const &index_uv )
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
element vertex )" << std::size( point ) / 3 << R"(
property float x
property float y
property float z
element face )" << std::size( index ) / 3 << R"(
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