#version 440
// render

uniform mat4 model_matrix;
uniform mat4 projection_matrix;

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

layout (xfb_buffer = 0, xfb_offset=0) out vec4 world_space_position; // transform feedback 

out vec3 vs_fs_normal;

void main(void)
{
    world_space_position = (model_matrix * position);
    vs_fs_normal = normalize((model_matrix * vec4(normal, 0.0)).xyz);
    gl_Position = projection_matrix * world_space_position;
}
