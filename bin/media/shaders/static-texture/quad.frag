#version 330 core
//quad_shader
in vec2 tex_coord;

layout (location = 0) out vec4 color;

uniform sampler2D tex;

void main(void)
{
    color = texture(tex, tex_coord);
}