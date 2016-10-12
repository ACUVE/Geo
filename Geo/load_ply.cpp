#include <fstream>
#include <tuple>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

void load_ply( std::string const &filename, std::vector< float > &point, std::vector< unsigned int > &index )
{
	using namespace std::string_literals;
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