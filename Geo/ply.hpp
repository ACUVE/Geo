#pragma once
#include <tuple>
#include <vector>
#include <string>

void load_ply( std::string const &filename, std::vector< float > &point, std::vector< unsigned int > &index );
std::tuple< std::vector< float >, std::vector< unsigned int > > load_ply( std::string const &filename );

void write_ply_with_texture( std::string const &filename, std::string const &texture_filename, std::vector< float > &point, std::vector< unsigned int > const &index, std::vector< float > const &index_uv );
