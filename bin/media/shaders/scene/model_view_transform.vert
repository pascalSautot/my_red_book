#version 410

// Uniforms
uniform mat4 model_matrix;
uniform mat4 projection_matrix;
uniform mat4 view_matrix;

// Regular vertex attributes
layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

// Instanced vertex attributes
layout (location = 3) in vec4 instance_color;

// Outputs to the fragment shader
out vec3 vs_fs_normal;
out vec4 vs_fs_color;
out vec4 vertex_position;

void main(void)
{
    vec4 pos = position;

    // Use that calculated matrix to transform the object.
    mat4 model_view_matrix = view_matrix * model_matrix;    

    vs_fs_normal = normalize((model_view_matrix* vec4(normal, 0.0)).xyz);    
    vs_fs_color = instance_color;
    vertex_position = projection_matrix * model_view_matrix * pos;
    gl_Position = vertex_position;
}
