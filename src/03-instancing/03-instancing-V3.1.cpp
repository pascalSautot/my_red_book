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

    GLuint weight_vbo;
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

enum VertexDim { xDim, yDim, zDim, wDim, VertexDim};
enum VAO_IDs { Planes, NumVAOs };
enum Buffer_IDs { Mesh, NumBuffers };
enum Attrib_IDs { vPositionLoc=0, vNormalIndexLoc=1, vTextureLoc=2, vInstanceColorLoc=3};

void build_refplane(GLfloat **ref_vertices, GLsizei offset, const GLboolean bx,const GLboolean by,const GLboolean bz)
{
    GLint id[3];
    if(bx &&  by){ id[0]=0,id[1]=1; id[2]=2;}  // [0,1],2 XY
    if(by &&  bz){ id[0]=1,id[1]=2; id[2]=0;}  // [1,2],0 XZ
    if(bz &&  bx){ id[0]=2,id[1]=0; id[2]=1;}  // [2,0],1 ZX

    //first vertex
    ref_vertices[offset][id[0]]  =0.0f;
    ref_vertices[offset][id[1]]  =0.0f;

    //second vertex
    ref_vertices[offset+1][id[0]]  =1.0f;
    ref_vertices[offset+1][id[1]]  =0.0f;

    //third vertex
    ref_vertices[offset+2][id[0]]  =1.0f;
    ref_vertices[offset+2][id[1]]  =1.0f;

    //fourth vertex
    ref_vertices[offset+3][id[0]]  =0.0f;
    ref_vertices[offset+3][id[1]]  =1.0f;
    
    // homogeneous variable + id[2] coord
    for(unsigned int i=0;i<4;i++)
    {
        ref_vertices[offset+i][3]      =1.0f;      
        ref_vertices[offset+i][id[2]]  =0.0f;
    }
#ifdef _DEBUG
    const char *plane_name= ( 
            (bx && by) ? "X-Y plane" : 
                ( 
                    (by && bz) ? "Y-Z plane" : 
                        ( (bz && bx) ?  "Z-X plane" : "unknown ") 
                )
    );
    std::cout << plane_name << ": [XYZ]=[";
    for(unsigned int i=0;i<3;i++)
    {
        std::cout << id[i];
        if(i<2) std::cout << ",";  
    }
    std::cout<<"]"<<std::endl;

    for(unsigned int j=0;j<4;j++)
    {
        std::cout << "[x,y,z,w]=[ ";
        for(unsigned int i=0;i<4;i++)
        {
            std::cout << ref_vertices[j+offset][i];
            if(i<3) std::cout << ",";
        }
        std::cout << "]" << std::endl;
    }
#endif     
}
void build_ref_colors(GLfloat **ref_colors)
{
    GLfloat red[4]={ 1.0f, 0.0f, 0.0f, 1.0f};
    GLfloat green[4]={ 0.0f, 1.0f, 0.0f, 1.0f};
    GLfloat blue[4]={ 0.0f, 0.0f, 1.0f, 1.0f};
    for(int j=0;j<4;j++)
    {
        for(int i=0;i<4;i++)
        {
            ref_colors[j][i]   = red[i];
            ref_colors[j+4][i] = green[i];
            ref_colors[j+8][i] = blue[i];
        }
    } 
#ifdef _DEBUG
    std::cout << "colors" << std::endl;


    for(unsigned int j=0;j<12;j++)
    {
        std::cout << "[RGBA]=[ ";
        for(unsigned int i=0;i<4;i++)
        {
            std::cout << ref_colors[j][i];
            if(i<3) std::cout << ",";
        }
        std::cout << "]" << std::endl;
    }
#endif  
}

void InstancingExample::Initialize(const char * title)
{
    int n;

    base::Initialize(title);

    // Create the program for rendering the model
    // load shaders in program
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/instancing/planes.vert" },
#ifdef DOIT
        { GL_FRAGMENT_SHADER, "media/shaders/instancing/planes-normals.frag" },
#else        
        { GL_FRAGMENT_SHADER, "media/shaders/instancing/planes.frag" },
#endif        
        { GL_NONE, NULL }
    };

    render_prog = LoadShaders( shaders );
    if( render_prog == 0) { std::cerr<<"shader program failed to build" << std::endl; }
    glUseProgram( render_prog );

   // "model_matrix" is actually an array of 4 matrices
    model_matrix_loc = glGetUniformLocation(render_prog, "model_matrix");
    //view_matrix_loc= glGetUniformLocation(render_prog, "view_matrix");
    projection_matrix_loc = glGetUniformLocation(render_prog, "projection_matrix");

#ifdef DOIT
    // Load the object
    object.LoadFromVBM("media/armadillo_low.vbm", vPositionLoc,vNormalIndexLoc, vTextureLoc);
#endif    
    const GLsizei vertex_count(12);
    GLfloat **ref_vertices= new ( GLfloat * [vertex_count]);
    GLfloat **ref_colors= new ( GLfloat * [vertex_count]);
    for(int i=0;i<vertex_count;i++){
        ref_vertices[i]=new GLfloat[4];
        ref_colors[i]=new GLfloat[4];
    }

    build_ref_colors(ref_colors);
  
    build_refplane(ref_vertices,0,true, true,   false); //XY 
    build_refplane(ref_vertices,4,false,true,   true);  //YZ
    build_refplane(ref_vertices,8,true, false,  true);  //XZ

    ref_planes.Build(ref_vertices,ref_colors,NULL,vertex_count);
    // Bind its vertex array object so that we can append the instanced attributes


#ifdef DOIT
    object.BindVertexArray();


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
#endif
    ref_planes.BindVertexArray();

    // Done (unbind the object's VAO)
    glBindVertexArray(0);

}


void InstancingExample::Display(bool auto_redraw)
{
    float t = float(app_time() & 0x3FFFF) / float(0x3FFFF);

    // Clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Activate instancing program
    glUseProgram(render_prog);

    // Set four model matrices
    vmath::mat4 model_matrix, view_matrix=vmath::mat4::identity();
//3000.0f*t
    float tt= (3000.0f*t);
    const int itt= ((int)tt)%360;
    float rtt= (float)itt;
//                    vmath::rotate( tt+0.0f , 0.0f, 0.0f, 1.0f) *

    model_matrix =  vmath::mat4::identity();
    /*vmath::rotate(180.0f, 0.0f, 1.0f, 0.0f) *
                    vmath::scale(0.5f)*
                    vmath::mat4::identity(); */

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

    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, view_matrix);
    //glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, view_matrix);

    // Set up the projection matrix
    /* vmath::mat4 projection_matrix(vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 250.0f) * vmath::translate(0.0f, 0.0f, -100.0f)); */
    vmath::mat4 projection_matrix=(vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 5.0f) * vmath::translate(0.0f, 0.0f, -3.0f));

    glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, projection_matrix);

    // Render INSTANCE_COUNT objects
#ifdef DOIT    
    object.Render(0, INSTANCE_COUNT);
#endif    
    ref_planes.Render();

    base::Display();
}

void InstancingExample::Finalize(void)
{
    glUseProgram(0);
    glDeleteProgram(update_prog);
    glDeleteVertexArrays(2, vao);
    glDeleteBuffers(2, vbo);
}

void InstancingExample::Resize(int width, int height)
{
    glViewport(0, 0 , width, height);

    aspect = float(height) / float(width);
}
