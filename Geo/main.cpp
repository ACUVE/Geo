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
#include <windows.h>
#include <gl/glew.h>
#include <gl/wglew.h>
#include <gl/GL.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "make_point.hpp"
#include "load_ply.hpp"
#include "shader.hpp"

#pragma comment( lib, "glew32.lib" )
#pragma comment( lib, "glfw3.lib" )
#pragma comment( lib, "OpenGL32.lib" )
#pragma comment( lib, "glu32.lib" )

#define IS_DUAL 1

constexpr unsigned int VIEW_WINDOW_WIDTH = 750u, VIEW_WINDOW_HEIGHT = VIEW_WINDOW_WIDTH;
constexpr unsigned int TEX_WINDOW_WIDTH = 500u * 2, TEX_WINDOW_HEIGHT = TEX_WINDOW_WIDTH / 2;
#if _DEBUG
constexpr unsigned int TEXTURE_WIDTH_HEIGHT = 64u;
#else
constexpr unsigned int TEXTURE_WIDTH_HEIGHT = 2048u;
#endif
constexpr unsigned int MAX_CLUSTER_NUM = 6;
static_assert( MAX_CLUSTER_NUM <= MAX_DUAL_TEXTURE_NUM_OF_SHADER, "" );
#if !IS_DUAL
constexpr bool IS_BALL = false;
#endif

static void writeBMP(char const *const filename, std::uint32_t const width, std::uint32_t const height, void const *ptr){
	HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if(hFile == INVALID_HANDLE_VALUE) return;
	std::uint32_t const pixel_num = width * height;
#define UINT16TO2BYTE(NUM) (std::uint16_t)(NUM) & 0xFF, ((std::uint16_t)(NUM) >> 8) & 0xFF
#define UINT32TO4BYTE(NUM) (std::uint32_t)(NUM) & 0xFF, ((std::uint32_t)(NUM) >> 8) & 0xFF, ((std::uint32_t)(NUM) >> 16) & 0xFF, ((std::uint32_t)(NUM) >> 24) & 0xFF
	DWORD written;
	std::uint8_t const header[14] =
	{
		'B', 'M',
		UINT32TO4BYTE(0),	// ファイルサイズ入れるべき
		UINT16TO2BYTE(0),
		UINT16TO2BYTE(0),
		UINT32TO4BYTE(14 + 40)
	};
	WriteFile(hFile, header, sizeof(header), &written, nullptr);
	std::uint8_t const infoheader[40] =
	{
		UINT32TO4BYTE(40),
		UINT32TO4BYTE(width),
		UINT32TO4BYTE(height),
		UINT16TO2BYTE(1),
		UINT16TO2BYTE(24),
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
	};
	WriteFile(hFile, infoheader, sizeof(infoheader), &written, nullptr);
	std::uint8_t const padsize = (4 - (width * 3) % 4) % 4;
	constexpr std::uint8_t zeros[ 3 ]{};
	if( padsize == 0 )
	{
		WriteFile(hFile, ptr, pixel_num * 3, &written, nullptr);
	}
	else
	{
		for( std::uint32_t i = 0u; i < height; ++i )
		{
			WriteFile( hFile, (std::uint8_t *)ptr + width * i, width * 3, &written, nullptr);
			WriteFile( hFile, zeros, padsize, &written, nullptr);
		}
	}
	CloseHandle(hFile);
#undef UINT16TO2BYTE
#undef UINT32TO4BYTE
}

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

// 重心を原点に移動
static
void set_cog_to_origin( std::vector< float > &point )
{
	float sum[ 3 ] = {};
	for( auto i = 0u; i + 2 < point.size(); i += 3 )
		for( auto j = 0u; j < 3u; ++j )
			sum[ j ] += point[ i + j ];
	for( auto j = 0u; j < 3; ++j ) sum[ j ] /= point.size() / 3;
	for( auto i = 0u; i + 2 < point.size(); i += 3 )
		for( auto j = 0u; j < 3u; ++j )
			point[ i + j ] -= sum[ j ];
}

