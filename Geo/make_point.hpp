#pragma once

#include <vector>
#include <tuple>
#include <memory>

void make_geodesic_dome_point( unsigned int const level, std::vector< float > &point, std::vector< unsigned int > &index );
std::tuple< std::vector< float >, std::vector< unsigned int > > make_geodesic_dome_point( unsigned int const level );
void make_regular_pentakis_dodecahedron_point( unsigned int const level, std::vector< float > &point, std::vector< unsigned int > &index );
std::tuple< std::vector< float >, std::vector< unsigned int > > make_regular_pentakis_dodecahedron_point( unsigned int const level );

void partition_polygon_on_ball( unsigned int level, std::vector< float > &point, std::vector< unsigned int > &index );

void euclidean_to_theta_phi_of_poler_for_draw( std::vector< float > const &point, std::vector< unsigned int > const &index, std::vector< float > &poler_theta_phi, std::vector< unsigned int > &poler_index, std::vector< unsigned int > *poler_point_to_point = nullptr );
std::tuple< std::vector< float >, std::vector< unsigned int > > euclidean_to_theta_phi_of_poler_for_draw( std::vector< float > const &point, std::vector< unsigned int > const &index, std::vector< unsigned int > *poler_point_to_point = nullptr );

void make_claster( unsigned int const point_num, unsigned int const max_num, std::vector< unsigned int > const &index, std::vector< unsigned int > &num );
std::vector< unsigned int > make_claster( unsigned int const point_num, unsigned int const max_num, std::vector< unsigned int > const &index );

std::vector< std::uint8_t > make_texture( unsigned int const width, unsigned int const height, std::vector< unsigned int > const &num, std::vector< float > const &point, float const dot_size, float const claster_size, bool const multi_thread = true );