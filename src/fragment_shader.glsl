#version 450 core
in vec2 tex_coord;
out vec4 color;

uniform sampler2DArray text;
uniform vec4 text_color;
uniform int char;

void main()
{
    color = text_color * vec4(1.0, 1.0, 1.0, texture(text, vec3(tex_coord, char)).r);
}