int main( int argc, char **argv )
{
#if IS_DUAL
	glm::mat4 proj = glm::perspective( glm::radians( 30.0f ), static_cast< float >( VIEW_WINDOW_WIDTH ) / VIEW_WINDOW_HEIGHT, 0.1f, 10000.0f );
	glm::mat4 view = glm::lookAt( glm::vec3( 1000, 500, 1000 ), glm::vec3( 0, 0, 0 ), glm::vec3( 0.0, 1.0, 0.0) );
	glm::mat4 model = glm::mat4( 1.0f );

	std::string filename = "C:\\Users\\t2ladmin\\Downloads\\acvd1.1\\output_1.ply";

	std::vector< float > point;
	std::vector< unsigned int > index;
	std::tie( point, index ) = load_ply( filename );
	set_cog_to_origin( point );
	std::cout << "面の数：" << index.size() / 3 << std::endl;
	std::cout << "点の数：" << point.size() / 3 << std::endl;
	std::cout << "辺の数：" << index.size() / 2 << std::endl;
	std::vector< unsigned int > num = make_claster( static_cast< unsigned int >( point.size() / 3 ), MAX_CLUSTER_NUM, index );
	// std::vector< unsigned int > num( point.size() / 3, 2 );

	{
		auto const now = std::chrono::system_clock::now();
		auto const d = now.time_since_epoch();
		auto const d_s = std::chrono::duration_cast< std::chrono::seconds >( d );
		std::ofstream ofs( filename + ".clu" + std::to_string( MAX_CLUSTER_NUM ) + "." + std::to_string( d_s.count() ) + ".dat" );
		for( auto i = 0u; i < std::size( num ); ++i )
		{
			ofs << num[ i ] << std::endl;
		}
		ofs.flush();
	}

	std::vector< float > d_point;
	std::vector< std::vector< unsigned int > > d_index;
	std::tie( d_point, d_index ) = make_dual( point, index );
	std::vector< std::vector< float > > d_uv = calc_dual_uv( d_point, d_index );
	for( auto &&v : d_uv ) for( auto &&u : v ) u = (u + 15) / 30;

	// 点と点の距離の最短
	/*
	{
		std::vector< bool > vb( std::size( point ) / 3, false );
		for( auto &idx : index ) vb[ idx ] = true;
		auto dist = [ & ]( unsigned int const i, unsigned int const j )
		{
			return std::hypot( std::hypot( point[ i + 0 ] - point[ j + 0 ], point[ i + 1 ] - point[ j + 1 ] ), point[ i + 2 ] - point[ j + 2 ] );
		};
		float min_len = 0.0f;
		{
			float min = std::numeric_limits< float >::max();
			for( auto i = 0u; i + 2 < std::size( index ); i += 3 )
			{
				min = (std::min)( min, dist( index[ i + 0 ] * 3, index[ i + 1 ] * 3 ) );
				min = (std::min)( min, dist( index[ i + 1 ] * 3, index[ i + 2 ] * 3 ) );
				min = (std::min)( min, dist( index[ i + 2 ] * 3, index[ i + 0 ] * 3 ) );
			}
			std::cout << min << std::endl;
			min_len = min;
		}
		{
			std::vector< std::tuple< unsigned int, unsigned int, float > > vec;
			for( auto i = 0u; i + 2 < std::size( point ); i += 3 )
			{
				if( !vb[ i / 3 ]) continue;
				for( auto j = i + 3; j + 2 < std::size( point ); j += 3 )
				{
					if( !vb[ j / 3 ] ) continue;
					if( i == j ) continue;
					auto const d = dist( i, j );
					if( d < min_len ){
						vec.push_back( std::make_tuple( i / 3, j / 3, d ) );
					}
				}
			}
			std::sort( vec.begin(), vec.end(), []( auto a, auto b ){ return std::get< 2 >( a ) < std::get< 2 >( b ); } );
			for( auto &&d : vec ) std::cout << std::get< 0 >( d ) << ", " << std::get< 1 >( d ) << " : " << std::get< 2 >( d ) << std::endl;
		}
	}
	// */

	std::vector< std::vector< std::uint8_t > > texture_data;
	for( unsigned int i = 1u; i <= MAX_CLUSTER_NUM; ++i )
	{
		texture_data.emplace_back( make_single_texture( TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT, i, 0.6f, 0.2f ) );
		std::ostringstream oss;
		oss << "texture_" << i << ".bmp";
		writeBMP( oss.str().c_str(), TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT, &texture_data.back()[ 0 ] );
	}

	glfwInit();
	auto window = glfwCreateWindow( VIEW_WINDOW_WIDTH, VIEW_WINDOW_HEIGHT, "OK", nullptr, nullptr );
	glfwMakeContextCurrent( window );
	glewExperimental = true;
	glewInit();

	glViewport( 0, 0, VIEW_WINDOW_WIDTH, VIEW_WINDOW_HEIGHT );

	auto program = compile_shader( vertex_shader_dual_src, fragment_shader_dual_src );
	auto program2 = compile_shader( vertex_shader_dual_src2, fragment_shader_dual_src2 );

	glEnable( GL_DEPTH_TEST );

	std::vector< std::vector< float > > d_subpoint;
	for( auto &&v : d_index )
	{
		std::vector< float > subpoint;
		for( auto &&idx : v )
		{
			auto const i = idx * 3;
			subpoint.insert( subpoint.end(), d_point.begin() + i, d_point.begin() + i + 3 );
		}
		d_subpoint.emplace_back( std::move( subpoint ) );
	}
	std::vector< GLuint > point_buffer, index_buffer, uv_buffer;
	for( auto &&v : d_subpoint)
	{
		point_buffer.emplace_back( make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, v ) );
	}
	for( auto &&v : d_uv )
	{
		uv_buffer.emplace_back( make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, v ) );
	}
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	std::vector< GLuint > dot_texture;
	for( auto i = 1u; i <= MAX_CLUSTER_NUM; ++i )
	{
		dot_texture.emplace_back( make_gl_texture_2d( GL_RGB, TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, &texture_data[ i - 1 ][ 0 ] ) );
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
		glm::mat4 model = glm::rotate( glm::radians( static_cast< float >( i++ ) / 80.0f ), glm::vec3( 0.0, 1.0, 0.0 ) );
		glm::mat4 mvp = proj * view * model;
		glUseProgram( program );
		glUniformMatrix4fv( glGetUniformLocation( program, "Hmat" ), 1, GL_FALSE, &mvp[ 0 ][ 0 ] );
		for( auto i = 1u; i < MAX_DUAL_TEXTURE_NUM_OF_SHADER; ++i )
		{
			std::ostringstream oss;
			oss << "tex" << i;
			glUniform1i( glGetUniformLocation( program, oss.str().c_str() ), i - 1 );
		}

		glUseProgram( program2 );
		glUniformMatrix4fv( glGetUniformLocation( program2, "Hmat" ), 1, GL_FALSE, &mvp[ 0 ][ 0 ] );

		glEnableClientState( GL_VERTEX_ARRAY );

		glUseProgram( program );
		for( auto i = 0u; i < d_index.size(); ++i )
		{
			if( !d_index[ i ].empty() )
			{
				glEnableVertexAttribArray( 0 );
				glBindBuffer( GL_ARRAY_BUFFER, point_buffer[ i ] );
				glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );
				glEnableVertexAttribArray( 1 );
				glBindBuffer( GL_ARRAY_BUFFER, uv_buffer[ i ] );
				glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );
				glActiveTexture( GL_TEXTURE0 );
				glBindTexture( GL_TEXTURE_2D, dot_texture[ num[ i ] - 1 ] );
				glDrawArrays( GL_TRIANGLE_FAN, 0, static_cast< GLsizei >( std::size( d_subpoint[ i ] ) / 3 ) );
			}
		}
		/*
		glUseProgram( program2 );
		for( auto i = 0u; i < d_index.size(); ++i )
		{
			if( !d_index[ i ].empty() )
			{
				glEnableVertexAttribArray( 0 );
				glBindBuffer( GL_ARRAY_BUFFER, point_buffer[ i ] );
				glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );
				glEnableVertexAttribArray( 1 );
				glBindBuffer( GL_ARRAY_BUFFER, uv_buffer[ i ] );
				glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );
				glDrawArrays( GL_LINE_STRIP, 1, static_cast< GLsizei >( std::size( d_subpoint[ i ] ) / 3 ) - 1 );
			}
		}
		*/

		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glFlush();
		glfwSwapBuffers( window );

		/*
		if( i < 360 + 4 )
		{
			auto ptr = std::make_unique< std::uint8_t[] >( VIEW_WINDOW_WIDTH * VIEW_WINDOW_HEIGHT * 3 );
			glReadPixels( 0, 0, VIEW_WINDOW_WIDTH, VIEW_WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, ptr.get() );
			std::ostringstream oss;
			oss << i << ".bmp";
			writeBMP( oss.str().c_str(), VIEW_WINDOW_WIDTH, VIEW_WINDOW_HEIGHT, ptr.get() );
		}
		*/

		glfwPollEvents();
	}
	glfwTerminate();
