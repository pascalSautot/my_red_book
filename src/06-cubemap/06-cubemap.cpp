/* $URL$
   $Rev$
   $Author$
   $Date$
   $Id$
 */

#include "vermilion.h"

#include "vapp.h"
#include "vutils.h"
#include "vbm.h"

#include "vmath.h"

#include <stdio.h>
#include <iostream>

#include "LoadShaders.h"
#define INSTANCE_COUNT 1

BEGIN_APP_DECLARATION(CubeMapExample)
    // Override functions from base class
    virtual void Initialize(const char * title);
    virtual void Display(bool auto_redraw);
    virtual void Finalize(void);
    virtual void Resize(int width, int height);

    protected:
        bool InitRef(void);
        void InitTorus(void);
        void InitCubeMap(void);
        bool InitArmadillo(void);

        void ClearDisplay(void);

        void DisplayCubeMap(
            bool auto_redraw, float tick, vmath::mat4& view_matrix, vmath::mat4& projection_matrix);   
        void DisplayTorus(
            bool auto_redraw, float tick, vmath::mat4& view_matrix, vmath::mat4& projection_matrix, GLfloat z_trans);  
        void DisplayRef(bool auto_redraw, float tick, vmath::mat4 &view_matrix,  vmath::mat4 & projection_matrix);
        void DisplayArmadillo(
            bool auto_redraw,GLfloat tick, vmath::mat4 &view_matrix, vmath::mat4 &projection_matrix);
    // Member variables
    GLuint skybox_prog;
    GLuint vao;

    GLuint cube_vbo;
    GLuint cube_element_buffer;

    GLuint tex;
    GLint skybox_model_view_camera_loc;

    GLint torus_model_view_camera_mat_loc;
    GLint torus_model_view_mat_loc;

    GLuint torus_prog;
    VBObject torus;

    // base planes and axes
    SimpleObject ref_planes;
    GLuint ref_prog;
    GLint ref_view_matrix_loc,ref_model_matrix_loc,ref_projection_matrix_loc;  
    GLboolean display_ref_planes;  

    //armadillo
    VBObject armadillo;
    GLuint armadillo_prog;
    GLint a_model_view_matrix_loc,a_model_view_projection_matrix_loc;    
    GLuint a_color_vbo;
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

    // camera settings
    float camera_aspect_ratio;
    float right;
    float left;
    float top;
    float bottom;
    float near_plane;
    float far_plane;    
    float camera_tx;
    float camera_ty;
    float camera_tz;
    float torus_z;     
END_APP_DECLARATION()

DEFINE_APP(CubeMapExample, "Cube Map Example")


#include "mesh.h"
mesh::XAxis x_axis;
mesh::YAxis y_axis;
mesh::ZAxis z_axis;
mesh::XYPlane xy_plane;
mesh::YZPlane yz_plane;
mesh::ZXPlane zx_plane;

bool CubeMapExample::InitRef(void)
{
    display_ref_planes=false;
    if(!display_ref_planes)return true;

    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/scene/model_view_transform.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/scene/no-lighting.frag" },
        { GL_NONE, NULL }
    };
    ref_prog = LoadShaders( shaders );
    GLboolean status= ( ref_prog != 0);
    if( !status) { std::cerr<<"ref shader program failed to build" << std::endl; }
    else
    {
        ref_model_matrix_loc = glGetUniformLocation(ref_prog, "model_matrix");
        ref_view_matrix_loc= glGetUniformLocation(ref_prog, "view_matrix");
        ref_projection_matrix_loc = glGetUniformLocation(ref_prog, "projection_matrix");

        if(ref_model_matrix_loc == -1 ){
            std::cout << "model matrix not fetched in shader" << std::endl;
            status=false;
        }
        
        if(ref_view_matrix_loc == -1){
            std::cout << "scale matrix not fetched in shader" << std::endl;
            status=false;
        }
        if(ref_projection_matrix_loc == -1){
            std::cout << "projection matrix not fetched in shader" << std::endl;
            status=false;
        } 
    }

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
void CubeMapExample::InitTorus(void)
{
    // load shaders in program
    ShaderInfo  object_shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/cubemap/object.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/cubemap/torus.frag" },
        { GL_NONE, NULL }
    };

    torus_prog = LoadShaders( object_shaders );
    GLboolean status= ( torus_prog != 0);
    if( !status) { std::cerr<<"object shader program failed to build" << std::endl; }
    else {
        torus_model_view_camera_mat_loc = glGetUniformLocation(torus_prog, "model_view_camera");
        if(torus_model_view_camera_mat_loc == -1 ){
            std::cout << "model_view_camera not fetched in shader" << std::endl;
            status=false;
        }        
        torus_model_view_mat_loc = glGetUniformLocation(torus_prog, "model_view");        
        if(torus_model_view_mat_loc == -1 ){
            std::cout << "model_view not fetched in shader" << std::endl;
            status=false;
        }
    }

    if(!status) exit(0);
    enum vbm_ids { vertexVBMId=0, normalVBMId=1, textureVBMId=2, VBMIdMax};
    torus.LoadFromVBM("media/torus.vbm", vertexVBMId, normalVBMId, textureVBMId);
    torus.BindVertexArray();
}

