#define NOMINMAX
#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
#include <tuple>
#include <cmath>
#include <random>
#include <unordered_map>
#include <iterator>
#include <windows.h>
#include <gl/glew.h>
#include <gl/wglew.h>
#include <gl/GL.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "make_point.hpp"

#pragma comment( lib, "glew32.lib" )
#pragma comment( lib, "glfw3.lib" )
#pragma comment( lib, "OpenGL32.lib" )
#pragma comment( lib, "glu32.lib" )

constexpr unsigned int VIEW_WINDOW_WIDTH = 500u, VIEW_WINDOW_HEIGHT = VIEW_WINDOW_WIDTH;
constexpr unsigned int TEX_WINDOW_WIDTH = 500u * 2, TEX_WINDOW_HEIGHT = TEX_WINDOW_WIDTH / 2;
constexpr unsigned int TEXTURE_WIDTH_HEIGHT = 2048u;

constexpr char vertex_shader_src[] =
R"(#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 incolor;
uniform mat4 Hmat;
out vec3 vcolor;
out vec2 poler;

void main( void )
{
	gl_Position = Hmat * vec4( position, 1 );
	vcolor = incolor;
	// poler = vec2( atan( position.y, position.x ), atan( position.z, length( position.xy ) ) );
	poler = vec2( atan( position.y, position.x ), atan( position.z, length( position.xy ) ) );
}
)";
constexpr char fragment_shader_src[] =
R"(#version 330 core

in vec3 vcolor;
in vec2 poler;
out vec4 color;
uniform sampler2D image;

const float PI = 3.141592653589793238462643383;

void main( void )
{
	// color = vec4( vcolor, 1.0 );
	color = texture( image, vec2( poler.x / PI / 2 + 0.5, poler.y / PI + 0.5 ) );
	// color = texture( image, vec2( gl_FragCoord.x / 500, gl_FragCoord.y / 250 ) );
}
)";
constexpr char fragment_shader_src2[] =
R"(#version 330 core

in vec3 vcolor;
in vec2 poler;
out vec4 color;

void main( void )
{
	// color = vec4( 1.0, 1.0, 1.0, 1.0 );
	color = vec4( 0.0, 1.0, 0.0, 1.0 );
}
)";

constexpr char vertex_shader_src2[] =
R"(#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 incolor;
out vec3 vcolor;
out vec2 poler;

const float PI = 3.141592653589793238462643383;

void main( void )
{
	float x = position.x / PI, y = position.y / PI * 2;
	// gl_Position = vec4( x * ( 1 - y * y ), y, 0.0, 1.0 );
	gl_Position = vec4( x, y, 0.0, 1.0 );
	vcolor = incolor;
	poler = position;
}
)";
constexpr char fragment_shader_src3[] =
R"(#version 330 core

in vec3 vcolor;
in vec2 poler;
out vec4 color;
uniform sampler2D image;

const float PI = 3.141592653589793238462643383;

void main( void )
{
	color = texture( image, vec2( poler.x / PI / 2 + 0.5, poler.y / PI + 0.5 ) );
	// color = vec4( vcolor, 1.0 );
	/*
	float x = float( gl_FragCoord.x - 500 ) / 500;
	float y = float( gl_FragCoord.y - 250 ) / 250;
	if( abs( x ) >= 1 - y * y )
		discard;
	*/
}
)";
constexpr char fragment_shader_src4[] =
R"(#version 330 core

in vec3 vcolor;
in vec2 poler;
out vec4 color;

void main( void )
{
	/*
	float x = float( gl_FragCoord.x - 500 ) / 500;
	float y = float( gl_FragCoord.y - 250 ) / 250;
	if( abs( x ) >= 1 - y * y )
		discard;
	*/
	// color = vec4( 1.0, 1.0, 1.0, 1.0 );
	color = vec4( 0.0, 1.0, 0.0, 1.0 );
}
)";

