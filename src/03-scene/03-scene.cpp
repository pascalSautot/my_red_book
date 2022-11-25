/* $URL$
   $Rev$
   $Author$
   $Date$
   $Id$
 */
#include "vapp.h"
#include "vutils.h"

#include "vmath.h"

#include "vbm.h"
#include "mesh.h"


#include <stdio.h>
#include <iostream>
#include "LoadShaders.h"
using namespace vmath;
using namespace std;

BEGIN_APP_DECLARATION(InstancingExample)
    // Override functions from base class
    virtual void Initialize(const char * title);
    virtual void Display(bool auto_redraw);    
    virtual void Finalize(void);
    virtual void Resize(int width, int height);

    // Member variables
    GLuint vao[2];
    GLuint vbo[2];
    GLuint xfb;

    //camera params 
    GLfloat camera_aspect_ratio;

    GLfloat camera_tx,camera_ty,camera_tz;
    GLfloat camera_rx,camera_ry,camera_rz;
    GLfloat frustrum_z_near_plane,frustrum_z_far_plane;
     
    // armadillo VBM object
    GLuint color_vbo;
    GLuint geometry_tex;
    GLuint geometry_xfb,particle_xfb;

    VBObject object;
    GLuint armadillo_prog;
    GLint a_view_matrix_loc,a_model_matrix_loc,a_projection_matrix_loc;

    // base planes and axes
    SimpleObject ref_planes;
    GLuint ref_prog;
    GLint ref_view_matrix_loc,ref_model_matrix_loc,ref_projection_matrix_loc;

    // ponctual lights as sphere
    mesh::Sphere sphere_light;
    GLuint sphere_prog;
    GLint s_model_matrix_loc,s_view_matrix_loc,s_projection_matrix_loc,s_time_loc,s_radius_loc;
    GLfloat s_time,s_radius;

    // robot as cylinders and axes
    mesh::Cylinder cylinder;
    GLuint cylinder_prog;
    GLint c_model_matrix_loc,c_view_matrix_loc,c_projection_matrix_loc,c_time_loc,c_radius_loc,c_heigth_loc;
    GLfloat c_time,c_radius,c_heigth;    
    vmath::vec4 robotBaseLocation;
    vmath::vec4 robotColor;
    
    // light models
    vmath::vec3 viewDirection;
    vmath::vec3 halfWayDirection;

    vmath::vec4 ambiantColor;
    vmath::vec4 pointlightSourceColor;
    vmath::vec4 pointLightSourcePosition;

    //far directionnal light source is assimilated to parallel rays of light
    vmath::vec4 farLightSourceColor;
    vmath::vec3 farLightSourceDirection;

    vmath::vec4 spotLightSourceColor;
    vmath::vec3 spotLightSourceDirection;


    GLint viewDirection_loc;
    GLint halfWayDirection_loc;

    GLint ambiantColor_loc;
    GLint pointlightSourceColor_loc;
    GLint pointLightSourcePosition_loc;

    GLint farLightSourceColor_loc;
    GLint farLightSourceDirection_loc; 

    GLint spotLightSourceColor_loc;
    GLint spotLightSourceDirection_loc;

    vmath::vec4 a_lightColor, a_ambiantColor;
    vmath::vec4 a_lightPosition;
    vmath::vec3 a_lightDirection;


    enum { vPositionLoc=0, vNormalIndexLoc=1, vTextureLoc=2, vInstanceColorLoc=3};    
    
