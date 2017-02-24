#pragma once

constexpr char vertex_shader_src[] =
R"(#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inuv;
uniform mat4 Hmat;
out vec2 outuv;

void main( void )
{
	gl_Position = Hmat * vec4( position.xyz, 1.0 );
	outuv = inuv;
}
)";
constexpr char fragment_shader_src1[] =
R"(#version 330 core

in vec2 outuv;
out vec4 color;
uniform sampler2D tex1;

void main( void )
{
	// color = vec4( 1.0, 1.0, 1.0, 1.0 );
	color = texture( tex1, outuv );
}
)";
constexpr char fragment_shader_src2[] =
R"(#version 330 core

in vec2 outuv;
out vec4 color;

void main( void )
{
	color = vec4( 0.0, 1.0, 0.0, 1.0 );
}
)";

