#version 410

// Uniforms
uniform mat4 model_matrix;
uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform float time;
uniform float radius;

// Regular vertex attributes
layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
layout (location = 3) in vec4 instance_color;

// Instanced vertex attributes

// Instanced vertex attributes

// Outputs to the fragment shader
out vec3 vs_fs_normal;
out vec4 vs_fs_color;

void main(void)
{
    //float rs= radius*sin(time)/3.0;
    float rs= radius*cos(time);
    float rc= radius*cos(time)/4.0;
    float x=position.x*(1.0f+rc);
    float y=position.y*(1.0f+rs);
    float z=position.z*(1.0f+rc);

    vec4 pos = vec4(x,y,z,1.0f);


    // Use that calculated matrix to transform the object.
    mat4 model_view_matrix = view_matrix * model_matrix;    

    vec3 normal = normalize((model_view_matrix* vec4(vec3(normal), 0.0)).xyz);
    vs_fs_normal= vec3( model_view_matrix * vec4(vec3(normal),0.0));
    vs_fs_color = instance_color;
    gl_Position = projection_matrix * model_view_matrix * pos;
}
