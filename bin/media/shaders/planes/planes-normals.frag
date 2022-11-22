#version 410

layout (location = 0) out vec4 color;

in vec3 vs_fs_normal;
in vec4 vs_fs_color;

void main(void)
{
    color = vs_fs_color ;
};
