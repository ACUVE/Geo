#define NOMINMAX
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iterator>
#include <vector>
#include <tuple>
#include <cmath>
#include <random>
#include <unordered_map>
#include <iterator>
#include <sstream>
#include <fstream>
#include <chrono>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iomanip>

#include "make_point.hpp"
#include "model_io.hpp"
#include "shader.hpp"
#include "my_utility.hpp"

#pragma comment( lib, "glew32.lib" )
#pragma comment( lib, "glfw3.lib" )
#pragma comment( lib, "OpenGL32.lib" )
#pragma comment( lib, "glu32.lib" )

constexpr unsigned int VIEW_WINDOW_WIDTH = 500u, VIEW_WINDOW_HEIGHT = VIEW_WINDOW_WIDTH;
constexpr unsigned int TEX_WINDOW_WIDTH = 500u * 2, TEX_WINDOW_HEIGHT = TEX_WINDOW_WIDTH / 2;
#if _DEBUG
constexpr unsigned int TEXTURE_WIDTH_HEIGHT = 64u;
#else
constexpr unsigned int TEXTURE_WIDTH_HEIGHT = 512u;
#endif
constexpr unsigned int MAX_CLUSTER_NUM = 7;

template< typename T >
static
GLuint make_gl_buffer( GLenum const type, GLenum const usage, std::vector< T > const &vec )
{
	GLuint id;
	glGenBuffers( 1, &id );
	glBindBuffer( type, id );
	auto const size = std::size( vec );
	glBufferData( type, sizeof( vec[ 0 ] ) * size, size ? &vec[ 0 ] : nullptr, usage );
	return id;
}
static
GLuint make_gl_texture_2d( GLint const internal_format, GLsizei const width, GLsizei const height, GLenum const format, GLenum const type, GLvoid const *data )
{
	GLuint id;
	glGenTextures( 1, &id );
	glBindTexture( GL_TEXTURE_2D, id );
	glTexImage2D( GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, data );
	return id;
}

static
GLuint compile_shader( char const *vertex_shader_src, char const *fragment_shader_src )
{
	GLint result, log_length;
	std::vector< char > log_buff;

	GLuint vertex_shader = glCreateShader( GL_VERTEX_SHADER );
	char const *pvss = vertex_shader_src;
	glShaderSource( vertex_shader, 1, &pvss, nullptr );
	glCompileShader( vertex_shader );
	glGetShaderiv( vertex_shader, GL_COMPILE_STATUS, &result );
	if( result == GL_FALSE )
	{
		glGetShaderiv( vertex_shader, GL_INFO_LOG_LENGTH, &log_length );
		if( log_length > 0 )
		{
			log_buff.resize( log_length );
			glGetShaderInfoLog( vertex_shader, log_length, nullptr, &log_buff[ 0 ] );
			std::clog << &log_buff[ 0 ] << std::endl;
		}
	}

	GLuint fragment_shader = glCreateShader( GL_FRAGMENT_SHADER );
	char const *pfss = fragment_shader_src;
	glShaderSource( fragment_shader, 1, &pfss, nullptr );
	glCompileShader( fragment_shader );
	glGetShaderiv( fragment_shader, GL_COMPILE_STATUS, &result );
	glGetShaderiv( fragment_shader, GL_INFO_LOG_LENGTH, &log_length );
	if( result == GL_FALSE )
	{
		if( log_length > 0 )
		{
			log_buff.resize( log_length );
			glGetShaderInfoLog( fragment_shader, log_length, nullptr, &log_buff[ 0 ] );
			std::clog << &log_buff[ 0 ] << std::endl;
		}
	}

	GLuint program = glCreateProgram();
	glAttachShader( program, vertex_shader );
	glAttachShader( program, fragment_shader );
	glLinkProgram( program );
	glGetProgramiv( program, GL_LINK_STATUS, &result );
	if( result == GL_FALSE )
	{
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &log_length );
		if( log_length > 0 )
		{
			log_buff.resize( log_length );
			glGetProgramInfoLog( program, log_length, nullptr, &log_buff[ 0 ] );
			std::clog << &log_buff[ 0 ] << std::endl;
		}
	}
	glDeleteShader( vertex_shader );
	glDeleteShader( fragment_shader );

	return program;
}

