//////////////////////////////////////////////////////////////////////////////
//
//  Triangles.cpp
//
//////////////////////////////////////////////////////////////////////////////

#include "vgl.h"
#include "LoadShaders.h"
#include <iostream>

enum VertexDim { xDim, yDim, zDim, wDim, VertexDim};
enum VAO_IDs { Triangle1, Triangle2, NumVAOs };
enum Buffer_IDs { Mesh, NumBuffers };
enum Attrib_IDs { vPosition = 0, vColor=1 };

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
        { GL_VERTEX_SHADER, "media/shaders/triangles/triangles2.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/triangles/triangles2.frag" },
        { GL_NONE, NULL }
    };

    GLuint program = LoadShaders( shaders );
    if( program == 0) { std::cerr<<"shader program failed to build" << std::endl; }
    glUseProgram( program );

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
    static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    glClearBufferfv(GL_COLOR, 0, black);

    glBindVertexArray( VAOs[Triangle1] );
    glDrawArrays( GL_TRIANGLES,  0, NumVertices/2 );
    glBindVertexArray( VAOs[Triangle2] );
    glDrawArrays( GL_TRIANGLES,  0, NumVertices/2 );   
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
