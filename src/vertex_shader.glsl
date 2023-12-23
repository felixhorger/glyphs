#version 450 core
layout (location = 0) in vec2 vertex;
uniform vec2 pos;
uniform float scale;
out vec2 tex_coord;

void main()
{
	gl_Position = vec4(vertex * scale + pos, 0.0, 1.0);
	tex_coord = vertex;
}