#define SIMPLE
bool CubeMapExample::InitArmadillo(void)
{
    enum { vPositionLoc=0, vNormalIndexLoc=1, vTextureLoc=2, vInstanceColorLoc=3};  
    // Create the program for rendering the model
    // load shaders in program
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/cubemap/object.vert" },
#ifdef SIMPLE
        { GL_FRAGMENT_SHADER, "media/shaders/cubemap/simple.frag" },
#else
        { GL_FRAGMENT_SHADER, "media/shaders/cubemap/armadillo.frag" },
#endif        
        { GL_NONE, NULL }
    };
    armadillo_prog = LoadShaders( shaders );
    GLboolean status= ( armadillo_prog != 0);
    if( !status) { std::cerr<<"armadillo shader program failed to build" << std::endl; }
    else {    
        ambiantColor= vmath::vec4(0.0f, 0.2f, 0.2f, 1.0f); // dark yellow
        ambiantColor= vmath::vec4(0.2f, 0.2f, 0.2f, 1.0f); // dark 

        pointlightSourceColor= vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f); // blue
        pointLightSourcePosition= vmath::vec4(0.f,0.75f,0.0f,1.0);

        farLightSourceColor= vmath::vec4(0.1f,205.0/255.0,128.0/255.0,1.0f);  // sun shine color
        farLightSourceColor= vmath::vec4(0.1f,0.1f,0.1f,1.0f);  // dark
        farLightSourceDirection= vmath::vec3(1.0,1.0,0.5);  // expressed from the vertex 

        viewDirection=vmath::vec3(0.0, 0.0, -1.0);   // camera related 
        // in the blinn phong relfection model a half vector is defined 
        // as being half way between view and far light
        // https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model        
        halfWayDirection= vmath::normalize(viewDirection+farLightSourceDirection);

        spotLightSourceColor= vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f); // blue
        spotLightSourceDirection = vmath::vec3(1.0,1.0,0.5); 
