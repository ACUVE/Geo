#pragma once

#include <vector>
#include <tuple>
#include <memory>
#include <unordered_map>
#include <atomic>
#include "custom_hash.hpp"

// ポリゴンの2点のp_indexから残りの1点のp_indexを引く
cunordered_map< std::tuple< unsigned int, unsigned int >, unsigned int >
make_vmap(
	std::vector< unsigned int > const &index
);
// ポリゴンの2点のp_indexからそれ自身のindexのを引く
cunordered_map< std::tuple< unsigned int, unsigned int >, unsigned int >
make_map_pindex_to_index(
	std::vector< unsigned int > const &index
);

void
make_geodesic_dome_point(
	unsigned int const level,
	std::vector< float > &point,
	std::vector< unsigned int > &index
);
std::tuple< std::vector< float >, std::vector< unsigned int > >
make_geodesic_dome_point(
	unsigned int const level
);
void
make_regular_pentakis_dodecahedron_point(
	unsigned int const level,
	std::vector< float > &point,
	std::vector< unsigned int > &index
);
std::tuple< std::vector< float >, std::vector< unsigned int > >
make_regular_pentakis_dodecahedron_point(
	unsigned int const level
);

void
partition_polygon_on_ball(
	unsigned int level,
	std::vector< float > &point,
	std::vector< unsigned int > &index
);

void
euclidean_to_theta_phi_of_poler_for_draw(
	std::vector< float > const &point,
	std::vector< unsigned int > const &index,
	std::vector< float > &poler_theta_phi,
	std::vector< unsigned int > &poler_index,
	std::vector< unsigned int > *poler_point_to_point = nullptr
);
std::tuple< std::vector< float >, std::vector< unsigned int > >
euclidean_to_theta_phi_of_poler_for_draw(
	std::vector< float > const &point,
	std::vector< unsigned int > const &index,
	std::vector< unsigned int > *poler_point_to_point = nullptr
);

std::vector< unsigned int >
make_claster_impl(
	unsigned int const point_num,
	unsigned int const max_num,
	unsigned int const sep_num,
	std::vector< std::vector< unsigned int * > > const &check_timing_list,
	std::vector< std::vector< unsigned int > > const &map,
	std::atomic< bool > &flag
);

void
make_claster(
	unsigned int const point_num,
	unsigned int const max_num,
	std::vector< unsigned int > const &index,
	std::vector< unsigned int > &num
);
std::vector< unsigned int >
make_claster(
	unsigned int const point_num,
	unsigned int const max_num,
	std::vector< unsigned int > const &index
);

void
make_texture(
	unsigned int const width,
	unsigned int const height,
	std::vector< unsigned int > const &num,
	std::vector< float > const &point,
	float const dot_size,
	float const claster_size_in_rad,
	bool const multi_thread,
	std::vector< std::uint8_t > &texture
);
std::vector< std::uint8_t >
make_texture(
	unsigned int const width,
	unsigned int const height,
	std::vector< unsigned int > const &num,
	std::vector< float > const &point,
	float const dot_size,
	float const claster_size,
	bool const multi_thread
);

// 実行後 std::equal( point.begin(), point.end(), d_point.begin(), d_point.begin() + point.size() ) == true && (d_index[ind].empty() || d_index[ind][0]==ind) を満たす
void
make_dual(
	std::vector< float > const &point,
	std::vector< unsigned int > const &index,
	std::vector< float > &d_point,
	std::vector< std::vector< unsigned int > > &d_index
);
std::tuple< std::vector< float >, std::vector< std::vector< unsigned int > > >
make_dual(
	std::vector< float > const &point,
	std::vector< unsigned int > const &index
);

void
calc_dcm_cluster_normal(
	std::vector< float > const &dual_point,
	std::vector< std::vector< unsigned int > > const &dual_index,
	std::vector< float > &dcm_normal
);
std::vector< float >
calc_dcm_cluster_normal(
	std::vector< float > const &dual_point,
	std::vector< std::vector< unsigned int > > const &dual_index
);

// ここでのUV座標は，テクスチャの中心を(0,0)としている点に注意．また，スケールも割と適当である．
void
calc_dual_uv(
	std::vector< float > const &d_point,
	std::vector< std::vector< unsigned int > > const &d_index,
	std::vector< std::vector< float > > &d_uv
);
std::vector< std::vector< float > >
calc_dual_uv(
	std::vector< float > const &d_point,
	std::vector< std::vector< unsigned int > > const &d_index
);

// ここでのUV座標は，テクスチャの中心を(0,0)としている点に注意．また，スケールも割と適当である．
void
calc_high_resolution_object_uv(
	std::vector< float > const &point,
	std::vector< unsigned int > const &index,
	std::vector< float > const &hires_point,
	std::vector< unsigned int > const &hires_index,
	std::vector< unsigned int > &hires_nearest_point_index,
	std::vector< float > &hires_uv
);
std::tuple< std::vector< unsigned int >, std::vector< float > >
calc_high_resolution_object_uv(
	std::vector< float > const &point,
	std::vector< unsigned int > const &index,
	std::vector< float > const &hires_point,
	std::vector< unsigned int > const &hires_index
);

void
make_cluster_texture(
	unsigned int const width,
	unsigned int const height,
	unsigned int const num,
	float const key_point_size,
	float const r,
	std::vector< std::uint8_t > &texture
);
std::vector< std::uint8_t >
make_cluster_texture(
	unsigned int const width,
	unsigned int const height,
	unsigned int const num,
	float const key_point_size,
	float const r
);

// hires_uvのUV座標は，テクスチャの中心を(0.5,0.5)としなければならない点に注意（スケールもいい感じに調節して欲しい）
// つまり，一般的なUV座標表現である
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
);
std::tuple< unsigned int, unsigned int, std::vector< float >, std::vector< std::uint8_t > >
make_high_resolution_object_texture_and_uv_for_ply(
	unsigned int const cluster_size,
	float const key_point_size,
	float const r,
	std::vector< unsigned int > const &num,
	std::vector< unsigned int > const &hires_nearest_point_index,
	std::vector< float > const &hires_uv
);

void
make_adjacency_matrix(
	std::vector< unsigned int > const &index,
	unsigned int const max_index,
	std::vector< std::vector< unsigned int > > &am
);
std::vector< std::vector< unsigned int > >
make_adjacency_matrix(
	std::vector< unsigned int > const &index,
	unsigned int const max_index
);

// 点の法線を計算するけれども，恐らく一般的であると思われる方法で計算するやつ
void
calc_normal(
	std::vector< float > const &point,
	std::vector< unsigned int > const &index,
	std::vector< float > &normal
);
std::vector< float >
calc_normal(
	std::vector< float > const &point,
	std::vector< unsigned int > const &index
);
