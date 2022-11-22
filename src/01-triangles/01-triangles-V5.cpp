//////////////////////////////////////////////////////////////////////////////
//
//  Triangles.cpp : draw and draw elements + rigid transform and scaling transform
//
//////////////////////////////////////////////////////////////////////////////

#include "vgl.h"
#include "vmath.h"
#include "LoadShaders.h"
#include <iostream>
using namespace vmath;

enum VertexDim { xDim, yDim, zDim, wDim, VertexDim};
enum EBO_IDs { Triangle1_Ind, Triangle2_Ind, NumEBOs };
enum VAO_IDs { Triangle1, Triangle2, NumVAOs };
enum Buffer_IDs { Mesh, NumBuffers };
enum Attrib_IDs { vPosition = 0, vColor=1 };

GLuint  EBOs[NumEBOs];
GLuint  VAOs[NumVAOs];
GLuint  Buffers1[NumBuffers],Buffers2[NumBuffers];

const GLuint  NumVertices = 6;
const GLuint curDim= VertexDim;
GLfloat  static_vertices1[NumVertices/2][VertexDim] = {
    { -0.90f, -0.90f, 0.5f, 1}, {  0.85f, -0.90f, 0.5f, 1 }, { -0.90f,  0.85f, 0.5f, 1 }  // Triangle 1
};
GLfloat  static_vertices2[NumVertices/2][VertexDim] = {
    {  0.90f, -0.85f, -0.5f, 1 }, {  0.90f,  0.90f, -0.5f, 1 }, { -0.85f,  0.90f, -0.5f, 1 }   // Triangle 2
};
enum ColorDim { rDim, gDim, bDim, aDim, ColorDim};

GLfloat infColor_value[4]= {0.5f, 0.3f, 0.2f, 1.0f};  
GLint model_matrix_loc;
GLint scale_matrix_loc;
// Indices for the triangle strips
static const GLushort vertex_indices[] =
{
    0, 1, 2
};
GLuint program;
//----------------------------------------------------------------------------
//
// init
//
typedef struct _Object {
    GLfloat vertex[VertexDim];
    GLfloat color[ColorDim];
} Object;

