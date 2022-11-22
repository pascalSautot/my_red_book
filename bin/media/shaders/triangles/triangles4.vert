
#version 400 core
uniform mat4 model_matrix;
uniform mat4 scale_matrix;
uniform mat4 projection_matrix;


layout( location = 0 ) in vec4 vPosition;
layout( location = 1 ) in vec4 vColor;
out vec4 color;

void
main()
{
    color= vColor;
    gl_Position = projection_matrix*scale_matrix * (model_matrix * vPosition);
}