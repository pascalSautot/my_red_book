#version 410

//uniform    vec3 viewDirection;
uniform    vec3 halfWayDirection;
uniform    vec4 ambiantColor;
uniform    vec4 pointlightSourceColor;
uniform    vec4 pointLightSourcePosition;
uniform    vec4 farLightSourceColor;
uniform    vec3 farLightSourceDirection; 
//uniform    vec4 spotLightSourceColor;
//uniform    vec3 spotLightSourceDirection;     

layout (location = 0) out vec4 FragColor;

in vec4 vertex_position;
in vec3 vs_fs_normal;
in vec4 vs_fs_color;

void main(void)
{
    vec3 x=halfWayDirection;
    vec3 normal= normalize(vs_fs_normal);

    vec3 direction= vec3(vertex_position)-vec3(pointLightSourcePosition);
    direction= normalize(direction);
    
    //Emissive model
    vec4 Ki=vs_fs_color; 
    FragColor = Ki; //the most simple model
    //Ambiant model
    vec4 Ka=FragColor; 
    vec4 Ia=ambiantColor; 
    vec4 Emissivecolor=  Ia*Ka; 

    //directionnal light (far and diffuse over the object surface)
    vec4 If=farLightSourceColor;
    vec4 Kd=vs_fs_color;
    vec3 N=normal;
    vec3 L= normalize(farLightSourceDirection);
    float Fatt= 1.0/5.0;
    vec4 FarDiffuseColor = Emissivecolor + Fatt*If*Kd*dot(N,L);

    // specular refelxion
    float shininess=10.0f;
    vec3 PS=normalize(direction);
    float shine= pow(dot(normal,PS),shininess); // Blinn Phong model - half is middle view and far light
    vec4 Ip=pointlightSourceColor;

    //vec4 SpecularColor= normalize(Ip*dot(direction,N)*shine);
    vec4 SpecularColor= Ip*shine;
    SpecularColor= FarDiffuseColor + SpecularColor;

    //FragColor = min(FarDiffuseColor,vec4(1.0)); 
    FragColor = min(SpecularColor,vec4(1.0)); 

}
