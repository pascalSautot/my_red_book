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

bool intersect(vec3 origin, vec3 direction, vec3 v0, vec3 v1, vec3 v2, out vec3 point)
{
    vec3 u, v, n;
    vec3 w0, w;
    float r, a, b;

    u = (v1 - v0);
    v = (v2 - v0);
    n = cross(u, v);
            // "    if (length(n) < 0.1)
            // "        return false;

    w0 = origin - v0;
    a = -dot(n, w0);
    b = dot(n, direction);
            //"    if (abs(b) < 0.1)
            //"        return false;

    r = a / b;
    if (r < 0.0 || r > 1.0)
        return false;

    point = origin + r * direction;

    float uu, uv, vv, wu, wv, D;

    uu = dot(u, u);
    uv = dot(u, v);
    vv = dot(v, v);
    w = point - v0;
    wu = dot(w, u);
    wv = dot(w, v);
    D = uv * uv - uu * vv;

    float s, t;

    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)
        return false;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)
        return false;

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
        if (intersect(position.xyz, position.xyz - predicted_position.xyz, t_v0, t_v1, t_v2, intersection))
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
