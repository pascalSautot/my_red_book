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

enum Vertex_IDs { xDim, yDim, zDim, wDim, VertexDim};
enum Color_IDs { rDim, gDim, bDim, aDim, ColorDim};

#define STRIP
typedef struct _Object {
    GLfloat vertex[VertexDim];
    GLfloat color[ColorDim];
} Object;

// define 3 planes X-Y, Y-Z, X-Z
const GLint NumPlanes = 3;
const GLint PolygonsPerPlane = 1;
const GLint VerticesPerPolygon = 4;
const GLint NumPlaneVertices = NumPlanes*PolygonsPerPlane*VerticesPerPolygon;
GLfloat planes_vertices[NumPlaneVertices][4] = {
    //X-Y
    //P1 (red)
        { 0.0f, 0.0f, 0.0f, 1 }, //vertex
        { 1.0f, 0.0f, 0.0f, 1 }, //vertex
        { 1.0f, 1.0f, 0.0f, 1 }, //vertex
        { 0.0f, 1.0f, 0.0f, 1 }, //vertex
    //Y-Z
    //P2 (blue)
        { 0.0f, 0.0f, 0.0f, 1 }, //vertex
        { 0.0f, 1.0f, 0.0f, 1 }, //vertex
        { 0.0f, 1.0f, 1.0f, 1 }, //vertex  
        { 0.0f, 0.0f, 1.0f, 1 }, //vertex  
    //Z-X
    //P3 (green)
        { 0.0f, 0.0f, 0.0f, 1 }, //vertex
        { 1.0f, 0.0f, 0.0f, 1 }, //vertex   
        { 0.0f, 0.0f, 1.0f, 1 }, //vertex 
        { 1.0f, 0.0f, 1.0f, 1 } //vertex  
};

GLfloat planes_colors[NumPlaneVertices][4] = {
    //X-Y
    //P1 (red)
        { 1.0f, 0.0f, 0.0f, 1 }, //color
        { 1.0f, 0.0f, 0.0f, 1 }, //color
        { 1.0f, 0.0f, 0.0f, 1 }, //color      
        { 1.0f, 0.0f, 0.0f, 1 }, //color        
    //Y-Z
    //P2 (blue)
        { 0.0f, 1.0f, 0.0f, 1 }, //color
        { 0.0f, 1.0f, 0.0f, 1 }, //color
        { 0.0f, 1.0f, 0.0f, 1 },   //color
        { 0.0f, 1.0f, 0.0f, 1 }, //color
    //Z-X
    //P3 (green)
        { 0.0f, 0.0f, 1.0f, 1 }, //color
        { 0.0f, 0.0f, 1.0f, 1 },    //color  
        { 0.0f, 0.0f, 1.0f, 1 }, //color
        { 0.0f, 0.0f, 1.0f, 1 } //color
};

class RefPlanes : public SimpleObject
{
public:
    RefPlanes(void):SimpleObject(),m_ref_colors(NULL),m_ref_vertices(NULL)
    {
        m_count=12;
        m_ref_vertices= new ( GLfloat * [m_count]);
        m_ref_colors= new ( GLfloat * [m_count]);
        for(unsigned int i=0;i<m_count;i++){
            m_ref_vertices[i]=new GLfloat[4];
            m_ref_colors[i]=new GLfloat[4];
        } 

        for(unsigned int j=0;j<m_count;j++){
            for(unsigned int i=0;i<VertexDim;i++){ 

            }
        }                  
    }
    virtual ~RefPlanes(void)
    {

        for(unsigned int i=0;i<m_count;i++){
            delete [] m_ref_vertices[i];
            delete [] m_ref_colors[i];
        }       
        delete [] m_ref_vertices; m_ref_vertices=NULL;
        delete [] m_ref_colors; m_ref_colors=NULL;         
        Destroy();
    }

    GLint vertex_count(void){return m_count;}
    void build_refplane(GLsizei offset, const GLboolean bx,const GLboolean by,const GLboolean bz);
    void build_ref_colors(void);
protected:
    GLfloat **m_ref_colors;  
    GLfloat **m_ref_vertices;  
};
BEGIN_APP_DECLARATION(PlanesExample)
    // Override functions from base class
    virtual void Initialize(const char * title);
    virtual void Display(bool auto_redraw);
    virtual void Resize(int width, int height);

    // Member variables
    float aspect;

    GLuint render_prog;

    GLint view_matrix_loc;
    GLint model_matrix_loc;
    GLint projection_matrix_loc;


    RefPlanes ref_planes;

    enum { vPositionLoc=0, vNormalIndexLoc=1, vTextureLoc=2, vInstanceColorLoc=3};    
    
END_APP_DECLARATION()

DEFINE_APP(PlanesExample, "Planes Example")


enum VAO_IDs { Planes, NumVAOs };
enum Buffer_IDs { Mesh, NumBuffers };
enum Attrib_IDs { vPositionLoc=0, vNormalIndexLoc=1, vTextureLoc=2, vInstanceColorLoc=3};

