#version 450 core
layout (location = 0) in vec2 vertex;
uniform vec4 rect;
uniform vec2 scale;
out vec2 tex_coord;

void main()
{
	tex_coord = vertex * rect.zw;
	gl_Position = vec4(tex_coord * scale + rect.xy, 0.0, 1.0);
}

