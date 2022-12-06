#version 440 core

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec3 in_normal;

// Instanced vertex attributes
layout (location = 3) in vec4 instance_color;

out vec3 vs_fs_position;
out vec3 vs_fs_normal;
out vec4 vs_fs_color;

uniform mat4 model_view_camera;
uniform mat4 model_view;

void main(void)
{
    gl_Position = model_view_camera * in_position;
    vs_fs_normal = mat3(model_view) * in_normal;
    vs_fs_position = (model_view * in_position).xyz;
    vs_fs_color= instance_color;
}