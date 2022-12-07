#version 410

in vec4 vs_fs_color;

layout (location = 0) out vec4 color;

//vec4 white= vec4(1.0);

void main(void)
{
    color = vs_fs_color; //white;
}
