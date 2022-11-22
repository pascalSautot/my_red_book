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
enum Buffer_IDs { ArrayBuffer1, ArrayBuffer2, NumBuffers };
enum Attrib_IDs { vPosition = 0, vColor=1 };

GLuint  VAOs[NumVAOs];
GLuint  Buffers[NumBuffers];

const GLuint  NumVertices = 6;
const GLuint curDim= NumDim;
GLfloat  static_vertices[NumVertices][NumDim] = {
    { -0.90f, -0.90f, 0.5f, 1}, {  0.85f, -0.90f, 0.3f, 1 }, { -0.90f,  0.85f, 0.0f, 1 },  // Triangle 1
    {  0.90f, -0.85f, -0.1f, 1 }, {  0.90f,  0.90f, -0.5f, 1 }, { -0.85f,  0.90f, -0.8f, 1 }   // Triangle 2
};

GLfloat  static_vertices1[NumVertices/2][NumDim] = {
    { -0.90f, -0.90f, 0.5f, 1}, {  0.85f, -0.90f, 0.3f, 1 }, { -0.90f,  0.85f, 0.0f, 1 }  // Triangle 1
};
GLfloat  static_vertices2[NumVertices/2][NumDim] = {
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
    const unsigned int n=NumVertices;
    GLfloat infColor_values[n][curDim];    
    for (int i=0; i<n; i++) 
    {
        for (int j=0;j< curDim ; j++)
        {
            const float c=2.0f/(2.0f*i+1.0f);
            infColor_values[i][j]=c*infColor_value[j];
        }
        infColor_values[i][3]=1;
    }    

    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/triangles/triangles2.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/triangles/triangles2.frag" },
        { GL_NONE, NULL }
    };

    GLuint program = LoadShaders( shaders );
    if( program == 0) { std::cerr<<"shader program failed to build" << std::endl; }
    glUseProgram( program );

    glGenVertexArrays( NumVAOs, VAOs );
    glBindVertexArray( VAOs[Triangles] );
    glCreateBuffers( NumBuffers, Buffers );

    // vertex coords
    glBindBuffer( GL_ARRAY_BUFFER, Buffers[ArrayBuffer1] );
    glNamedBufferData( Buffers[ArrayBuffer1], sizeof(static_vertices) , NULL, GL_STATIC_DRAW);
    glNamedBufferSubData( Buffers[ArrayBuffer1], 0,  sizeof(static_vertices), static_vertices);
    GLuint64 offset = 0;
    glVertexAttribPointer( vPosition, NumDim, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    glEnableVertexAttribArray( vPosition );

    // vertex colors 
    glBindBuffer( GL_ARRAY_BUFFER, Buffers[ArrayBuffer2] );       
    glNamedBufferData( Buffers[ArrayBuffer2],  sizeof(infColor_values), NULL, GL_STATIC_DRAW);
    glNamedBufferSubData( Buffers[ArrayBuffer2], 0,  sizeof(infColor_values), infColor_values);
    
    offset = 0;
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)offset );
    glEnableVertexAttribArray( vColor );
    
    //second triangle    
    /*
    glBindBuffer( GL_ARRAY_BUFFER, Buffers[ArrayBuffer2] );
    glNamedBufferData(  Buffers[ArrayBuffer2], sizeof(static_vertices2) + sizeof(infColor_values2), NULL, GL_STATIC_DRAW);
    glNamedBufferSubData(  Buffers[ArrayBuffer2], 0,                        sizeof(static_vertices2), static_vertices2);
    glNamedBufferSubData(  Buffers[ArrayBuffer2], sizeof(static_vertices2), sizeof(infColor_values2), infColor_values2);   
   
    offset = sizeof(static_vertices1);
    glVertexAttribPointer( vPosition, NumDim, GL_FLOAT, GL_FALSE, sizeof(static_vertices1), (const GLvoid *)offset );
    glEnableVertexAttribArray( vPosition );
    
    offset = sizeof(infColor_values1);

    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, sizeof(infColor_values1), (const GLvoid *)offset );
    glEnableVertexAttribArray( vColor );    
 */
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray(0);
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
    glDrawArrays( GL_TRIANGLES,  0, NumVertices );
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