#ifdef SIMPLE
#else
        ambiantColor_loc= glGetUniformLocation(armadillo_prog, "ambiantColor");
        pointlightSourceColor_loc = glGetUniformLocation(armadillo_prog, "pointlightSourceColor");
        pointLightSourcePosition_loc = glGetUniformLocation(armadillo_prog, "pointLightSourcePosition");
        farLightSourceColor_loc = glGetUniformLocation(armadillo_prog, "farLightSourceColor");
        farLightSourceDirection_loc = glGetUniformLocation(armadillo_prog, "farLightSourceDirection");
        halfWayDirection_loc = glGetUniformLocation(armadillo_prog, "halfWayDirection");
        /*
        viewDirection_loc = glGetUniformLocation(armadillo_prog, "viewDirection");
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
        if(halfWayDirection_loc == -1 ){
            std::cout << "halfWayDirection not fetched in shader" << std::endl;
            status=false;
        }  
#endif               
        /*
        if(viewDirection_loc == -1 ){
            std::cout << "viewDirection not fetched in shader" << std::endl;
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
        a_model_view_matrix_loc= glGetUniformLocation(armadillo_prog, "model_view");
        a_model_view_projection_matrix_loc = glGetUniformLocation(armadillo_prog, "model_view_camera");

        if(a_model_view_matrix_loc == -1){
            std::cout << "view matrix not fetched in shader" << std::endl;
            status=false;
        }
        if(a_model_view_projection_matrix_loc == -1){
            std::cout << "projection matrix not fetched in shader" << std::endl;
            status=false;
        }                                  
        if(status)
        {
            // Load the object
            status=armadillo.LoadFromVBM("media/armadillo_low.vbm", vPositionLoc,vNormalIndexLoc, vTextureLoc);
            armadillo.BindVertexArray();       

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
            glGenBuffers(1, &a_color_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, a_color_vbo);
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

const GLint vertexCount=8;
const GLint VDim=3;
static GLfloat cube_vertices[vertexCount*VDim] =
{
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f
};
const GLuint stripDim=2;
const GLuint vertexIndexDim=vertexCount*stripDim;
static const GLushort cube_indices[vertexIndexDim] =
{
    0, 1, 2, 3, 6, 7, 4, 5,         // First strip
    2, 6, 0, 4, 1, 5, 3, 7          // Second strip
};
void CubeMapExample::InitCubeMap(void)
{
    // load shaders in program
    ShaderInfo  skybox_shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/cubemap/skybox.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/cubemap/skybox.frag" },
        { GL_NONE, NULL }
    };

    skybox_prog = LoadShaders( skybox_shaders );
    GLboolean status= ( skybox_prog != 0);
    if( !status ) { std::cerr<<"skybox shader program failed to build" << std::endl; }
    else {
        skybox_model_view_camera_loc = glGetUniformLocation(skybox_prog, "model_view_camera");
        if(skybox_model_view_camera_loc == -1 ){
            std::cout << "model_view_camera not fetched in shader" << std::endl;
            status=false;
        }
    }
    if(!status) exit(0);

    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);

    const GLint count(vertexCount*VDim);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    const GLint attribSize=VDim; 
    const GLboolean normalize= GL_FALSE;
    const GLsizei stride=0;
    GLuint offset=0;
    const GLint att_type=GL_FLOAT;
    
    // vec3 in_position in shader
    enum skybox_ids { vertexSBId=0, SBIdMax};
    
    glVertexAttribPointer(vertexSBId, attribSize, att_type, normalize, stride, BUFFER_OFFSET(offset));
    glEnableVertexAttribArray(vertexSBId);

    glGenBuffers(1, &cube_element_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    vglImageData image;
    tex = vglLoadTexture("media/TantolundenCube.dds", 0, &image);

    GLenum e;
    e = glGetError();

    vglUnloadImage(&image);
}
void CubeMapExample::Initialize(const char * title)
{
    right   = 1.0f;
    left    = -right;
    top     = camera_aspect_ratio;
    bottom  = -top;
    near_plane  = 0.1f;
    far_plane  = 500.0f;    
    camera_tx=  0.0f;
    camera_ty=  0.0f;
    camera_tz=  0.0f;
    torus_z= 0;     
    base::Initialize(title);
    InitCubeMap();
    InitTorus();
    InitRef();
    InitArmadillo();
}
void CubeMapExample::ClearDisplay(void)
{
    // Clear
    glClearColor(0.0f, 0.25f, 0.3f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
}

void CubeMapExample::Display(bool auto_redraw)
{
    ClearDisplay();

    static const unsigned int start_time = app_time();
    float tick = float((app_time() - start_time)) / float(0x3FFF);

    const float view_angle(5.0f); // slightly tilt the scene to view all 3 planes
    const float angle(-tick * 70.0f * 3.0f);

    static const vmath::vec3 X(1.0f, 0.0f, 0.0f);
    static const vmath::vec3 Y(0.0f, 1.0f, 0.0f);
    static const vmath::vec3 Z(0.0f, 0.0f, 1.0f);
    vmath::mat4 top_view_matrix =   vmath::rotate(90.0f, X)*
                                    vmath::rotate(angle, Y) *
                                    vmath::rotate(view_angle, Z) *
                                    vmath::rotate(-view_angle, Y) ;  

    vmath::mat4 side_view_matrix =   
                                vmath::rotate(angle, Y) *
                                vmath::rotate(view_angle, X) *
                                vmath::rotate(-view_angle, Y);  // rotate around Y axis

    vmath::mat4 frustrum_projection_matrix= 
        vmath::frustum(  
            left, right, bottom, top, 
            near_plane,far_plane) * 
            vmath::translate(camera_tx,camera_ty,camera_tz);  

    const float field_of_view_angle=70.0f;
    vmath::mat4 perspective_projection_matrix= 
        vmath::perspective(field_of_view_angle, 1.0f / camera_aspect_ratio, near_plane, far_plane);

    vmath::mat4 & projection_matrix= perspective_projection_matrix;

    vmath::mat4 &view_matrix= side_view_matrix;

    DisplayCubeMap(     auto_redraw,tick,   view_matrix,        projection_matrix);
    DisplayTorus(       auto_redraw,tick,   view_matrix,        projection_matrix,  torus_z);
    DisplayRef(         auto_redraw,tick,   view_matrix,        projection_matrix);
    DisplayArmadillo(   auto_redraw,tick,   view_matrix,        projection_matrix);

    base::Display();
}

void CubeMapExample::DisplayCubeMap(
        bool auto_redraw, float tick, vmath::mat4& view_matrix, vmath::mat4& projection_matrix)
{
    vmath::mat4 model_view_projection_matrix(vmath::mat4::identity());
    vmath::mat4 model_matrix(vmath::mat4::identity());
    model_view_projection_matrix = projection_matrix * view_matrix * model_matrix;

    glUseProgram(skybox_prog);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glUniformMatrix4fv(skybox_model_view_camera_loc, 1, GL_FALSE, model_view_projection_matrix);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_element_buffer);

    GLenum indexType=GL_UNSIGNED_SHORT;
    GLuint indexSize=sizeof(GLushort);
    GLuint offset=vertexCount * indexSize;
    //first strip 
    glDrawElements(GL_TRIANGLE_STRIP, vertexCount, indexType, NULL);
    //second strip
    glDrawElements(GL_TRIANGLE_STRIP, vertexCount, indexType, BUFFER_OFFSET(offset));
}

void CubeMapExample::DisplayArmadillo(
    bool auto_redraw,GLfloat tick, vmath::mat4 &view_matrix, vmath::mat4 &projection_matrix)
{      
    // Activate instancing program
    glUseProgram(armadillo_prog);

    //float tt= (3000.0f*tick);

    const VBM_BOUNDING_BOX & bb= armadillo.BoundingBox();
    const float l(bb.max[1]-bb.min[1]);
    const float scale1=(l!=0 ? 1.0f/l : 1.0f);
    const float scale4=scale1/4.0f;
    const float scale=scale4;

    // Set four model matrices
    vmath::mat4 model_matrix=vmath::mat4::identity();
    const float tx(0.0);
    const float ty(0.0); //std::abs(scale*bb.min[1]));
    const float tz(-0.5);

    model_matrix =  vmath::translate(tx, ty, tz); 
                   /* vmath::rotate(   10*tick+0.0f , 0.0f, 1.0f, 0.0f) */ 
                    vmath::scale(    scale);        // put the beast on the corner of XY plane patch 
                                                // rotate around beast Y axis
                                                // scale so that the beast will fit a unit side box 
    vmath::mat4 model_view_matrix=view_matrix*model_matrix;

    vmath::vec4 pLSP= pointLightSourcePosition * view_matrix;
    vmath::vec4 fLSD= { farLightSourceDirection[0], farLightSourceDirection[1], farLightSourceDirection[2], 0.0f};
    fLSD= fLSD * view_matrix;
    vmath::vec3 fLSD3;
    for(int k=0;k<3;k++)fLSD3[k]=fLSD[k];

    vmath::vec4 vD= { viewDirection[0], viewDirection[1], viewDirection[2], 0.0f};
    vD= vD * view_matrix;
    vmath::vec3 vD3;
    for(int k=0;k<3;k++)vD3[k]=vD[k];

