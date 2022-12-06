#version 440 core

layout (location = 0) in vec3 in_position;

out vec3 tex_coord;

uniform mat4 model_view_camera;

void main(void)
{
    gl_Position = model_view_camera * vec4(in_position, 1.0);
    tex_coord = in_position;
}
 