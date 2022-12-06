#version 440

in vec3 vs_fs_position;
in vec3 vs_fs_normal;
in vec4 vs_fs_color;

layout (location = 0) out vec4 color;

void main(void)
{
    vec3 p=vs_fs_position;
    vec4 farLightSourceColor= vec4(0.8, 0.9, 0.7, 1.0);
    color = vs_fs_color * (0.1 + abs(vs_fs_normal.z)) + farLightSourceColor * pow(abs(vs_fs_normal.z), 40.0);
}
