#version 410
// light model including ambiant, diffuse and reflected components 
layout (location = 0) out vec4 FragColor;

in vec3 vs_fs_normal;
in vec4 vs_fs_color;

void main(void)
{
    vec4 ambiantColor= vec4(0.51f, 0.59f, 0.69f, 1.0f); //could be a uniform and passed from the client application

    vec4 lightColor= vec4(0.54f, 0.67f, 0.24f, 1.0f);
    vec3 lightDirection= vec3(1.0,1.0,1.0);

    vec3 farlightDirection= vec3(0.0,0.0,1.0);

    vec3 normal= vec3(vs_fs_normal.x, vs_fs_normal.y, vs_fs_normal.z);

    float diffuse_coef= max(0.0, dot(normal,lightDirection));
    float specular_coef= max(0.0, dot(normal,farlightDirection));

    float shininess=40.0;
    if(diffuse_coef==0) specular_coef= 0.0;
    else specular_coef= pow(specular_coef,shininess);
    
    vec4 scatteredLight = ambiantColor + lightColor * diffuse_coef;
    float attenuation_coef=0.15;
    vec4 reflectedLight = lightColor * specular_coef * (1.0-attenuation_coef);

    FragColor = min(0.5*(scatteredLight+reflectedLight) , vec4(1.0));

}
