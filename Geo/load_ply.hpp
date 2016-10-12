#pragma once
#include <tuple>
#include <vector>
#include <string>

void load_ply( std::string const &filename, std::vector< float > &point, std::vector< unsigned int > &index );
std::tuple< std::vector< float >, std::vector< unsigned int > > load_ply( std::string const &filename );