#ifdef SIMPLE
#else
    /*
    uniform    vec3 halfWayDirection;
    uniform    vec4 ambiantColor;
    uniform    vec4 pointlightSourceColor;
    uniform    vec4 pointLightSourcePosition;
    uniform    vec4 farLightSourceColor;
    uniform    vec3 farLightSourceDirection; 
    */
    glUniform3fv(halfWayDirection_loc,1,halfWayDirection);
    glUniform4fv(ambiantColor_loc,1,ambiantColor);
    glUniform4fv(pointlightSourceColor_loc,1,pointlightSourceColor);
    glUniform4fv(pointLightSourcePosition_loc,1,pLSP);
    glUniform4fv(farLightSourceColor_loc,1,farLightSourceColor);
    glUniform3fv(farLightSourceDirection_loc,1,fLSD3);
    //glUniform3fv(viewDirection_loc,1,vD3);
#endif
    
    // pass on matrices 
    glUniformMatrix4fv(a_model_view_matrix_loc, 1, GL_FALSE, model_view_matrix);
    glUniformMatrix4fv(a_model_view_projection_matrix_loc, 1, GL_FALSE, projection_matrix);

    // Render INSTANCE_COUNT objects
    armadillo.Render(0, INSTANCE_COUNT);

    glUseProgram(0);
}