protected:
    virtual bool InitArmadillo(void);
    virtual bool InitRef(void);
    virtual bool InitSphere(void);
    virtual bool InitCylinder(void);
    virtual bool BuildShaders(ShaderInfo  *shaders, GLuint &prog, GLint &prog_model_matrix_loc, GLint &prog_view_matrix_loc, GLint &prog_projection_matrix_loc);
    virtual void ClearDisplay(void);
    virtual void DisplayRef(bool auto_redraw, GLfloat t, vmath::mat4 &view_matrix,  vmath::mat4 &projection_matrix);
    virtual void DisplayArmadillo(bool auto_redraw, GLfloat t, vmath::mat4 &view_matrix, vmath::mat4 &projection_matrix);
    virtual void DisplaySphere(bool auto_redraw, GLfloat t, vmath::mat4 &view_matrix,  vmath::mat4 & projection_matrix);
    virtual void DisplayCylinder(bool auto_redraw, GLfloat t, vmath::mat4 &view_matrix,  vmath::mat4 & projection_matrix);
    virtual bool InstancingExample::FetchUniformVariables(GLint prog);
END_APP_DECLARATION()

DEFINE_APP(InstancingExample, "Instancing Example")

#undef DO_ARMADILLO
#define INSTANCE_COUNT 1

#include "mesh.h"
mesh::XAxis x_axis;
mesh::YAxis y_axis;
mesh::ZAxis z_axis;
mesh::XYPlane xy_plane;
mesh::YZPlane yz_plane;
mesh::ZXPlane zx_plane;