static void writeBMP(char const *const filename, std::uint32_t const width, std::uint32_t const height, void *ptr){
	HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if(hFile == INVALID_HANDLE_VALUE) return;
	std::uint32_t const pixel_num = width * height;
	std::uint32_t const filesize = 14 + 40 + pixel_num * 3;
#define UINT16TO2BYTE(NUM) (std::uint16_t)(NUM) & 0xFF, ((std::uint16_t)(NUM) >> 8) & 0xFF
#define UINT32TO4BYTE(NUM) (std::uint32_t)(NUM) & 0xFF, ((std::uint32_t)(NUM) >> 8) & 0xFF, ((std::uint32_t)(NUM) >> 16) & 0xFF, ((std::uint32_t)(NUM) >> 24) & 0xFF
	DWORD written;
	std::uint8_t const header[14] =
	{
		'B', 'M',
		UINT32TO4BYTE(filesize),
		UINT16TO2BYTE(0),
		UINT16TO2BYTE(0),
		UINT32TO4BYTE(14 + 40)
	};
	WriteFile(hFile, header, sizeof(header), &written, nullptr);
	std::uint8_t const infoheader[40] =
	{
		UINT32TO4BYTE(40),
		UINT32TO4BYTE(width),
		UINT32TO4BYTE(~height + 1),
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
	assert((width * 3) % 8 == 0);
	WriteFile(hFile, ptr, pixel_num * 3, &written, nullptr);
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
	glBufferData( type, sizeof( vec[ 0 ] ) * std::size( vec ), &vec[ 0 ], usage );
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

int main( int argc, char **argv )
{
	glm::mat4 proj = glm::perspective( glm::radians( 30.0f ), static_cast< float >( VIEW_WINDOW_WIDTH ) / VIEW_WINDOW_HEIGHT, 0.1f, 100.0f );
	glm::mat4 view = glm::lookAt( glm::vec3( 3, 3, 3 ), glm::vec3( 0, 0, 0 ), glm::vec3( 0.0, 0.0, 1.0) );
	glm::mat4 model = glm::mat4( 1.0f );

	std::vector< float > point;
	std::vector< unsigned int > index;
	std::tie( point, index ) = make_geodesic_dome_point( 1 );
	// std::tie( point, index ) = make_regular_pentakis_dodecahedron_point( 0 );
	std::cout << "面の数：" << index.size() / 3 << std::endl;
	std::cout << "点の数：" << point.size() / 3 << std::endl;
	std::cout << "辺の数：" << index.size() / 2 << std::endl;
	std::vector< unsigned int > num = make_claster( static_cast< unsigned int >( point.size() / 3 ), 4, index );
	// std::vector< unsigned int > num( point.size() / 3, 2 );
	auto texture_data = make_texture( TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT, num, point, 0.02f, 0.05f );
	writeBMP( "test.bmp", TEXTURE_WIDTH_HEIGHT, TEXTURE_WIDTH_HEIGHT, texture_data.data() );
	// std::tie( point, index ) = make_geodesic_dome_point( 4 );


	std::vector< float > poler_point;
	std::vector< unsigned int > poler_index;
	std::vector< unsigned int > poler_point_to_point;
	std::tie( poler_point, poler_index ) = euclidean_to_theta_phi_of_poler_for_draw( point, index, &poler_point_to_point );

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

	auto program = compile_shader( vertex_shader_src, fragment_shader_src );
	auto program2 = compile_shader( vertex_shader_src, fragment_shader_src2 );

	glEnable( GL_DEPTH_TEST );

	auto point_buffer = make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, point );
	auto index_buffer = make_gl_buffer( GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, index );
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
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );
		glEnableVertexAttribArray( 1 );
		glBindBuffer( GL_ARRAY_BUFFER, color_buffer );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );
		glEnableClientState( GL_VERTEX_ARRAY );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
		glUseProgram( program );
		glDrawElements( GL_TRIANGLES, static_cast< GLsizei >( index.size() ), GL_UNSIGNED_INT, reinterpret_cast< void * >( 0 ) );
		/*
		glUseProgram( program2 );
		for( auto i = 0u; i + 2 < index.size(); i += 3 )
		{
			glDrawElements( GL_LINE_LOOP, 3, GL_UNSIGNED_INT, reinterpret_cast< void * >( i * sizeof( index[ 0 ] ) ) );
		}
		*/
		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glFlush();
		glfwSwapBuffers( window );





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

		glfwPollEvents();
	}
	glfwTerminate();
}