void RefPlanes::build_refplane(GLsizei offset, const GLboolean bx,const GLboolean by,const GLboolean bz)
{
    GLint id[3];
    if(bx &&  by){ id[0]=0,id[1]=1; id[2]=2;}  // [0,1],2 XY
    if(by &&  bz){ id[0]=1,id[1]=2; id[2]=0;}  // [1,2],0 XZ
    if(bz &&  bx){ id[0]=2,id[1]=0; id[2]=1;}  // [2,0],1 ZX

    //first vertex
    m_ref_vertices[offset][id[0]]  =0.0f;
    m_ref_vertices[offset][id[1]]  =0.0f;

    //second vertex
    m_ref_vertices[offset+1][id[0]]  =1.0f;
    m_ref_vertices[offset+1][id[1]]  =0.0f;

    //third vertex
    m_ref_vertices[offset+2][id[0]]  =1.0f;
    m_ref_vertices[offset+2][id[1]]  =1.0f;

    //fourth vertex
    m_ref_vertices[offset+3][id[0]]  =0.0f;
    m_ref_vertices[offset+3][id[1]]  =1.0f;
    
    // homogeneous variable + id[2] coord
    for(unsigned int i=0;i<4;i++)
    {
        m_ref_vertices[offset+i][3]      =1.0f;      
        m_ref_vertices[offset+i][id[2]]  =0.0f;
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
            std::cout << m_ref_vertices[j+offset][i];
            if(i<3) std::cout << ",";
        }
        std::cout << "]" << std::endl;
    }
#endif     
}
void RefPlanes::build_ref_colors(void)
{
    GLfloat red[4]={ 1.0f, 0.0f, 0.0f, 1.0f};
    GLfloat green[4]={ 0.0f, 1.0f, 0.0f, 1.0f};
    GLfloat blue[4]={ 0.0f, 0.0f, 1.0f, 1.0f};
    for(int j=0;j<4;j++)
    {
        for(int i=0;i<4;i++)
        {
            m_ref_colors[j][i]   = red[i];
            m_ref_colors[j+4][i] = green[i];
            m_ref_colors[j+8][i] = blue[i];
        }
    } 
#ifdef _DEBUG
    std::cout << "colors" << std::endl;


    for(unsigned int j=0;j<12;j++)
    {
        std::cout << "[RGBA]=[ ";
        for(unsigned int i=0;i<4;i++)
        {
            std::cout << m_ref_colors[j][i];
            if(i<3) std::cout << ",";
        }
        std::cout << "]" << std::endl;
    }
#endif  
}

void PlanesExample::Initialize(const char * title)
{

    base::Initialize(title);

    // Create the program for rendering the model
    // load shaders in program
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/planes/planes.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/planes/planes.frag" },
        { GL_NONE, NULL }
    };

    render_prog = LoadShaders( shaders );
    if( render_prog == 0) { std::cerr<<"shader program failed to build" << std::endl; }
    glUseProgram( render_prog );

   // "model_matrix" is actually an array of 4 matrices
    model_matrix_loc = glGetUniformLocation(render_prog, "model_matrix");
    //view_matrix_loc= glGetUniformLocation(render_prog, "view_matrix");
    projection_matrix_loc = glGetUniformLocation(render_prog, "projection_matrix");


/*
    ref_planes.build_ref_colors();
  
    ref_planes.build_refplane(0,true, true,   false); //XY 
    ref_planes.build_refplane(4,false,true,   true);  //YZ
    ref_planes.build_refplane(8,true, false,  true);  //XZ
*/
#ifdef _DEBUG


    for(unsigned int j=0;j<ref_planes.vertex_count();j++)
    {
        std::cout << "[x,y,z,w]=[ ";
        for(unsigned int i=0;i<4;i++)
        {
            std::cout << planes_vertices[j][i];
            if(i<3) std::cout << ",";
        }
        std::cout << "]" << std::endl;
    }

    std::cout << "colors" << std::endl;


    for(unsigned int j=0;j<12;j++)
    {
        std::cout << "[RGBA]=[ ";
        for(unsigned int i=0;i<4;i++)
        {
            std::cout << planes_colors[j][i];
            if(i<3) std::cout << ",";
        }
        std::cout << "]" << std::endl;
    }
#endif    
    ref_planes.Build(planes_vertices,planes_colors,NULL,ref_planes.vertex_count());
    ref_planes.BindVertexArray();


    glBindVertexArray(0);

}


void PlanesExample::Display(bool auto_redraw)
{
    float t = float(app_time() & 0x3FFFF) / float(0x3FFFF);

    // Clear
    static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    glClearBufferfv(GL_COLOR, 0, black);

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

    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, model_matrix);
    vmath::mat4 projection_matrix=(vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 5.0f) * vmath::translate(0.0f, 0.0f, -3.0f));

    glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, projection_matrix);

    ref_planes.Render();

    base::Display();
}

void PlanesExample::Resize(int width, int height)
{
    glViewport(0, 0 , width, height);

    aspect = float(height) / float(width);
}