bool InstancingExample::BuildShaders(ShaderInfo  *shaders, GLuint &prog,
    GLint &prog_model_matrix_loc, GLint &prog_view_matrix_loc, GLint &prog_projection_matrix_loc)
{
    bool status(false);
    prog = LoadShaders( shaders);
    status=( prog != 0);
    if( !status) 
    { 
        std::cerr<<"shader program failed to build" << std::endl; 
    }
    else
    {
        glUseProgram( prog );

    // "model_matrix" is actually an array of 4 matrices
        prog_model_matrix_loc = glGetUniformLocation(prog, "model_matrix");
        prog_view_matrix_loc= glGetUniformLocation(prog, "view_matrix");
        prog_projection_matrix_loc = glGetUniformLocation(prog, "projection_matrix");

        if(prog_model_matrix_loc == -1 ){
            std::cout << "model matrix not fetched in shader" << std::endl;
            status=false;
        }
        
        if(prog_view_matrix_loc == -1){
            std::cout << "scale matrix not fetched in shader" << std::endl;
            status=false;
        }
        if(prog_projection_matrix_loc == -1){
            std::cout << "projection matrix not fetched in shader" << std::endl;
            status=false;
        }           
    }
    return status;
}
void InstancingExample::Initialize(const char * title)
{
    base::Initialize(title);
    camera_tx=  0.0f;
    camera_ty=  0.0f;
    camera_tz= -2.5f;
    camera_rx= 0.0f;
    camera_ry= 0.0f;
    camera_rz= 0.0f;
    frustrum_z_near_plane=0.5f;
    frustrum_z_far_plane=5.0f;

    bool armadillo_status=InitArmadillo();
    bool ref_status=InitRef();
    bool s_status=InitSphere();
    bool c_status=InitCylinder();
    if( ! (armadillo_status&&ref_status&&s_status&&c_status) ) exit(0);
}
size_t TypeSize(GLenum type)
{
    size_t size;
#define CASE(Enum, Count, Type) \
    case Enum: size = Count * sizeof(Type); break
    
    switch (type) {
        CASE(GL_FLOAT, 1, GLfloat);
        CASE(GL_FLOAT_VEC2, 2, GLfloat);
        CASE(GL_FLOAT_VEC3, 3, GLfloat);
        CASE(GL_FLOAT_VEC4, 4, GLfloat);
        CASE(GL_INT, 1, GLint);
        CASE(GL_INT_VEC2, 2, GLint);
        CASE(GL_INT_VEC3, 3, GLint);
        CASE(GL_INT_VEC4, 4, GLint);
        CASE(GL_UNSIGNED_INT, 1, GLuint);
        CASE(GL_UNSIGNED_INT_VEC2, 2, GLuint);
        CASE(GL_UNSIGNED_INT_VEC3, 3, GLuint);
        CASE(GL_UNSIGNED_INT_VEC4, 4, GLuint);
        CASE(GL_BOOL, 1, GLboolean);
        CASE(GL_BOOL_VEC2, 2, GLboolean);
        CASE(GL_BOOL_VEC3, 3, GLboolean);
        CASE(GL_BOOL_VEC4, 4, GLboolean);
        CASE(GL_FLOAT_MAT2, 4, GLfloat);
        CASE(GL_FLOAT_MAT2x3, 6, GLfloat);
        CASE(GL_FLOAT_MAT2x4, 8, GLfloat);
        CASE(GL_FLOAT_MAT3, 9, GLfloat);
        CASE(GL_FLOAT_MAT3x2, 6, GLfloat);
        CASE(GL_FLOAT_MAT3x4, 12, GLfloat);
        CASE(GL_FLOAT_MAT4, 16, GLfloat);
        CASE(GL_FLOAT_MAT4x2, 8, GLfloat);
        CASE(GL_FLOAT_MAT4x3, 12, GLfloat);
#undef CASE
        default:
            fprintf(stderr, "Unknown type: 0x%x\n", type);
            exit(EXIT_FAILURE);
        break;
    }
    return size;
}
bool InstancingExample::FetchUniformVariables(GLint prog)
{
    bool status(true);


    ambiantColor_loc= glGetUniformLocation(armadillo_prog, "ambiantColor");
    pointlightSourceColor_loc = glGetUniformLocation(armadillo_prog, "pointlightSourceColor");
    pointLightSourcePosition_loc = glGetUniformLocation(armadillo_prog, "pointLightSourcePosition");
    farLightSourceColor_loc = glGetUniformLocation(armadillo_prog, "farLightSourceColor");
    farLightSourceDirection_loc = glGetUniformLocation(armadillo_prog, "farLightSourceDirection");
    viewDirection_loc = glGetUniformLocation(armadillo_prog, "viewDirection");
    /*
    halfWayDirection_loc = glGetUniformLocation(armadillo_prog, "halfWayDirection");
    spotLightSourceColor_loc= glGetUniformLocation(armadillo_prog, "spotLightSourceColor");
    spotLightSourceDirection_loc = glGetUniformLocation(armadillo_prog, "spotLightSourceDirection"); 
    */

    if(ambiantColor_loc == -1 ){
        std::cout << "ambiantColor not fetched in shader" << std::endl;
        status=false;
    }
    if(pointlightSourceColor_loc == -1 ){
        std::cout << "pointlightSourceColor not fetched in shader" << std::endl;
        status=false;
    }
    if(pointLightSourcePosition_loc == -1 ){
        std::cout << "pointLightSourcePosition not fetched in shader" << std::endl;
        status=false;
    }
    if(farLightSourceColor_loc == -1 ){
        std::cout << "farLightSourceColor not fetched in shader" << std::endl;
        status=false;
    }
    if(farLightSourceDirection_loc == -1 ){
        std::cout << "farLightSourceDirection not fetched in shader" << std::endl;
        status=false;
    }
    if(viewDirection_loc == -1 ){
        std::cout << "viewDirection not fetched in shader" << std::endl;
        status=false;
    }   
    /*
    if(halfWayDirection_loc == -1 ){
        std::cout << "halfWayDirection not fetched in shader" << std::endl;
        status=false;
    }   
    if(spotLightSourceColor_loc == -1 ){
        std::cout << "spotLightSourceColor not fetched in shader" << std::endl;
        status=false;
    } 
    if(spotLightSourceDirection_loc == -1 ){
        std::cout << "spotLightSourceDirection not fetched in shader" << std::endl;
        status=false;
    }   
    */                           
    return status;
}
bool InstancingExample::InitArmadillo()
{
    // Create the program for rendering the model
    // load shaders in program
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/scene/model_view_transform.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/scene/lighting.frag" },
        { GL_NONE, NULL }
    };
    bool status=InstancingExample::BuildShaders(shaders,armadillo_prog,a_model_matrix_loc,a_view_matrix_loc,a_projection_matrix_loc);
    if(status)
    {
        ambiantColor= vmath::vec4(0.0f, 0.2f, 0.2f, 1.0f); // dark yellow
        ambiantColor= vmath::vec4(0.2f, 0.2f, 0.2f, 1.0f); // dark 

        pointlightSourceColor= vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f); // blue
        pointLightSourcePosition= vmath::vec4(0.f,1.5f,-camera_tz/5.0f,1.0);
        pointLightSourcePosition= vmath::vec4(0.f,1.f,0.0f,1.0);

        farLightSourceColor= vmath::vec4(0.1f,205.0/255.0,128.0/255.0,1.0f);  // sun shine color
        farLightSourceColor= vmath::vec4(0.1f,0.1f,0.1f,1.0f);  // dark
        farLightSourceDirection= vmath::vec3(1.0,1.0,0.5);  // expressed from the vertex 

        viewDirection=vmath::vec3(0.0, 0.0, -1.0);   // camera related 
        halfWayDirection= vmath::normalize(viewDirection+farLightSourceDirection);

        spotLightSourceColor= vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f); // blue
        spotLightSourceDirection = vmath::vec3(1.0,1.0,0.5); 

        // in the blinn phong relfection model a half vector is defined 
        // as being half way between view and far light
        // https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model

       
        FetchUniformVariables(armadillo_prog);

        if(status)
        {
            // Load the object
            object.LoadFromVBM("media/armadillo_low.vbm", vPositionLoc,vNormalIndexLoc, vTextureLoc);

            object.BindVertexArray();       

            // Generate the colors of the objects
            vmath::vec4 colors[INSTANCE_COUNT];

            for (int n = 0; n < INSTANCE_COUNT; n++)
            {
                float a = float(n) / 4.0f;
                float b = float(n) / 5.0f;
                float c = float(n) / 6.0f;

                colors[n][0] = 0.5f * (sinf(a + 1.0f) + 1.0f);
                colors[n][1] = 0.5f * (sinf(b + 2.0f) + 1.0f);
                colors[n][2] = 0.5f * (sinf(c + 3.0f) + 1.0f);
                colors[n][3] = 1.0f;
            }

            // Same with the instance color array
            glGenBuffers(1, &color_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

            glVertexAttribDivisor(vInstanceColorLoc, 1);
            glVertexAttribPointer(vInstanceColorLoc, 4, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(vInstanceColorLoc);

            // Done (unbind the object's VAO)
            glBindVertexArray(0);
        }        
    }
    return status;
}
bool InstancingExample::InitRef(void)
{
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/scene/model_view_transform.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/scene/no-lighting.frag" },
        { GL_NONE, NULL }
    };

    bool status=InstancingExample::BuildShaders(shaders,
        ref_prog,ref_model_matrix_loc,ref_view_matrix_loc,ref_projection_matrix_loc);    

    if(status){
        x_axis.setData();
        y_axis.setData();
        z_axis.setData();
        xy_plane.setData();
        yz_plane.setData();
        zx_plane.setData();       
    }
    return status;
}
bool InstancingExample::InitSphere()
{
    // Create the program for rendering the model
    // load shaders in program
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/scene/model_view_transform_sphere.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/scene/no-lighting.frag" },
        { GL_NONE, NULL }
    };
    
    bool status=InstancingExample::BuildShaders(shaders,sphere_prog,s_model_matrix_loc,s_view_matrix_loc,s_projection_matrix_loc);
    if(status){
        s_time_loc = glGetUniformLocation(sphere_prog, "time");
        if(s_time_loc == -1 ){
            std::cout << "time not fetched in shader" << std::endl;
            status=false;
        }
        s_radius_loc = glGetUniformLocation(sphere_prog, "radius");
        if(s_radius_loc == -1 ){
            std::cout << "radius not fetched in shader" << std::endl;
            status=false;
        }        
    }
    if(status){
        sphere_light.setColor(pointlightSourceColor);
        sphere_light.setPosition(pointLightSourcePosition);
        sphere_light.setRadius(0.25f); //1.0f/16.0f);
        sphere_light.setData();
    }

    return status;    
}
bool InstancingExample::InitCylinder()
{
    // Create the program for rendering the model
    // load shaders in program
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/scene/model_view_transform_cylinder.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/scene/no-lighting.frag" },
        { GL_NONE, NULL }
    };
    
    bool status=InstancingExample::BuildShaders(shaders,
                    cylinder_prog,
                    c_model_matrix_loc,c_view_matrix_loc,c_projection_matrix_loc);
    if(status){
        
        std::string name="time";
        c_time_loc = glGetUniformLocation(cylinder_prog, name.c_str());
        if(c_time_loc == -1 ){
            std::cout << name << " not fetched in shader" << std::endl;
            status=false;
        }
        name = "radius";
        c_radius_loc = glGetUniformLocation(cylinder_prog, name.c_str());
        if(c_radius_loc == -1 ){
            std::cout << name << " not fetched in shader" << std::endl;
            status=false;
        }   
        name="heigth";
        c_heigth_loc = glGetUniformLocation(cylinder_prog, name.c_str());
        if(c_heigth_loc == -1 ){
            std::cout << name << " not fetched in shader" << std::endl;
            status=false;
        }             
    }
    if(status){
        robotBaseLocation= { 1.0f, 0.f, 0.f, 1.0f};
        robotColor= { 0.8f, 0.8f, 0.8f, 1.0f};
        cylinder.setColor(robotColor);
        cylinder.setPosition(robotBaseLocation);
        cylinder.setRadius(0.25f); 
        cylinder.setHeigth(0.5f); 
        cylinder.setData();
    }

    return status;    
}
void InstancingExample::ClearDisplay(void)
{
    // Clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}
void InstancingExample::DisplayArmadillo(
    bool auto_redraw,GLfloat t, vmath::mat4 &view_matrix, vmath::mat4 &projection_matrix)
{      
    // Activate instancing program
    glUseProgram(armadillo_prog);

    glUniform4fv(ambiantColor_loc,1,ambiantColor);
    glUniform4fv(pointlightSourceColor_loc,1,pointlightSourceColor);
    glUniform4fv(farLightSourceColor_loc,1,farLightSourceColor);

    //glUniform3fv(halfWayDirection_loc,1,halfWayDirection);

    float tt= (3000.0f*t);

    const VBM_BOUNDING_BOX & bb= object.BoundingBox();
    const float l(bb.max[1]-bb.min[1]);
    //const float scale=(bb.l_max!=0 ? 1.0f/bb.l_max : 1.0f);
    const float scale=(l!=0 ? 1.0f/l : 1.0f);

    // Set four model matrices
    vmath::mat4 model_matrix=vmath::mat4::identity();
    //const float tx(scale*(bb.max[0]-bb.min[0]));
    const float tx(1.0);
    const float ty(std::abs(scale*bb.min[1]));
    //const float tz(scale*(bb.max[2]-bb.min[2]));
    const float tz(1.0);

    model_matrix =  vmath::translate(tx, ty, tz) *
                    vmath::rotate( 10*tt+0.0f , 0.0f, 1.0f, 0.0f) * 
                    vmath::scale(scale);        // put the beast on the corner of XY plane patch 
                                                // rotate around beast Y axis
                                                // scale so that the beast will fit a unit side box 
    /* 
    const int itt= ((int)tt)%360;
    float rtt= (float)itt;    
    vmath::vec3 eye(0.0f, 0.0f, 0.0f);
    vmath::vec4 eye4(0.0f, 0.0f, 50.0f, 1.0f);
    //rotate in x-z plane around the object placed by at
    vmath::vec3 up(0.0f, 1.0f, 0.0f);
    vmath::vec3 at(0.0f, 0.0f, 50.0f);
    mat4 r=vmath::rotate(rtt, 0.0f, 1.0f, 0.0f);
    for(int i =0;i<3;i++) 
    {
        float s=0;
        for(int k =0;k<3;k++) 
            s += r[i][k]* eye4[k];
        eye[i]=s;
    }

    for(int i =0;i<3;i++) eye[i]= eye[i]  - at[i];
    view_matrix =  vmath::lookat(eye,at,up);*/

    vmath::vec4 pLSP= pointLightSourcePosition * view_matrix;
    vmath::vec4 fLSD= { farLightSourceDirection[0], farLightSourceDirection[1], farLightSourceDirection[2], 0.0f};
    fLSD= fLSD * view_matrix;
    vmath::vec3 fLSD3;
    for(int k=0;k<3;k++)fLSD3[k]=fLSD[k];
/*
    vmath::vec4 vD= { viewDirection[0], viewDirection[1], viewDirection[2], 0.0f};
    vD= vD * view_matrix;
    vmath::vec3 vD3;
    for(int k=0;k<3;k++)vD3[k]=vD[k];
*/
    glUniform4fv(pointLightSourcePosition_loc,1,pointLightSourcePosition);
    glUniform3fv(farLightSourceDirection_loc,1,fLSD3);
    glUniform3fv(viewDirection_loc,1,viewDirection);
/*
    glUniform4fv(pointLightSourcePosition_loc,1,pLSP);
    glUniform3fv(farLightSourceDirection_loc,1,farLightSourceDirection);
    glUniform3fv(viewDirection_loc,1,viewDirection);
*/    
    // pass on matrices 
    glUniformMatrix4fv(a_model_matrix_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(a_view_matrix_loc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(a_projection_matrix_loc, 1, GL_FALSE, projection_matrix);

    // Render INSTANCE_COUNT objects
    object.Render(0, INSTANCE_COUNT);

    glUseProgram(0);

}
void InstancingExample::DisplaySphere(bool auto_redraw,GLfloat t, vmath::mat4 &view_matrix,  vmath::mat4 & projection_matrix)
{
    // Activate instancing program
    glUseProgram(sphere_prog);

    //projection_matrix=(vmath::frustum(left_side,right_side, bottom, top, near_plane, far_plane) * vmath::translate(0.0f, 0.0f, -1000.0f));
    GLfloat scale(sphere_light.Radius());
    vmath::vec3 translation(sphere_light.Position());
    vmath::mat4 model_matrix =  vmath::mat4::identity() *
                    vmath::translate(translation) *
                    vmath::scale(scale);  
    // pass on matrices 
    glUniformMatrix4fv(s_model_matrix_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(s_view_matrix_loc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(s_projection_matrix_loc, 1, GL_FALSE, projection_matrix);
    float tt= (1000.0f*t);

    glUniform1f(s_time_loc, tt);
    glUniform1f(s_radius_loc, sphere_light.Radius());

    //render axes
    sphere_light.render();

    glUseProgram(0);
}

void InstancingExample::DisplayCylinder(bool auto_redraw,GLfloat t, vmath::mat4 &view_matrix,  vmath::mat4 & projection_matrix)
{
    // Activate instancing program
    glUseProgram(cylinder_prog);

    //projection_matrix=(vmath::frustum(left_side,right_side, bottom, top, near_plane, far_plane) * vmath::translate(0.0f, 0.0f, -1000.0f));
    GLfloat scale(cylinder.Radius());
    vmath::vec3 translation(cylinder.Position());
    vmath::mat4 model_matrix =  vmath::mat4::identity() *
                    vmath::translate(translation) *
                    vmath::scale(scale);  
    // pass on matrices 
    glUniformMatrix4fv(c_model_matrix_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(c_view_matrix_loc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(c_projection_matrix_loc, 1, GL_FALSE, projection_matrix);
    float tt= (1000.0f*t);

    glUniform1f(c_time_loc, tt);
    glUniform1f(c_radius_loc, cylinder.Radius());
    glUniform1f(c_heigth_loc, cylinder.Heigth());

    //render axes
    cylinder.render();

    glUseProgram(0);
}

void InstancingExample::DisplayRef(bool auto_redraw, float t, vmath::mat4 &view_matrix,  vmath::mat4 & projection_matrix)
{
    // Activate instancing program
    glUseProgram(ref_prog);

    //projection_matrix=(vmath::frustum(left_side,right_side, bottom, top, near_plane, far_plane) * vmath::translate(0.0f, 0.0f, -1000.0f));
    vmath::mat4 model_matrix=vmath::mat4::identity();//rotate(20.0f, 1.0f, 0.0f, 0.0f)*vmath::rotate(-20.0f, 0.0f, 1.0f, 1.0f);

    // pass on matrices 
    glUniformMatrix4fv(ref_model_matrix_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(ref_view_matrix_loc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(ref_projection_matrix_loc, 1, GL_FALSE, projection_matrix);

    //render axes
    x_axis.render();
    y_axis.render();
    z_axis.render();
    // render planes
    xy_plane.render();
    yz_plane.render();
    zx_plane.render();     

    glUseProgram(0);
}
void InstancingExample::Display(bool auto_redraw)
{
    float t = float(app_time() & 0x3FFFF) / float(0x3FFFF);
    float tt= (9000.0f*t);

    ClearDisplay();    

    const float view_angle(5.0f); // slightly tilt the scene to view all 3 planes

    vmath::mat4 top_view_matrix =   vmath::rotate(90.0f, 1.0f, 0.0f, 0.0f)*
                                    vmath::rotate(-tt/2.0f, 0.0f, 1.0f, 0.0f) *
                                    vmath::rotate(view_angle, 1.0f, 0.0f, 0.0f) *
                                    vmath::rotate(-view_angle, 0.0f, 1.0f, 1.0f) ;  

    vmath::mat4 side_view_matrix =   
                                vmath::rotate(-tt/2.0f, 0.0f, 1.0f, 0.0f) *
                                vmath::rotate(view_angle, 1.0f, 0.0f, 0.0f) *
                                vmath::rotate(-view_angle, 0.0f, 1.0f, 1.0f);  // rotate around Y axis
    
    const float right   = 1.0f;
    const float left    = -right;
    const float top     = camera_aspect_ratio;
    const float bottom  = -top;

    vmath::mat4 projection_matrix= 
        vmath::frustum(  
            left, right, bottom, top, 
            frustrum_z_near_plane,frustrum_z_far_plane) * 
        vmath::translate(camera_tx,camera_ty,camera_tz);   

    vmath::mat4 &view_matrix= side_view_matrix;
    DisplayArmadillo(   auto_redraw,t,view_matrix,projection_matrix);
    DisplayRef(         auto_redraw,t,view_matrix,projection_matrix);
    DisplaySphere(      auto_redraw,t,view_matrix,projection_matrix);
    DisplayCylinder(    auto_redraw,t,view_matrix,projection_matrix);

    base::Display();
}
void InstancingExample::Finalize(void)
{
    glUseProgram(0);
    if(ref_prog)glDeleteProgram(ref_prog);
    if(armadillo_prog)glDeleteProgram(armadillo_prog);
    if(sphere_prog)glDeleteProgram(sphere_prog);
    if(cylinder_prog)glDeleteProgram(cylinder_prog);
    glDeleteVertexArrays(2, vao);
    glDeleteBuffers(2, vbo);
}

void InstancingExample::Resize(int width, int height)
{
    const int w(width), h(height);
    glViewport(0, 0 , w, h);
    camera_aspect_ratio = float(h) / float(w);
}
