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

#include <stdio.h>
#include <iostream>
#include "LoadShaders.h"
using namespace vmath;
using namespace std;
#undef DOIT

BEGIN_APP_DECLARATION(InstancingExample)
    // Override functions from base class
    virtual void Initialize(const char * title);
    virtual void Display(bool auto_redraw);
    virtual void Finalize(void);
    virtual void Resize(int width, int height);

    // Member variables
    float aspect;
    GLuint update_prog;
    GLuint vao[2];
    GLuint vbo[2];
    GLuint xfb;

    GLuint color_vbo;
    GLuint render_prog;


    GLuint geometry_tex;

    GLuint geometry_xfb;
    GLuint particle_xfb;

    GLint view_matrix_loc;
    GLint model_matrix_loc;
    GLint projection_matrix_loc;
    GLint triangle_count_loc;
    GLint time_step_loc;

    VBObject object;
    SimpleObject ref_planes;

    enum { vPositionLoc=0, vNormalIndexLoc=1, vTextureLoc=2, vInstanceColorLoc=3};    
    
END_APP_DECLARATION()

DEFINE_APP(InstancingExample, "Instancing Example")

#define INSTANCE_COUNT 1

#include "mesh.h"
mesh::XAxis x_axis;
mesh::YAxis y_axis;
mesh::ZAxis z_axis;
mesh::XYPlane xy_plane;
mesh::YZPlane yz_plane;
mesh::ZXPlane zx_plane;

#undef DO_ARMADILLO

void InstancingExample::Initialize(const char * title)
{
    int n;

    base::Initialize(title);

    // Create the program for rendering the model
    // load shaders in program
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/instancing/instancing4.vert" },

#ifdef DO_ARMADILLO        
        { GL_FRAGMENT_SHADER, "media/shaders/instancing/instancing4.frag" },
#else        
        { GL_FRAGMENT_SHADER, "media/shaders/instancing/instancing-wo-normals.frag" },
#endif     
        { GL_NONE, NULL }
    };

    render_prog = LoadShaders( shaders );
    if( render_prog == 0) { std::cerr<<"shader program failed to build" << std::endl; }
    glUseProgram( render_prog );

   // "model_matrix" is actually an array of 4 matrices
    model_matrix_loc = glGetUniformLocation(render_prog, "model_matrix");
    view_matrix_loc= glGetUniformLocation(render_prog, "view_matrix");
    projection_matrix_loc = glGetUniformLocation(render_prog, "projection_matrix");

    // Load the object
    object.LoadFromVBM("media/armadillo_low.vbm", vPositionLoc,vNormalIndexLoc, vTextureLoc);

    object.BindVertexArray();

#ifdef DO_ARMADILLO        

    // Generate the colors of the objects
    vmath::vec4 colors[INSTANCE_COUNT];

    for (n = 0; n < INSTANCE_COUNT; n++)
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
#endif

    x_axis.setData();
    y_axis.setData();
    z_axis.setData();
    xy_plane.setData();
    yz_plane.setData();
    zx_plane.setData();
}


void InstancingExample::Display(bool auto_redraw)
{
    float t = float(app_time() & 0x3FFFF) / float(0x3FFFF);

#ifdef DO_ARMADILLO        

    // Clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
#else
    // clear framebuffer
    static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glClearBufferfv(GL_COLOR, 0, black);
#endif
    // Activate instancing program
    glUseProgram(render_prog);

    float tt= (3000.0f*t);
    const int itt= ((int)tt)%360;
    float rtt= (float)itt;
#ifdef DO_ARMADILLO
    // Set four model matrices
    vmath::mat4 model_matrix, view_matrix=vmath::mat4::identity();
//3000.0f*t

//                    vmath::rotate( tt+0.0f , 0.0f, 0.0f, 1.0f) *

    model_matrix =  vmath::rotate( tt+0.0f , 0.0f, 0.0f, 1.0f) * /* vmath::rotate(0.0f, 0.0f, 1.0f, 0.0f) **/
                    vmath::scale(0.025f)*
                    vmath::mat4::identity(); 

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
    view_matrix = vmath::mat4::identity(); /*vmath::lookat(eye,at,up);*/


    // Set up the projection matrix
    vmath::mat4 projection_matrix(vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 10.0f) * vmath::translate(0.0f, 0.0f, -5.0f)); 



    // Render INSTANCE_COUNT objects
    object.Render(0, INSTANCE_COUNT);
#endif

    vmath::mat4 view_matrix = vmath::rotate(tt, 0.0f, 1.0f, 0.0f);

    vmath::mat4 projection_matrix=(vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 5.0f) * vmath::translate(0.0f, 0.0f, -3.0f));
    
    //projection_matrix=(vmath::frustum(left_side,right_side, bottom, top, near_plane, far_plane) * vmath::translate(0.0f, 0.0f, -1000.0f));
    vmath::mat4 model_matrix=vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f)*vmath::rotate(-20.0f, 0.0f, 1.0f, 1.0f);


    // pass on matrices 
    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, projection_matrix);

    //render axes
    x_axis.render();
    y_axis.render();
    z_axis.render();
    // render planes
    xy_plane.render();
    yz_plane.render();
    zx_plane.render();     

    base::Display();
}

void InstancingExample::Finalize(void)
{
    glUseProgram(0);
    glDeleteProgram(render_prog);
    glDeleteVertexArrays(2, vao);
    glDeleteBuffers(2, vbo);
}

void InstancingExample::Resize(int width, int height)
{
    glViewport(0, 0 , width, height);

    aspect = float(height) / float(width);
}
