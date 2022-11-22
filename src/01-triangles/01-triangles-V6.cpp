//////////////////////////////////////////////////////////////////////////////
//
//  Triangles.cpp
//
//////////////////////////////////////////////////////////////////////////////

#include "vgl.h"
#include "vmath.h"
#include "LoadShaders.h"
#include <iostream>
using namespace vmath;

enum VertexDim { xDim, yDim, zDim, wDim, VertexDim};
enum EBO_IDs { Triangle_Ind, NumEBOs };
enum VAO_IDs { Triangle1, Triangle2, Planes, NumVAOs };
enum Buffer_IDs { Mesh, NumBuffers };
enum Attrib_IDs { vPosition = 0, vColor=1 };

GLuint  EBOs[NumEBOs];
GLuint  VAOs[NumVAOs];
GLuint  Buffers1[NumBuffers],Buffers2[NumBuffers],PlaneBuffers[NumBuffers];

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
GLint projection_matrix_loc;
// Indices for the triangle strips
static const GLushort vertex_indices[] =
{
    0, 1, 2
};
GLuint program;
GLfloat aspect=1.0f;
//----------------------------------------------------------------------------
//
// init
//
typedef struct _Object {
    GLfloat vertex[VertexDim];
    GLfloat color[ColorDim];
} Object;
// define 3 planes X-Y, Y-Z, X-Z
cont GLint NumPlanes = 3;
cont GLint NumPlaneVertices = NumPlanes*2;
Object planes[NumPlaneVertices] = {
    //X-Y
    {
        { 0.0f, 0.0f, 0.0f, 1}, { 1.0f, 0.0f, 0.0f, 1 }, { 1.0f, 1.0f, 0.0f, 1 } ,  //vertex
        { 1.0f, 0.0f, 0.0f, 1}, { 1.0f, 0.0f, 0.0f, 1} ,  { 1.0f, 0.0f, 0.0f, 1 },  //color

        { 0.0f, 0.0f, 0.0f, 1}, { 1.0f, 1.0f, 0.0f, 1 }, { 0.0f, 1.0f, 0.0f, 1 },   //vertex
        { 1.0f, 0.0f, 0.0f, 1}, { 1.0f, 0.0f, 0.0f, 1 ,  { 1.0f, 0.0f, 0.0f, 1 },   //color
    },

    //Y-Z
    {
        { 0.0f, 0.0f, 0.0f, 1}, { 0.0f, 1.0f, 0.0f, 1 }, { 0.0f, 1.0f, 1.0f, 1 },   //vertex  
        { 0.0f, 1.0f, 0.0f, 1}, { 0.0f, 1.0f, 0.0f, 1 }, { 0.0f, 1.0f, 0.0f, 1 },   //color

        { 0.0f, 0.0f, 0.0f, 1}, { 0.0f, 1.0f, 1.0f, 1 }, { 0.0f, 0.0f, 1.0f, 1 } ,  //vertex  
        { 0.0f, 1.0f, 0.0f, 1},{ 0.0f, 1.0f, 0.0f, 1},{ 0.0f, 1.0f, 0.0f, 1},
    },
    //Z-X
    {
        { 0.0f, 0.0f, 0.0f, 1}, { 0.0f, 0.0f, 1.0f, 1 }, { 1.0f, 0.0f, 1.0f, 1 },   //vertex  
        { 0.0f, 0.0f, 1.0f, 1}, { 0.0f, 0.0f, 1.0f, 1 }, { 0.0f, 0.0f, 1.0f, 1},    //color

        { 0.0f, 0.0f, 0.0f, 1}, { 1.0f, 0.0f, 1.0f, 1 }, { 1.0f, 0.0f, 0.0f, 1 },   //vertex   
        { 0.0f, 0.0f, 1.0f, 1}, { 0.0f, 0.0f, 1.0f, 1 }, { 0.0f, 0.0f, 1.0f, 1 }    //color
    }
};


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
   

    model_matrix_loc = glGetUniformLocation(program, "model_matrix");
    scale_matrix_loc = glGetUniformLocation(program, "scale_matrix");
    projection_matrix_loc = glGetUniformLocation(program, "projection_matrix");
    
    if(model_matrix_loc == -1 ){
        std::cout << "model matrix not fetched in shader" << std::endl;
    }
    if(scale_matrix_loc == -1){
        std::cout << "scale matrix not fetched in shader" << std::endl;
    }
    if(projection_matrix_loc == -1){
        std::cout << "projection matrix not fetched in shader" << std::endl;
    }    
#undef DOTI    
#idef DOTI
    // indexed elements
    glGenBuffers(1, EBOs);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[Triangle_Ind]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_indices), vertex_indices, GL_STATIC_DRAW);
#endif
    // define buffers and associate data to buffers
    glGenVertexArrays( NumVAOs, VAOs );
    glBindVertexArray( VAOs[Planes] );
    glCreateBuffers( NumBuffers, Planes );

    // planes 
    GLint size =  sizeof(planes);
    GLsizei offset = 0;
    glBindBuffer( GL_ARRAY_BUFFER, PlaneBuffers[Mesh] );
    glNamedBufferData( PlaneBuffers[Mesh], size , NULL, GL_STATIC_DRAW);
    glNamedBufferSubData( PlaneBuffers[Mesh], offset,  size, planes);

    size = VertexDim;
    offset = 0;
    GLboolean normalize_vertices=GL_TRUE;
    GLsizei stride = sizeof(planes[0]);
    glVertexAttribPointer( vPosition, size, GL_FLOAT, normalize_vertices, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vPosition );

    size = ColorDim;
    offset = sizeof(planes[0].vertex);
    stride = sizeof(planes[0]);    
    GLboolean normalize_colors=GL_FALSE;

    glVertexAttribPointer( vColor, size, GL_FLOAT, normalize_colors, stride, (const GLvoid *)offset );
    glVertexAttribDivisor( vColor, 1 );
    glEnableVertexAttribArray( vColor );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray(0);
