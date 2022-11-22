//////////////////////////////////////////////////////////////////////////////
//
//  Triangles.cpp
//
//////////////////////////////////////////////////////////////////////////////

#include "vgl.h"
#include "vmath.h"
#include "LoadShaders.h"

#include <iostream>
#include <string>
using namespace vmath;


 
//GLint view_matrix_loc;
GLint model_matrix_loc;
GLint scale_matrix_loc;
GLint projection_matrix_loc;
GLuint program;
GLfloat aspect=1.0f;
//----------------------------------------------------------------------------
//
// init
//

// define 3 planes X-Y, Y-Z, X-Z
const GLint NumPlanes = 3;
#ifdef STRIP
const GLint PolygonsPerPlane = 1;
const GLint VerticesPerPolygon = 4;
#else
const GLint PolygonsPerPlane = 2;
const GLint VerticesPerPolygon = 3;
#endif

const GLint NumPlaneVertices = NumPlanes*PolygonsPerPlane*VerticesPerPolygon;
#include "mesh.h"
mesh::XAxis x_axis;
mesh::YAxis y_axis;
mesh::ZAxis z_axis;
mesh::XYPlane xy_plane;
mesh::YZPlane yz_plane;
mesh::ZXPlane zx_plane;
void
init( void )
{   
    // load shaders in program
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/triangles/triangles4.vert"  },
        { GL_FRAGMENT_SHADER, "media/shaders/triangles/triangles2.frag"},
        { GL_NONE, NULL }
    };
    const unsigned int nshaders = sizeof(shaders)/sizeof(shaders[0])-1;

    program = LoadShaders( shaders );
    for ( int i=0; i<nshaders; i++)
    {
        if( !shaders[i].compile_status)
        {
            std::cerr<<"shader program [" << shaders[i].shader << "]:" << shaders[i].filename << ", failed to build" << std::endl;
        }
    }
    if( program == 0) { std::cerr<<"shader program failed to build" << std::endl; }
    glUseProgram( program );
   
    //view_matrix_loc = glGetUniformLocation(program, "view_matrix");
    model_matrix_loc = glGetUniformLocation(program, "model_matrix");
    scale_matrix_loc = glGetUniformLocation(program, "scale_matrix");
    projection_matrix_loc = glGetUniformLocation(program, "projection_matrix");
    /*
    if(view_matrix_loc == -1 ){
        std::cout << "view matrix not fetched in shader" << std::endl;
    }
    */
    if(model_matrix_loc == -1 ){
        std::cout << "model matrix not fetched in shader" << std::endl;
    }
    if(scale_matrix_loc == -1){
        std::cout << "scale matrix not fetched in shader" << std::endl;
    }
    if(projection_matrix_loc == -1){
        std::cout << "projection matrix not fetched in shader" << std::endl;
    }    
    x_axis.setData();
    y_axis.setData();
    z_axis.setData();
    xy_plane.setData();
    yz_plane.setData();
    zx_plane.setData();
}
void release()
{
}
//----------------------------------------------------------------------------
//
// display
//

float 
translate_x(const float x,const float tx, const int i)
{
    return x+(5+i)*tx;
}
float 
translate_y(const float y,const float ty, const int i)
{
    return y+(10+i)*ty;
}
float 
translate_z(const float z,const float tz, const int i)
{
    return z+(i*5.0f)*tz;
}

void
display( void )
{
    vmath::mat4 view_matrix=vmath::mat4::identity();
    vmath::mat4 model_matrix=vmath::mat4::identity();
    vmath::mat4 projection_matrix=vmath::mat4::identity();
    vmath::mat4 scale_matrix=vmath::mat4::identity();

    view_matrix = vmath::rotate(10.0f, 1.0f, 0.0f, 0.0f)*
                    vmath::rotate(-30.0f, 0.0f, 1.0f, 0.0f);

    projection_matrix=(vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 5.0f) * vmath::translate(0.0f, 0.0f, -3.0f));
    
    //projection_matrix=(vmath::frustum(left_side,right_side, bottom, top, near_plane, far_plane) * vmath::translate(0.0f, 0.0f, -1000.0f));
    model_matrix=vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f)*vmath::rotate(-20.0f, 0.0f, 1.0f, 1.0f);
 
    glUseProgram(program);

    //glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, projection_matrix);  
    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_matrix);     
    glUniformMatrix4fv(scale_matrix_loc, 1, GL_FALSE, scale_matrix);   
     
    // clear framebuffer
    static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glClearBufferfv(GL_COLOR, 0, black);

    //render axex
    x_axis.render();
    y_axis.render();
    z_axis.render();
    // render planes
    xy_plane.render();
    yz_plane.render();
    zx_plane.render();      

}

//----------------------------------------------------------------------------
//
// main
//

#ifdef _WIN32
int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nCmdShow
)
#else
int
main( int argc, char** argv )
#endif
{
    glfwInit();

    const int width=800, height=600;

    GLFWwindow* window = glfwCreateWindow(width, height, "Triangles", NULL, NULL);
    aspect = float(height) / float(width);
    glfwMakeContextCurrent(window);
    gl3wInit();

    init();

    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    release();

    glfwDestroyWindow(window);

    glfwTerminate();
}
