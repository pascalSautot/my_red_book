#version 440

//uniform    vec3 viewDirection;
uniform    vec3 halfWayDirection;
uniform    vec4 ambiantColor;
uniform    vec4 pointlightSourceColor;
uniform    vec4 pointLightSourcePosition;
uniform    vec4 farLightSourceColor;
uniform    vec3 farLightSourceDirection; 
//uniform    vec4 spotLightSourceColor;
//uniform    vec3 spotLightSourceDirection;     

layout (location = 0) out vec4 color;

in vec3 vs_fs_position;
in vec3 vs_fs_normal;
in vec4 vs_fs_color;

void main(void)
{
    vec3 direction= vec3(pointLightSourcePosition)-vs_fs_position;
    float distance =  length(direction);

    if(distance!=0)direction= direction / distance;

/*
    const float spotCosCutOff = 0.99f;
    float spotCos = dot(direction,-spotLightSourceDirection);
    float spotColorAttenuation = 1.0;
    if(spotCos < spotCosCutOff)
        spotColorAttenuation = 0.0;
*/
    // in lambertian reflection the light source is so far that all light rays have the same direction.
    // The direction is expressed oriented from the vertex stemming out of the mesh
    vec3 normal= vs_fs_normal;
    float diffuse_coef= max(0.0, dot(normal,farLightSourceDirection));

    float c1=0.5f,c2=1.0f,c3=1.f;
    float attenuation_coef= 1.0/( c1 + c2*distance + c3*pow(distance,2) );    

    float specular_reflection= max(0.0, dot(normal,halfWayDirection)); // Blinn Phong model - half is middle view and far light

    float shininess=40.0;
    if(diffuse_coef==0) specular_reflection= 0.0;
    else specular_reflection= pow(specular_reflection,shininess);
    
    vec4 lambertian_reflection =   ambiantColor + farLightSourceColor * diffuse_coef;  
    vec4 reflected_light = farLightSourceColor * specular_reflection + pointlightSourceColor * attenuation_coef;

    color = min((vs_fs_color * lambertian_reflection  + reflected_light ) , vec4(1.0));

}
