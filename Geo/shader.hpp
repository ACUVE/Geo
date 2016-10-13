#pragma once

constexpr char vertex_shader_src[] =
R"(#version 330 core

// position.x : theta, position.y : phi
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 incolor;
uniform mat4 Hmat;
out vec3 vcolor;
out vec2 poler;

void main( void )
{
	float st = sin( position.x ), ct = cos( position.x );
	float sp = sin( position.y ), cp = cos( position.y );
	gl_Position = Hmat * vec4( ct * cp, st * cp, sp, 1 );
	vcolor = incolor;
	poler = position.xy;
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
	color = texture( image, vec2( poler.x / PI / 2 + 0.5, poler.y / PI + 0.5 ) );
}
)";
constexpr char fragment_shader_src2[] =
R"(#version 330 core

in vec3 vcolor;
in vec2 poler;
out vec4 color;

void main( void )
{
	color = vec4( 0.0, 1.0, 0.0, 1.0 );
}
)";
constexpr char vertex_shader_no_ball_src[] =
R"(#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 incolor;
uniform mat4 Hmat;
out vec3 vcolor;

void main( void )
{
	gl_Position = Hmat * vec4( position.xyz, 1.0 );
	vcolor = incolor;
}
)";
constexpr char fragment_shader_no_ball_src[] =
R"(#version 330 core

in vec3 vcolor;
out vec4 color;
uniform sampler2D image;

void main( void )
{
	color = vec4( 1.0, 1.0, 1.0, 1.0 );
}
)";
constexpr char fragment_shader_no_ball_src2[] =
R"(#version 330 core

in vec3 vcolor;
out vec4 color;

void main( void )
{
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

constexpr char vertex_shader_dual_src[] = 
R"(#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inuv;
uniform mat4 Hmat;
out vec2 outuv;

void main( void )
{
	gl_Position = Hmat * vec4( position, 1 );
	outuv = inuv;
}
)";
constexpr char fragment_shader_dual_src[] =
R"(#version 330 core

in vec2 outuv;
out vec4 color;
uniform sampler2D tex1;

void main( void )
{
	color = texture( tex1, outuv );
}
)";
constexpr unsigned int MAX_DUAL_TEXTURE_NUM_OF_SHADER = 6;
constexpr char vertex_shader_dual_src2[] = 
R"(#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inuv;
uniform mat4 Hmat;
out vec2 outuv;

void main( void )
{
	gl_Position = Hmat * vec4( position, 1 );
	gl_Position.z -= 0.001;
	outuv = inuv;
}
)";
constexpr char fragment_shader_dual_src2[] =
R"(#version 330 core

in vec3 outuv;
out vec4 color;

void main( void )
{
	color = vec4( 0.0, 1.0, 0.0, 1.0 );
}
)";