//////////////////////////////////////////////////////////////////////////////
//
//  Triangles.cpp
//
//////////////////////////////////////////////////////////////////////////////

#include "vgl.h"
#include "LoadShaders.h"
#include <iostream>

enum VertexDim { xDim, yDim, zDim, wDim, NumDim};
enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer, NumBuffers };
enum Attrib_IDs { vPosition = 0, vColor=1 };

GLuint  VAOs[NumVAOs];
GLuint  Buffers[NumBuffers];

const GLuint  NumVertices = 6;
const GLuint curDim= NumDim-1;

GLfloat  vertices[NumVertices][NumDim] = {
    { -0.90f, -0.90f, 0.5f, 1}, {  0.85f, -0.90f, 0.3f, 1 }, { -0.90f,  0.85f, 0.0f, 1 },  // Triangle 1
    {  0.90f, -0.85f, -0.1f, 1 }, {  0.90f,  0.90f, -0.5f, 1 }, { -0.85f,  0.90f, -0.8f, 1 }   // Triangle 2
};
GLfloat infColor_value[4]= {0.9f, 0.7f, 0.5f, 1.0f};  

//----------------------------------------------------------------------------
//
// init
//

void
init( void )
{
    glGenVertexArrays( NumVAOs, VAOs );
    glBindVertexArray( VAOs[Triangles] );

    GLfloat infColor_values[NumVertices][curDim];    
    for (int i=0; i<NumVertices; i++) 
    {
        for (int j=0;j< curDim ; j++)
        {
            infColor_values[i][j]=infColor_value[j]/(i+1);
        }
        infColor_value[3]=1;
    }    

    glCreateBuffers( NumBuffers, Buffers );
    glBindBuffer( GL_ARRAY_BUFFER, Buffers[ArrayBuffer] );
/* 1/ with original shaders 
    glBufferStorage( GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0); 
*/

/* 2/ with original shaders 
    glNamedBufferStorage( Buffers[ArrayBuffer],                 sizeof(vertices),       vertices, 0);
*/

/* 3/ with original shaders 
    glNamedBufferData( Buffers[ArrayBuffer], sizeof(vertices) , NULL, GL_STATIC_DRAW);
    glNamedBufferSubData( Buffers[ArrayBuffer], 0,                  sizeof(vertices),       vertices);
*/
    glNamedBufferData( Buffers[ArrayBuffer], sizeof(vertices) +sizeof(infColor_values), NULL, GL_STATIC_DRAW);
    glNamedBufferSubData( Buffers[ArrayBuffer], 0,                  sizeof(vertices),       vertices);
    glNamedBufferSubData( Buffers[ArrayBuffer], sizeof(vertices),   sizeof(infColor_values), infColor_values);
   
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/triangles/triangles2.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/triangles/triangles2.frag" },
        { GL_NONE, NULL }
    };

    GLuint program = LoadShaders( shaders );
    if( program == 0) { std::cerr<<"shader program failed to build" << std::endl; }
    glUseProgram( program );

    GLuint64 offset = 0;
    glVertexAttribPointer( vPosition, NumDim, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    glEnableVertexAttribArray( vPosition );
    
    offset += sizeof(vertices);

    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)offset );
    glEnableVertexAttribArray( vColor );

}
void release()
{
    std::cout <<"attributes" << std::endl;
    glDisableVertexAttribArray( vPosition );
    std::cout <<"buffers" << std::endl;
    glDeleteBuffers(NumBuffers, Buffers );
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

    glBindVertexArray( VAOs[Triangles] );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
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
