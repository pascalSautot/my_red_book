#version 440
// update

uniform mat4 model_matrix;
uniform mat4 projection_matrix;
uniform int triangle_count;

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 velocity;

layout (xfb_buffer = 0, xfb_offset=0) out vec4 position_out; // transform feedback 
layout (xfb_buffer = 0, xfb_offset=16) out vec3 velocity_out; // transform feedback 

uniform samplerBuffer geometry_tbo; // a handle for accessing a buffer texture
uniform float time_step = 0.02;

bool intersect(vec3 origin, vec3 destination, vec3 v0, vec3 v1, vec3 v2, out vec3 point)
{
    // the parametric equation of the line is : L= L1 + t (L2-L1)
    vec3 L1= origin;
    vec3 L2= destination;

    // the parametric equation of the ^plane is : v = v0 + u (v1-v0) + v (v2-v0)
    // where the line and the plane meet (intersect) v = L
    //   L1 + t a  = v0 + u b + v c 
    //   L1 - v0   = -t a + u b + v c
    // [ L1 - v0 ] = [ -a b c ][t,u,v]T 
    // [ L1 - v0 ] = [ _a b c ][t,u,v]T 
    vec3 a  = (L2 - L1);
    vec3 b =  (v1 - v0);
    vec3 c =  (v2 - v0);
    vec3 _a = -a;
    vec3 w0=(L1 - v0);
    // ps vector is (t,u,v)    
    // w0 = M ps
    // ps = inv(M) w0
    // inv(M) = 1/det(M) Adj(M)

    vec3 b_c=cross(b,c);
    vec3 c_a=cross(c,_a);
    vec3 a_b=cross(_a,b);

    float detM= dot(_a,b_c);
    if(detM==0.0) // line is // to the plane
        return false;
    mat3 M; //col row conventions
    for(int k=0;k<3;k++)
    {
        M[0][k]=b_c[k];
        M[1][k]=c_a[k];
        M[2][k]=a_b[k];
    }
    M=M/detM;
    mat3 MT=transpose(M);
    vec3 ps= MT*w0;
    float t=  ps[0];
    float u=  ps[1];
    float v=  ps[2];

    if( (t>1.0) || (t<0.0) )
        return false;
    if( (u>1.0) || (u<0.0) )
        return false;
    if( (v>1.0) || (v<0.0) )
        return false;
    point = L1 + t * a;
    return true;
}

vec3 reflect_vector(vec3 v, vec3 n)
{
    return v - 2.0 * dot(v, n) * n;
}

void main(void)
{
    //predict particules' position as if there was no collision
    vec3 accelleration = vec3(0.0, -0.3, 0.0);
    vec3 predicted_velocity = velocity + accelleration * time_step;
    vec4 predicted_position = position + vec4(predicted_velocity * time_step, 0.0);

    // given predcted position check for collisions
    vec3 new_velocity= predicted_velocity;
    vec4 new_position= predicted_position;    
    vec3 t_v0, t_v1, t_v2;
    vec3 intersection;
    int i;
    // explicit loop over triangle
    for (i = 0; i < triangle_count; i++)
    {
        //texelFetch() return a single texel from the texture buffer
        //get 3 triangle vertices coordinates from geometry_tbo
        t_v0 = texelFetch(geometry_tbo, i * 3).xyz;
        t_v1 = texelFetch(geometry_tbo, i * 3 + 1).xyz;
        t_v2 = texelFetch(geometry_tbo, i * 3 + 2).xyz;
        if (intersect(position.xyz, predicted_position.xyz, t_v0, t_v1, t_v2, intersection))
        {
            vec3 bounce = normalize(cross(t_v1 - t_v0, t_v2 - t_v0));
            new_position = vec4(intersection + reflect_vector(predicted_position.xyz - intersection, bounce), 1.0);
            new_velocity = 0.8 * reflect_vector(predicted_velocity, bounce);
        }
    }
    if (new_position.y < -40.0)
    {
        new_position = vec4(-new_position.x * 0.3, position.y + 80.0, 0.0, 1.0);
        new_velocity *= vec3(0.2, 0.1, -0.3);
    }
    velocity_out = new_velocity * 0.9999;
    position_out = new_position;
    gl_Position = projection_matrix * (model_matrix * position);
}
