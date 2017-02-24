#pragma once
#include <tuple>
#include <vector>
#include <string>

void load_ply( std::string const &filename, std::vector< float > &point, std::vector< unsigned int > &index );
std::tuple< std::vector< float >, std::vector< unsigned int > > load_ply( std::string const &filename );

void write_ply( std::string const &filename, std::vector< float > const &point, std::vector< unsigned int > const &index );

void write_ply_with_texture( std::string const &filename, std::string const &texture_filename, std::vector< float > const &point, std::vector< unsigned int > const &index, std::vector< float > const &index_uv );

void load_num( std::string const &filename, std::vector< unsigned int > &num );
std::vector< unsigned int > load_num( std::string const &filename );

void write_num( std::string const &filename, std::vector< unsigned int > const &num );

void load_wow_data( std::string const &filename, unsigned int &point_cols, unsigned int &point_rows, std::vector< float > &point, std::vector< unsigned int > &num );
std::tuple< unsigned int, unsigned int, std::vector< float >, std::vector< unsigned int > > load_wow_data( std::string const &filename );