static
std::tuple< float, float, float > calc_cog( std::vector< float > const &point )
{
	auto const point_size = std::size( point );
	auto t = std::make_tuple( 0.0f, 0.0f, 0.0f );
	for( auto i = 0u; i + 2 < point_size; i += 3 )
	{
		std::get< 0 >( t ) += point[ i + 0 ];
		std::get< 1 >( t ) += point[ i + 1 ];
		std::get< 2 >( t ) += point[ i + 2 ];
	}
	std::get< 0 >( t ) /= point_size / 3;
	std::get< 1 >( t ) /= point_size / 3;
	std::get< 2 >( t ) /= point_size / 3;
	return std::move( t );
}

static
void set_origin( std::tuple< float, float, float > const &org, std::vector< float > &point )
{
	auto const point_size = std::size( point );
	for( auto i = 0u; i + 2 < point_size; i += 3 )
	{
		point[ i + 0 ] -= std::get< 0 >( org );
		point[ i + 1 ] -= std::get< 1 >( org );
		point[ i + 2 ] -= std::get< 2 >( org );
	}
}

// 重心を原点に移動
static
void set_cog_to_origin( std::vector< float > &point )
{
	set_origin( calc_cog( point ), point );
}

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

template< typename Func >
void apply( std::vector< float > &point, Func func )
{
	auto const point_size = std::size( point );
	for( auto i = 0u; i + 2 < point_size; i += 3 )
	{
		func( point[ i + 0 ], point[ i + 1 ], point[ i + 2 ] );
	}
}