#idef DOTI
    // triangle 1
    GLint size =  sizeof(triangle1);
    GLsizei offset = 0;
    glBindBuffer( GL_ARRAY_BUFFER, Buffers1[Mesh] );
    glNamedBufferData( Buffers1[Mesh], size , NULL, GL_STATIC_DRAW);
    glNamedBufferSubData( Buffers1[Mesh], offset,  size, triangle1);

    size = VertexDim;
    offset = 0;
    GLboolean normalize_vertices=GL_TRUE;
    GLsizei stride = sizeof(triangle1[0]);
    glVertexAttribPointer( vPosition, size, GL_FLOAT, normalize_vertices, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vPosition );

    size = ColorDim;
    offset = sizeof(triangle1[0].vertex);
    stride = sizeof(triangle1[0]);    
    GLboolean normalize_colors=GL_FALSE;

    glVertexAttribPointer( vColor, size, GL_FLOAT, normalize_colors, stride, (const GLvoid *)offset );
    glVertexAttribDivisor( vColor, 1 );
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
    glVertexAttribPointer( vPosition, size, GL_FLOAT, normalize_vertices, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vPosition );

    size = ColorDim;
    offset = sizeof(triangle2[0].vertex);
    stride = sizeof(triangle2[0]);  
    glVertexAttribPointer( vColor, size, GL_FLOAT, normalize_colors, stride, (const GLvoid *)offset );
    glVertexAttribDivisor( vColor, 1 );
    glEnableVertexAttribArray( vColor );
#endif
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
    vmath::mat4 model_matrix=vmath:mat4::identity();

    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, projection_matrix);  

    const int ninstances=5;
    float x= 0.0f,y=0.0f,z=0.0f;

    const float tx=0.1f, ty=0.1f,tz= 0.07f;
    float X1= translate_x(x,tx,0);
    float X2= translate_x(x,tx,ninstances);
    float Y1= translate_x(y,ty,0);
    float Y2= translate_x(y,ty,ninstances);    
    float Z1= translate_z(z,tz,0);
    float Z2= translate_z(z,tz,ninstances);

    GLfloat near_plane= vmath::min(Z1,Z2)-1.0f, far_plane=vmath::max(Z1,Z2)+1.0f; 
    const GLfloat left_side= vmath::min(X1,X2)-1.0f, right_side=vmath::max(X1,X2)+1.0f;
    const GLfloat bottom= vmath::min(Y1,Y2)-1.0f, top=vmath::max(vmath::max(Y1,Y2),10*ty)+1.0f;
    
    vmath::mat4 projection_matrix(vmath::frustum(left_side,right_side, bottom, top, near_plane, far_plane) * vmath::translate(0.0f, 0.0f, -1000.0f));
 
    vmath::mat4 model_matrix=vmath::mat4::identity();
    vmath::mat4 scale_matrix=vmath::mat4::identity();
    glUseProgram(program);

    glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, projection_matrix);
    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_matrix);     
    glUniformMatrix4fv(scale_matrix_loc, 1, GL_FALSE, scale_matrix);   
     
    // Draw Arrays...
    
    static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    glClearBufferfv(GL_COLOR, 0, black);

    // planes
    glBindVertexArray( VAOs[Planes] );
    glDrawArrays( GL_TRIANGLES,  0, NumPlaneVertices );

    //triangles 
    const unsigned int n=NumVertices/2;

    // triangle 1

    glBindVertexArray( VAOs[Triangle1] ); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[Triangle_Ind]);
    glDrawArrays( GL_TRIANGLES,  0, n );

    // triangle 2
    glBindVertexArray( VAOs[Triangle2] );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[Triangle_Ind]);
    glDrawArrays( GL_TRIANGLES,  0, n );   

    // triangle 1 smaller and translated
    glBindVertexArray( VAOs[Triangle1] ); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[Triangle_Ind]);

    model_matrix = vmath::translate(x-tx, y+10*ty, z-tz);
    scale_matrix[0][0]= 0.8f;
    scale_matrix[1][1]= 0.8f;
    glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, projection_matrix);
    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_matrix);     
    glUniformMatrix4fv(scale_matrix_loc, 1, GL_FALSE, scale_matrix);     
    glDrawArraysInstanced(GL_TRIANGLES, 0, n, n);

    // loop over triangle 2 :smaller and rotated + translation
    glBindVertexArray( VAOs[Triangle2] ); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[Triangle_Ind]);


    const float theta(5.0f);
    scale_matrix[0][0]= 0.5f;
    scale_matrix[1][1]= 0.5f;      
    for (int i=0;i<ninstances;i++)
    {
        GLfloat ttx= translate_x(x,tx,i);
        GLfloat tty= translate_y(y,ty,i);
        GLfloat ttz= translate_z(z,tz,i);
        model_matrix = vmath::translate(ttx, tty, ttz);
        model_matrix = vmath::rotate(i*theta, 0.0f, 0.0f, 1.0f)*model_matrix;
        glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, projection_matrix);
        glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_matrix);   
        glUniformMatrix4fv(scale_matrix_loc, 1, GL_FALSE, scale_matrix);    
    
        glDrawArraysInstanced(GL_TRIANGLES, 0, n, n);
    }
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