void
init( void )
{   
    // init colors
    const unsigned int n=NumVertices/2;
    Object triangle1[n], triangle2[n];
    for (int i=0; i<n; i++) 
    {
        for (int j=0;j< VertexDim ; j++)
        {
            triangle1[i].vertex[j]= static_vertices1[i][j];
            triangle2[i].vertex[j]= static_vertices2[i][j];
        }
        for (int j=0;j< ColorDim ; j++)
        {
            triangle1[i].color[j]= ((i+1)/(float)n)*infColor_value[j];
            triangle2[i].color[j]= ((i+1)/(float)2.0f*n)*infColor_value[j];
        }        
        triangle1[i].color[aDim]=1;
        triangle2[i].color[aDim]=1;
    }    
    // load shaders in program
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/triangles/triangles3.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/triangles/triangles2.frag" },
        { GL_NONE, NULL }
    };

    program = LoadShaders( shaders );
    if( program == 0) { std::cerr<<"shader program failed to build" << std::endl; }
    glUseProgram( program );

    model_matrix_loc = glGetUniformLocation(program, "model_matrix");
    scale_matrix_loc = glGetUniformLocation(program, "scale_matrix");
    if(model_matrix_loc == -1 ){
        std::cout << "model matrix not fetched in shader" << std::endl;
    }
    if(scale_matrix_loc == -1){
        std::cout << "scale matrix not fetched in shader" << std::endl;
    }
    glGenBuffers(1, EBOs);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_indices), vertex_indices, GL_STATIC_DRAW);

    // define buffers and associate dat ato buffers
    glGenVertexArrays( NumVAOs, VAOs );
    glBindVertexArray( VAOs[Triangle1] );
    glCreateBuffers( NumBuffers, Buffers1 );

    // triangle 1
    GLint size =  sizeof(triangle1);
    GLsizei offset = 0;
    glBindBuffer( GL_ARRAY_BUFFER, Buffers1[Mesh] );
    glNamedBufferData( Buffers1[Mesh], size , NULL, GL_STATIC_DRAW);
    glNamedBufferSubData( Buffers1[Mesh], offset,  size, triangle1);

    size = VertexDim;
    offset = 0;
    GLboolean normalized=GL_FALSE;
    GLsizei stride = sizeof(triangle1[0]);
    glVertexAttribPointer( vPosition, size, GL_FLOAT, normalized, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vPosition );

    size = ColorDim;
    offset = sizeof(triangle1[0].vertex);
    stride = sizeof(triangle1[0]);    
    glVertexAttribPointer( vColor, size, GL_FLOAT, normalized, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vColor );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray(0);

    // triangle 2
    glBindVertexArray( VAOs[Triangle2] );
    glCreateBuffers( NumBuffers, Buffers2 );
    size =  sizeof(triangle2);
    offset = 0;
    glBindBuffer( GL_ARRAY_BUFFER, Buffers2[Mesh] );
    glNamedBufferData( Buffers2[Mesh], size , NULL, GL_STATIC_DRAW);
    glNamedBufferSubData( Buffers2[Mesh], offset,  size, triangle2);
    // vertex 
    offset = 0;
    size = VertexDim;
    stride = sizeof(triangle2[0]);  
    glVertexAttribPointer( vPosition, size, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vPosition );

    size = ColorDim;
    offset = sizeof(triangle2[0].vertex);
    stride = sizeof(triangle2[0]);  
    glVertexAttribPointer( vColor, size, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vColor );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray(0);
    
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray(0);
}
void release()
{
    std::cout <<"attributes" << std::endl;
    glDisableVertexAttribArray( vPosition );
    std::cout <<"buffers" << std::endl;
    glDeleteBuffers(NumBuffers, Buffers1 );
    glDeleteBuffers(NumBuffers, Buffers2 );
    bool areVertexArrays(NumVAOs>=1);
    for (int i=0; i<NumVAOs;i++)
    {
        areVertexArrays = areVertexArrays && glIsVertexArray(VAOs[i]);
    }
    if( areVertexArrays) 
    {
        std::cout <<"vertex arrays" << std::endl;
        glDeleteVertexArrays(NumVAOs, VAOs);
    }
}
//----------------------------------------------------------------------------
//
// display
//

void
display( void )
{
    vmath::mat4 model_matrix=vmath::mat4::identity();
    vmath::mat4 scale_matrix=vmath::mat4::identity();
    glUseProgram(program);
    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_matrix);     
    glUniformMatrix4fv(scale_matrix_loc, 1, GL_FALSE, scale_matrix);   
     
    // Draw Arrays...
    
    static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    glClearBufferfv(GL_COLOR, 0, black);

    const unsigned int n=NumVertices/2;
    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_matrix);     

    glBindVertexArray( VAOs[Triangle1] ); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    glDrawArrays( GL_TRIANGLES,  0, n );

    glBindVertexArray( VAOs[Triangle2] );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    glDrawArrays( GL_TRIANGLES,  0, n );   


    float x= 0.0f,y=0.0f,z=0.0f;
    const float tx=0.1f, ty=0.1f, tz= 0.1f;

    glBindVertexArray( VAOs[Triangle1] ); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);

    model_matrix = vmath::translate(x-tx, y-ty, z-tz);
    scale_matrix[0][0]= 0.8f;
    scale_matrix[1][1]= 0.8f;
    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_matrix);     
    glUniformMatrix4fv(scale_matrix_loc, 1, GL_FALSE, scale_matrix);     
    glDrawElements(GL_TRIANGLES, n, GL_UNSIGNED_SHORT, NULL);

    glBindVertexArray( VAOs[Triangle2] ); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);

    model_matrix = vmath::translate(x-5*tx, y-10*ty, z-2*tz);
    scale_matrix[0][0]= 0.5f;
    scale_matrix[1][1]= 0.5f;    
    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_matrix);   
    glUniformMatrix4fv(scale_matrix_loc, 1, GL_FALSE, scale_matrix);    
   
    glDrawElements(GL_TRIANGLES, n, GL_UNSIGNED_SHORT, NULL);

    glBindVertexArray(0);    

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

    GLFWwindow* window = glfwCreateWindow(800, 600, "Triangles", NULL, NULL);

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