int main( int argc, char **argv )
try
{
	std::string const to_data = R"(../../exp_data/)";
	// ファイル名の設定
	// /*
	std::string filename = to_local_path( to_data + R"(Lau_150_normalized.ply)" );
	std::string filename_hires = to_local_path( to_data +  R"(Lau_hires_normalized.ply)" );
	// std::string filename_num = to_local_path( to_data + R"(lau/150.ply.clu6.1476391063.dat)" );
	std::string filename_num = to_local_path( to_data + R"(dummy)" );
	//*/
	/*
	std::string filename = to_local_path( to_data + R"(bun_zipper_lores.ply)" );
	std::string filename_hires = to_local_path( to_data +  R"(bun_zipper_hires.ply)" );
	std::string filename_num = to_local_path( to_data + R"(bun_zipper_lores.ply.6.1486540768.clu)" );
	//*/

	glm::mat4 proj = glm::perspective( glm::radians( 30.0f ), static_cast< float >( VIEW_WINDOW_WIDTH ) / VIEW_WINDOW_HEIGHT, 0.1f, 10000.0f );
	// glm::mat4 view = glm::lookAt( glm::vec3( 400, 400, 200 ), glm::vec3( 0, 0, 0 ), glm::vec3( 0.0, 0.0, 1.0) );
	glm::mat4 view = glm::lookAt( glm::vec3( 400, 0, 0 ), glm::vec3( 0, 0, 0 ), glm::vec3( 0.0, 1.0, 0.0) );
	glm::mat4 model = glm::mat4( 1.0f );
	// glm::mat4 proj = glm::perspective( glm::radians( 30.0f ), static_cast< float >( VIEW_WINDOW_WIDTH ) / VIEW_WINDOW_HEIGHT, 0.1f, 10000.0f );
	// glm::mat4 view = glm::lookAt( glm::vec3( 0.5, 0.2, 0.5 ), glm::vec3( 0, 0, 0 ), glm::vec3( 0.0, 1.0, 0.0) );
	// glm::mat4 model = glm::mat4( 1.0f );

	// ローポリ画像のロード
	std::vector< float > point;
	std::vector< unsigned int > index;
	std::tie( point, index ) = load_ply( filename );
	std::cout << filename << "をロードしました" << std::endl;
	// ハイレゾ画像のロード
	std::vector< float > hires_point;
	std::vector< unsigned int > hires_index;
	std::tie( hires_point, hires_index ) = load_ply( filename_hires );
	std::cout << filename_hires << "をロードしました" << std::endl;

	/*
	{
		constexpr auto scale = 0.3982f;
		auto conv_point = [ scale ]( float &x, float &y, float &z ) noexcept
		{
			std::tie( y, z ) = std::make_tuple( -z, y );
			x *= scale, y *= scale, z *= scale;
		};
		apply( point, conv_point );
		apply( hires_point, conv_point );
		auto cog = calc_cog( hires_point );
		set_origin( cog, point );
		set_origin( cog, hires_point );
		auto min_z = std::numeric_limits< float >::max();
		apply( hires_point, [ & ]( float &x, float &y, float &z ){ min_z = std::min( z, min_z ); } );
		apply( point, [ & ]( float &x, float &y, float &z ){ z -= min_z; });
		apply( hires_point, [ & ]( float &x, float &y, float &z ){ z -= min_z; });
	}
	// */

	/*
	write_ply( "Lau_hires_normalized.ply", hires_point, hires_index );
	std::exit( 0 );
	*/

	// 簡単なデータの出力
	std::cout << "クラスタ設置点の数："  << point.size() / 3 << std::endl;
	std::cout << "クラスタ用CGの面の数：" << index.size() / 3 << std::endl;
	std::cout << "ハイレゾ画像の点の数：" << hires_point.size() / 3 << std::endl;
	std::cout << "ハイレゾ画像の面の数：" << hires_index.size() / 3 << std::endl;
	
	// クラスタの計算
	std::cout << "クラスタデータロード開始……" << std::endl;
	std::vector< unsigned int > num;
	try
	{
		num = load_num( filename_num );
		std::cout << "クラスタデータロード終了……" << std::endl;
	}
	catch( ... )
	{
		std::cout << "クラスタデータロード失敗" << std::endl;
		std::cout << "クラスタ計算開始……" << std::endl;
		num = make_claster( static_cast< unsigned int >( point.size() / 3 ), MAX_CLUSTER_NUM, index );
		// クラスタデータ出力
		{
			auto const now = std::chrono::system_clock::now();
			auto const d = now.time_since_epoch();
			auto const d_s = std::chrono::duration_cast< std::chrono::seconds >( d );
			auto const wfnn = filename + "." + std::to_string( MAX_CLUSTER_NUM ) + "." + std::to_string( d_s.count() ) + ".clu";
			write_num( wfnn, num );
		}
		std::cout << "クラスタ計算終了" << std::endl;
	}
	std::cout << "クラスタ点数：" << num.size() << std::endl;

	// ハイレゾデータのuv座標を取得
	std::vector< unsigned int > hires_nearest_point_index;
	std::vector< float > hires_uv;
	std::cout << "uv座標計算開始……" << std::endl;
	std::tie( hires_nearest_point_index, hires_uv ) = calc_high_resolution_object_uv( point, index, hires_point, hires_index );
	std::cout << "uv座標計算完了" << std::endl;

	auto const triangle_num = hires_index.size() / 3;

	/*
	std::vector< float > d_point;
	std::vector< std::vector< unsigned int > > d_index;
	std::tie( d_point, d_index ) = make_dual( point, index );
	std::vector< std::vector< float > > d_uv = calc_dual_uv( d_point, d_index );
	for( auto &&v : d_uv ) for( auto &&u : v ) u = (u + 15) / 30;
	*/
	// for( auto &uv : hires_uv ) uv = (uv + 0.005f) / 0.01f;
	for( auto &uv : hires_uv ) uv = (uv + 15) / 30;
	// for( auto &uv : hires_uv ) uv = uv / 100.0f + 0.5f;
	// for( auto &uv : hires_uv ) uv = uv / 30.0f + 0.5f;
	// for( auto &uv : hires_uv ) uv = uv * 150.0f + 0.5f;
	// for( auto &uv : hires_uv ) uv = uv * 80.0f + 0.5f;

	auto ply_texture = make_high_resolution_object_texture_and_uv_for_ply( TEXTURE_WIDTH_HEIGHT, 0.2f, 0.6f, num, hires_nearest_point_index, hires_uv );
	kato::writeBMP( "ply.bmp", std::get< 0 >( ply_texture ), std::get< 1 >( ply_texture ), &std::get< 3 >( ply_texture )[ 0 ] );

	std::cout << "ply出力開始……" << std::endl;
	write_ply_with_texture( "ply.ply", "ply.bmp", hires_point, hires_index, std::get< 2 >( ply_texture ) );
	std::cout << "ply出力完了" << std::endl;

	//TODO: 
	std::exit( 0 );

	// 表示のために重心が原点の位置になるように調整
	set_cog_to_origin( hires_point );

	// テクスチャデータ生成
	std::vector< std::vector< std::uint8_t > > texture_data;
	for( unsigned int i = 0u; i <= MAX_CLUSTER_NUM; ++i )
	{
		texture_data.emplace_back( make_cluster_texture( TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT, i, 0.2f, 0.6f ) );
		std::ostringstream oss;
		oss << "texture_" << i << ".bmp";
		kato::writeBMP( oss.str().c_str(), TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT, &texture_data.back()[ 0 ] );
	}

	// 新しくかいたこーど
	glfwInit();
	auto window = glfwCreateWindow( VIEW_WINDOW_WIDTH, VIEW_WINDOW_HEIGHT, "OK", nullptr, nullptr );
	glfwMakeContextCurrent( window );
	glewExperimental = true;
	glewInit();

	glViewport( 0, 0, VIEW_WINDOW_WIDTH, VIEW_WINDOW_HEIGHT );

	auto program = compile_shader( vertex_shader_src, fragment_shader_src1 );
	auto program2 = compile_shader( vertex_shader_src, fragment_shader_src2 );

	glEnable( GL_DEPTH_TEST );

	std::vector< GLuint > point_buffer, uv_buffer;
	for( auto i = 0u; i < triangle_num; ++i )
	{
		std::vector< float > pb( 3 * 3 ), uvb( 2 * 3 );
		for( auto j = 0u; j < 3u; ++j )
		{
			std::copy_n( &hires_point[ hires_index[ i * 3 + j ] * 3 ], 3, pb.begin() + j * 3 );
		}
		std::copy_n( &hires_uv[ i * 3 * 2 ], 2 * 3, uvb.begin() );
		point_buffer.emplace_back( make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, pb ) );
		uv_buffer.emplace_back( make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, uvb ) );
	}

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	std::vector< GLuint > dot_texture;
	for( auto i = 0u; i <= MAX_CLUSTER_NUM; ++i )
	{
		dot_texture.emplace_back( make_gl_texture_2d( GL_RGB, TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, &texture_data[ i ][ 0 ] ) );
	}
	for( auto &&v : dot_texture )
	{
		glBindTexture( GL_TEXTURE_2D, v );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	}

	while( true )
	{
		if( glfwWindowShouldClose( window ) )
		{
			break;
		}
		
		glfwMakeContextCurrent( window );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		static unsigned int i = 0;
		glm::mat4 model = glm::rotate( glm::radians( static_cast< float >( i++ ) * 3 ), glm::vec3( 0.0, 1.0, 0.0 ) );
		glm::mat4 mvp = proj * view * model;
		glUseProgram( program );
		glUniformMatrix4fv( glGetUniformLocation( program, "Hmat" ), 1, GL_FALSE, &mvp[ 0 ][ 0 ] );

		glUseProgram( program2 );
		glUniformMatrix4fv( glGetUniformLocation( program2, "Hmat" ), 1, GL_FALSE, &mvp[ 0 ][ 0 ] );

		glEnableClientState( GL_VERTEX_ARRAY );

		///*
		glUseProgram( program );
		for( auto i = 0u; i < triangle_num; ++i )
		{
			glEnableVertexAttribArray( 0 );
			glBindBuffer( GL_ARRAY_BUFFER, point_buffer[ i ] );
			glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );
			glEnableVertexAttribArray( 1 );
			glBindBuffer( GL_ARRAY_BUFFER, uv_buffer[ i ] );
			glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );
			glActiveTexture( GL_TEXTURE0 );
			unsigned int dotnum;
			if( hires_nearest_point_index[ i ] == std::numeric_limits< unsigned int >::max() )
			{
				dotnum = 0;
			}
			else
			{
				dotnum = num[ hires_nearest_point_index[ i ] ];
			}
			glBindTexture( GL_TEXTURE_2D, dot_texture[ dotnum ] );
			glDrawArrays( GL_TRIANGLES, 0, 3 );
		}

		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glFlush();
		glfwSwapBuffers( window );

		if( 2 <= i && i <= 121 )
		{
			auto ptr = std::make_unique< std::uint8_t[] >( VIEW_WINDOW_WIDTH * VIEW_WINDOW_HEIGHT * 3 );
			glReadPixels( 0, 0, VIEW_WINDOW_WIDTH, VIEW_WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, ptr.get() );
			std::ostringstream oss;
			oss << "img" << std::setw( 3 ) << std::setfill( '0' ) << i - 1 << ".bmp";
			kato::writeBMP( oss.str().c_str(), VIEW_WINDOW_WIDTH, VIEW_WINDOW_HEIGHT, ptr.get() );
		}

		glfwPollEvents();
	}
	glfwTerminate();
}
catch( std::exception &exp )
{
	std::cerr << exp.what() << std::endl;
	char c;
	std::cin >> c;
}
