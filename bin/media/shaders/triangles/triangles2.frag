#version 450 core

in vec4 color;
out vec4 fColor;

void main()
{
    fColor = color; //vec4(0.5, 0.4, 0.8, 1.0);
}
