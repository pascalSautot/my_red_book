#version 410

// Uniforms
uniform mat4 model_matrix;
uniform mat4 projection_matrix;

// Regular vertex attributes
layout (location = 0) in vec4 position;

// Instanced vertex attributes
layout (location = 3) in vec4 instance_color;

// Outputs to the fragment shader
out vec4 vs_fs_color;

void main(void)
{
    vec4 pos = position;

    // Use that calculated matrix to transform the object.
    vs_fs_color = instance_color;
    gl_Position = projection_matrix * model_matrix * pos;
}