#else
	// glm::mat4 proj = glm::perspective( glm::radians( 30.0f ), static_cast< float >( VIEW_WINDOW_WIDTH ) / VIEW_WINDOW_HEIGHT, 0.1f, 100.0f );
	// glm::mat4 view = glm::lookAt( glm::vec3( 3, 3, 3 ), glm::vec3( 0, 0, 0 ), glm::vec3( 0.0, 0.0, 1.0) );
	glm::mat4 proj = glm::perspective( glm::radians( 30.0f ), static_cast< float >( VIEW_WINDOW_WIDTH ) / VIEW_WINDOW_HEIGHT, 0.1f, 10000.0f );
	glm::mat4 view = glm::lookAt( glm::vec3( 1000, 1000, 1000 ), glm::vec3( 0, 0, 0 ), glm::vec3( 0.0, 0.0, 1.0) );
	glm::mat4 model = glm::mat4( 1.0f );

	std::vector< float > point;
	std::vector< unsigned int > index;
	std::tie( point, index ) = load_ply( "C:\\Users\\t2ladmin\\Downloads\\acvd1.1\\output_1.ply" );
	// std::tie( point, index ) = make_geodesic_dome_point( 3 );
	// std::tie( point, index ) = make_regular_pentakis_dodecahedron_point( 1 );
	std::cout << "面の数：" << index.size() / 3 << std::endl;
	std::cout << "点の数：" << point.size() / 3 << std::endl;
	std::cout << "辺の数：" << index.size() / 2 << std::endl;
	// std::vector< unsigned int > num = make_claster( static_cast< unsigned int >( point.size() / 3 ), 6, index );
	std::vector< unsigned int > num( point.size() / 3, 2 );
	auto texture_data = make_texture( TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT, num, point, 0.02f, 0.05f );
	writeBMP( "test.bmp", TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT, texture_data.data() );
	// std::tie( point, index ) = make_geodesic_dome_point( 8 );

	if( !IS_BALL ) set_cog_to_origin( point );

	std::vector< float > poler_point;
	std::vector< unsigned int > poler_index;
	std::vector< unsigned int > poler_point_to_point;
	if( IS_BALL ) std::tie( poler_point, poler_index ) = euclidean_to_theta_phi_of_poler_for_draw( point, index, &poler_point_to_point );


	std::vector< float > color( std::size( point ) / 3 * 3 );
	std::random_device rnd_dev;
	std::seed_seq seq = { rnd_dev(), rnd_dev(), rnd_dev() };
	std::mt19937_64 gen( seq );
	for( std::size_t i = 0u; i < color.size(); ++i )
	{
		color[ i ] = std::uniform_real_distribution< float >()( gen );
	}
	color.reserve( poler_point_to_point.size() * 3 );
	for( std::size_t i = color.size() / 3; i < poler_point_to_point.size(); ++i )
	{
		for( auto j = 0u; j < 3; ++j )
		{
			color.push_back( color[ poler_point_to_point[ i ] * 3 + j ] );
		}
	}

	glfwInit();
	auto window = glfwCreateWindow( VIEW_WINDOW_WIDTH, VIEW_WINDOW_HEIGHT, "OK", nullptr, nullptr );
	glfwMakeContextCurrent( window );
	glewExperimental = true;
	glewInit();

	glViewport( 0, 0, VIEW_WINDOW_WIDTH, VIEW_WINDOW_HEIGHT );

	auto program = compile_shader( IS_BALL ? vertex_shader_src : vertex_shader_no_ball_src, IS_BALL ? fragment_shader_src : fragment_shader_no_ball_src );
	auto program2 = compile_shader( IS_BALL ? vertex_shader_src : vertex_shader_no_ball_src, IS_BALL ? fragment_shader_src2 : fragment_shader_no_ball_src2 );

	glEnable( GL_DEPTH_TEST );

	auto point_buffer = make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, IS_BALL ? poler_point : point );
	auto index_buffer = make_gl_buffer( GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, IS_BALL ? poler_index : index );
	auto color_buffer = make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, color );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	auto dot_texture = make_gl_texture_2d( GL_RGB, TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, &texture_data[0] );
	glBindTexture( GL_TEXTURE_2D, dot_texture );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	auto window2 = glfwCreateWindow( TEX_WINDOW_WIDTH, TEX_WINDOW_HEIGHT, "OK2", nullptr, nullptr );
	glfwMakeContextCurrent( window2 );
	glViewport( 0, 0, TEX_WINDOW_WIDTH, TEX_WINDOW_HEIGHT );

	auto program3 = compile_shader( vertex_shader_src2, fragment_shader_src3 );
	auto program4 = compile_shader( vertex_shader_src2, fragment_shader_src4 );

	auto point_buffer2 = make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, poler_point );
	auto index_buffer2 = make_gl_buffer( GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, poler_index );
	auto color_buffer2 = make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, color );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	auto dot_texture2 = make_gl_texture_2d( GL_RGB, TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, &texture_data[0] );
	glBindTexture( GL_TEXTURE_2D, dot_texture2 );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	while( true )
	{
		if( glfwWindowShouldClose( window ) || glfwWindowShouldClose( window2 ) )
		{
			break;
		}
		
		glfwMakeContextCurrent( window );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		static unsigned int i = 0;
		glm::mat4 model = glm::rotate( glm::radians( static_cast< float >( i++ ) / 80.0f ), glm::vec3( 0.0, 0.0, 1.0 ) );
		glm::mat4 mvp = proj * view * model;
		glUseProgram( program );
		glUniformMatrix4fv( glGetUniformLocation( program, "Hmat" ), 1, GL_FALSE, &mvp[ 0 ][ 0 ] );
		glUniform1i( glGetUniformLocation( program, "image" ), 0 );
		glUseProgram( program2 );
		glUniformMatrix4fv( glGetUniformLocation( program2, "Hmat" ), 1, GL_FALSE, &mvp[ 0 ][ 0 ] );

		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, dot_texture );
		glEnableVertexAttribArray( 0 );
		glBindBuffer( GL_ARRAY_BUFFER, point_buffer );
		glVertexAttribPointer( 0, IS_BALL ? 2 : 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );
		glEnableVertexAttribArray( 1 );
		glBindBuffer( GL_ARRAY_BUFFER, color_buffer );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );
		glEnableClientState( GL_VERTEX_ARRAY );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
		glUseProgram( program );
		glDrawElements( GL_TRIANGLES, static_cast< GLsizei >( index.size() ), GL_UNSIGNED_INT, reinterpret_cast< void * >( 0 ) );
		// /*
		glUseProgram( program2 );
		for( auto i = 0u; i + 2 < index.size(); i += 3 )
		{
			glDrawElements( GL_LINE_LOOP, 3, GL_UNSIGNED_INT, reinterpret_cast< void * >( i * sizeof( index[ 0 ] ) ) );
		}
		// */
		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glFlush();
		glfwSwapBuffers( window );

		if( i < 360 + 4 )
		{
			auto ptr = std::make_unique< std::uint8_t[] >( VIEW_WINDOW_WIDTH * VIEW_WINDOW_HEIGHT * 3 );
			glReadPixels( 0, 0, VIEW_WINDOW_WIDTH, VIEW_WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, ptr.get() );
			std::ostringstream oss;
			oss << i << ".bmp";
			writeBMP( oss.str().c_str(), VIEW_WINDOW_WIDTH, VIEW_WINDOW_HEIGHT, ptr.get() );
		}

		if( i == 1 && IS_BALL )
		{
		glfwMakeContextCurrent( window2 );
		glClear( GL_COLOR_BUFFER_BIT );

		glUseProgram( program3 );
		glUniform1i( glGetUniformLocation( program3, "image" ), 0 );
		glUseProgram( program4 );

		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, dot_texture2 );
		glEnableVertexAttribArray( 0 );
		glBindBuffer( GL_ARRAY_BUFFER, point_buffer2 );
		glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );
		glEnableVertexAttribArray( 1 );
		glBindBuffer( GL_ARRAY_BUFFER, color_buffer2 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );
		glEnableClientState( GL_VERTEX_ARRAY );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer2 );
		glUseProgram( program3 );
		glDrawElements( GL_TRIANGLES, static_cast< GLsizei >( poler_index.size() ), GL_UNSIGNED_INT, reinterpret_cast< void * >( 0 ) );
		glUseProgram( program4 );
		for( auto i = 0u; i + 2 < poler_index.size(); i += 3 )
		{
			glDrawElements( GL_LINE_LOOP, 3, GL_UNSIGNED_INT, reinterpret_cast< void * >( i * sizeof( poler_index[ 0 ] ) ) );
		}
		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableVertexAttribArray( 0 );
		glDisableVertexAttribArray( 1 );
		glFlush();
		glfwSwapBuffers( window2 );
		}

		glfwPollEvents();
	}
	glfwTerminate();
#endif
}
