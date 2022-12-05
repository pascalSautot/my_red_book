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
        void ClearDisplay(void);

        void DisplayCubeMap(
            bool auto_redraw, float tick, vmath::mat4& view_matrix, vmath::mat4& projection_matrix);   
        void DisplayTorus(
            bool auto_redraw, float tick, vmath::mat4& view_matrix, vmath::mat4& projection_matrix, GLfloat z_trans);  
        void DisplayRef(bool auto_redraw, float t, vmath::mat4 &view_matrix,  vmath::mat4 & projection_matrix);

    // Member variables
    float camera_aspect_ratio;
    GLuint skybox_prog;
    GLuint vao;

    GLuint cube_vbo;
    GLuint cube_element_buffer;

    GLuint tex;
    GLint skybox_rotate_loc;

    GLint torus_model_view_projection_mat_loc;
    GLint torus_model_view_mat_loc;

    GLuint torus_prog;
    VBObject torus;

    // base planes and axes
    SimpleObject ref_planes;
    GLuint ref_prog;
    GLint ref_view_matrix_loc,ref_model_matrix_loc,ref_projection_matrix_loc;  
    GLboolean display_ref_planes;  

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
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/scene/model_view_transform.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/scene/no-lighting.frag" },
        { GL_NONE, NULL }
    };
    ref_prog = LoadShaders( shaders );
    GLboolean status= ( ref_prog != 0);
    if( !status) { std::cerr<<"object shader program failed to build" << std::endl; }
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
        { GL_FRAGMENT_SHADER, "media/shaders/cubemap/object.frag" },
        { GL_NONE, NULL }
    };

    torus_prog = LoadShaders( object_shaders );
    GLboolean status= ( torus_prog != 0);
    if( !status) { std::cerr<<"object shader program failed to build" << std::endl; }
    else {
        torus_model_view_projection_mat_loc = glGetUniformLocation(torus_prog, "mat_mvp");
        if(torus_model_view_projection_mat_loc == -1 ){
            std::cout << "mat_mvp not fetched in shader" << std::endl;
            status=false;
        }        
        torus_model_view_mat_loc = glGetUniformLocation(torus_prog, "mat_mv");        
        if(torus_model_view_mat_loc == -1 ){
            std::cout << "mat_mv not fetched in shader" << std::endl;
            status=false;
        }
    }

    if(!status) exit(0);
    torus.LoadFromVBM("media/torus.vbm", 0, 1, 2);
    torus.BindVertexArray();
}
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
        skybox_rotate_loc = glGetUniformLocation(skybox_prog, "tc_rotate");
        if(skybox_rotate_loc == -1 ){
            std::cout << "tc_rotate not fetched in shader" << std::endl;
            status=false;
        }
    }
    if(!status) exit(0);

    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);

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

    static const GLushort cube_indices[] =
    {
        0, 1, 2, 3, 6, 7, 4, 5,         // First strip
        2, 6, 0, 4, 1, 5, 3, 7          // Second strip
    };
    const float scale=20.0f;
    const GLint count(vertexCount*VDim);
    //for(int i=0;i<count;i++)cube_vertices[i] *= scale;
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    const GLint attribSize=VDim; 
    const GLboolean normalize= GL_FALSE;
    const GLsizei stride=0;
    GLuint offset=0;
    const GLint att_type=GL_FLOAT;
    //VertexCoordID=0
    glVertexAttribPointer(0, attribSize, att_type, normalize, stride, BUFFER_OFFSET(offset));
    glEnableVertexAttribArray(0);

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
    base::Initialize(title);
    InitCubeMap();
    InitTorus();
    InitRef();
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
    /*
    float t = float(app_time() & 0x3FFFF) / float(0x3FFFF);
    float tt= (9000.0f*t);
    */
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

    /*
    const float fovy  = 35.0f;
    const float near_plane  = 0.1f;
    const float far_plane  = 500.0f;
    vmath::mat4 projection_matrix= vmath::perspective(fovy, 1.0f / camera_aspect_ratio, near_plane, far_plane); 
    */

    const float right   = 1.0f;
    const float left    = -right;
    const float top     = camera_aspect_ratio/2.0f;
    const float bottom  = -top;
    const float near_plane  = -0.14f;
    const float far_plane  = 5.0f;    
    const float camera_tx=  0.0f;
    const float camera_ty=  0.0f;
    const float camera_tz=  -0.15f;
    const float torus_z= 0; //-2.40f;

    vmath::mat4 projection_matrix= 
        vmath::frustum(  
            left, right, bottom, top, 
            near_plane,far_plane) * 
            vmath::translate(camera_tx,camera_ty,camera_tz);  
    
    //vmath::mat4 view_matrix= vmath::mat4(vmath::mat4::identity());
    //vmath::mat4 view_matrix= vmath::translate(vmath::vec3(0.0f, 0.0f, torus_z)) * side_view_matrix * vmath::translate(vmath::vec3(0.0f, 0.0f, -torus_z));
    vmath::mat4 &view_matrix= side_view_matrix;

    DisplayCubeMap( auto_redraw,tick,   view_matrix,        projection_matrix);
    DisplayTorus(   auto_redraw,tick,   view_matrix,        projection_matrix,  torus_z);
    DisplayRef(     auto_redraw,tick,   view_matrix,        projection_matrix);

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

    glUniformMatrix4fv(skybox_rotate_loc, 1, GL_FALSE, model_view_projection_matrix);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_element_buffer);

    glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_SHORT, NULL);
    glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_SHORT, BUFFER_OFFSET(8 * sizeof(GLushort)));
}
void CubeMapExample::DisplayTorus(
        bool auto_redraw, float tick, vmath::mat4& view_matrix, vmath::mat4& projection_matrix, GLfloat z_trans)    
{
    static const vmath::vec3 X(1.0f, 0.0f, 0.0f);
    static const vmath::vec3 Y(0.0f, 1.0f, 0.0f);
    static const vmath::vec3 Z(0.0f, 0.0f, 1.0f);

    const VBM_BOUNDING_BOX & bb= torus.BoundingBox();
    const float l((bb.max[1]-bb.min[1])*2.0f);
    const float scale=(l!=0 ? 1.0f/l : 1.0f);
    const float tx(0.0);
    const float ty(0.0); //std::abs(scale*bb.min[1]));
    const float tz(0.0);

    glUseProgram(torus_prog);

    vmath::mat4 model_matrix = vmath::translate(tx, ty, tz) *
    // vmath::translate(vmath::vec3(0.0f, 0.0f, z_trans)) * 
    //            vmath::rotate(80.0f * 3.0f * tick, Y) *
                vmath::rotate(70.0f * 3.0f * tick, X) *
                vmath::rotate(70.0f * 3.0f * tick, Z) *
                vmath::scale(scale);
    vmath::mat4 model_view_matrix= view_matrix * model_matrix;
    glUniformMatrix4fv(torus_model_view_mat_loc, 1, GL_FALSE, model_view_matrix);

    vmath::mat4 model_view_projection_matrix = projection_matrix * model_view_matrix;
    glUniformMatrix4fv(torus_model_view_projection_mat_loc, 1, GL_FALSE, model_view_projection_matrix);

    glClear(GL_DEPTH_BUFFER_BIT);

    torus.Render();
    

}
void CubeMapExample::DisplayRef(bool auto_redraw, float t, vmath::mat4 &view_matrix,  vmath::mat4 & projection_matrix)
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
    if(display_ref_planes)
    {
        xy_plane.render();
        yz_plane.render();
        zx_plane.render();       
    }
    glUseProgram(0);
}
void CubeMapExample::Finalize(void)
{
    glUseProgram(0);
    glDeleteProgram(skybox_prog);
    glDeleteProgram(torus_prog);
    glDeleteProgram(ref_prog);
    glDeleteTextures(1, &tex);
    glDeleteVertexArrays(1, &tex);
}

void CubeMapExample::Resize(int width, int height)
{
    glViewport(0, 0 , width, height);
    
    camera_aspect_ratio = float(height) / float(width);
}
