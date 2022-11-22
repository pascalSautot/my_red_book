#version 410

uniform vec3 viewDirection;
uniform vec4 ambiantColor;
uniform vec4 pointlightSourceColor;
uniform vec4 pointLightSourcePosition;
uniform vec4 farLightSourceColor;
uniform vec3 farLightSourceDirection; 

layout (location = 0) out vec4 FragColor;

in vec4 vertex_position;
in vec3 vs_fs_normal;
in vec4 vs_fs_color;

void main(void)
{
    float dummy=dot(farLightSourceColor,(pointlightSourceColor-pointLightSourcePosition));
    dummy= dot(farLightSourceDirection,viewDirection);

    vec3 direction= vec3(pointLightSourcePosition)-vec3(vertex_position);
    float distance =  length(direction);

    if(distance!=0)direction= direction / distance;

    // in lambertian reflection the light source is so far that all light rays have the same direction.
    // The direction is expressed oriented from the vertex stemming out of the mesh
    vec3 normal= vs_fs_normal;
    float diffuse_coef= max(0.0, dot(normal,farLightSourceDirection));

    float c1=10.0,c2=1.0,c3=1.0;
    float attenuation_coef= 1.0/( c1 + c2*distance + c3*pow(distance,2) );    

    float specular_reflection= max(0.0, dot(normal,viewDirection));

    float shininess=40.0;
    if(diffuse_coef==0) specular_reflection= 0.0;
    else specular_reflection= pow(specular_reflection,shininess);
    
    vec4 lambertian_reflection =   farLightSourceColor * diffuse_coef;  
    FragColor = min((vs_fs_color *( ambiantColor  + specular_reflection) + lambertian_reflection) , vec4(1.0));

   // FragColor = min(vs_fs_color * (0.1 + diffuse_coef) + ambiantColor * specular_reflection, vec4(1.0));

}