void CubeMapExample::DisplayTorus(
        bool auto_redraw, float tick, vmath::mat4& view_matrix, vmath::mat4& projection_matrix, GLfloat z_trans)    
{
    static const vmath::vec3 X(1.0f, 0.0f, 0.0f);
    static const vmath::vec3 Y(0.0f, 1.0f, 0.0f);
    static const vmath::vec3 Z(0.0f, 0.0f, 1.0f);

    const VBM_BOUNDING_BOX & bb= torus.BoundingBox();
    const float l((bb.max[1]-bb.min[1]));
    const float scale1=(l!=0 ? 1.0f/l : 1.0f);
    const float scale4=scale1/4.0f;
    const float scale=scale4;
    const float tx(0.0);
    const float ty(0.0); //std::abs(scale*bb.min[1]));
    const float tz(0.5);

    glUseProgram(torus_prog);

    const float angle(70.0f * 3.0f * tick);
    vmath::mat4 model_matrix = vmath::translate(tx, ty, tz) *
    // vmath::translate(vmath::vec3(0.0f, 0.0f, z_trans)) * 
                vmath::rotate( -angle, Y) *
                vmath::rotate(  angle, X) *
                vmath::rotate(  angle, Z) *
                vmath::scale(   scale);
    vmath::mat4 model_view_matrix= view_matrix * model_matrix;
    vmath::mat4 model_view_projection_matrix = projection_matrix * model_view_matrix;

    glUniformMatrix4fv(torus_model_view_mat_loc, 1, GL_FALSE, model_view_matrix);
    glUniformMatrix4fv(torus_model_view_camera_mat_loc, 1, GL_FALSE, model_view_projection_matrix);

    torus.Render();
}
void CubeMapExample::DisplayRef(bool auto_redraw, float tick, vmath::mat4 &view_matrix,  vmath::mat4 & projection_matrix)
{
    if(!display_ref_planes)return;

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
    xy_plane.render();
    yz_plane.render();
    zx_plane.render();       

    glUseProgram(0);
}
void CubeMapExample::Finalize(void)
{
    glUseProgram(0);
    if(skybox_prog)glDeleteProgram(skybox_prog);
    if(torus_prog)glDeleteProgram(torus_prog);
    if(ref_prog && display_ref_planes)glDeleteProgram(ref_prog);
    if(armadillo_prog)glDeleteProgram(armadillo_prog);
    glDeleteTextures(1, &tex);
    glDeleteVertexArrays(1, &tex);
}

void CubeMapExample::Resize(int width, int height)
{
    glViewport(0, 0 , width, height);
    
    camera_aspect_ratio = float(height) / float(width);
}
