#version 430 core

layout (local_size_x = 1024) in;

layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D output_image;

shared vec4 scanline[1024];

void main(void)
{
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    scanline[pos.x] = imageLoad(input_image, pos);
    barrier();
    imageStore(output_image, pos.yx, scanline[min(pos.x + 1, 1023)] - scanline[max(pos.x - 1, 0)]);
}
