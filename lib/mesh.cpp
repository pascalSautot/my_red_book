#include "mesh.h"
using namespace mesh;

#include <iostream>
#include <iomanip>
static const GLfloat __pi=3.1415926535897f;

#include <cstdlib>

Mesh::Mesh(GLuint count):m_vertex(NULL),m_color(NULL),m_nelem(count),m_ndim(4),m_VAO(0),m_BO(0),
    m_data_v_offset(-1),m_data_c_offset(-1),m_data_n_offset(-1),
    m_ndataset (0 )
{
    if(m_nelem)
    {

        m_vertex= new VertexCoord[m_nelem];
        m_color= new VertexColor[m_nelem];
        m_normal= new VertexColor[m_nelem];
    }
}
Mesh::~Mesh()
{
    if(m_vertex) delete [] m_vertex;
    if(m_color) delete [] m_color;
    if(m_normal) delete [] m_normal;
}



#ifdef _DEBUG
void Mesh::debug(const std::string &s)
{
    std::cout << s.c_str() << std::endl;

    std::cout << "vertices" << std::endl;
    //vertices
    for(unsigned int j=0;j<m_nelem;j++)
    {
        std::cout << "["<<j<<"][x,y,z,w]=[ ";
        for(unsigned int i=0;i<m_ndim;i++)
        {
            std::cout << m_vertex[j][i];
            if(i<3) std::cout << ",";
        }
        std::cout << "]" << std::endl;
    }

    //colors
    std::cout << "colors" << std::endl;
    for(unsigned int j=0;j<m_nelem;j++)
    {
        std::cout << "["<<j<<"][RGBA]=[ ";
        for(unsigned int i=0;i<m_ndim;i++)
        {
            std::cout << m_color[j][i];
            if(i<3) std::cout << ",";
        }
        std::cout << "]" << std::endl;
    }
}
#endif  


Axis::Axis(GLuint count):Mesh(count)
{
    m_data_v_offset=0;
    m_data_c_offset=count;
    m_data_n_offset= -1;
    m_ndataset =(  (int)(m_data_v_offset != -1) + (int)(m_data_c_offset != -1) + (int)(m_data_n_offset != -1) );
}
Axis::~Axis(){}
void Axis::render(void)
{
    glBindVertexArray( m_VAO );
    glDrawArrays( GL_LINES,  0, m_nelem );        
}    

void Axis::push_data(const GLfloat v[2][4], const GLfloat c[2][4])
{
    for(unsigned int j=0;j<m_nelem;j++)
    {
        for (unsigned int i=0;i<m_ndim;i++)
        {
            m_vertex[j][i] = v[j][i];
            m_color [j][i] = c[j][i];
        }
    }
}
void Axis::bind_data(GLint buffer_size)
{
    glGenVertexArrays( 1, &m_VAO );
    glBindVertexArray( m_VAO );        
    glCreateBuffers( 1, &m_BO );
       
    glBindBuffer( GL_ARRAY_BUFFER, m_BO );
    glNamedBufferData( m_BO, buffer_size , NULL, GL_STATIC_DRAW);
    GLint size=sizeof(GLfloat)*m_nelem*m_ndim;
    GLsizei offset=sizeof(GLfloat)*m_data_v_offset*m_ndim;
    glNamedBufferSubData( m_BO, offset,  size, m_vertex);
    offset=sizeof(GLfloat)*m_data_c_offset*m_ndim;
    glNamedBufferSubData( m_BO, offset,  size, m_color);

    size = 4;
    offset = sizeof(GLfloat)*m_data_v_offset*m_ndim;
    GLboolean normalize_vertices=GL_TRUE;
    GLsizei stride = 0;
    glVertexAttribPointer( vPosition, size, GL_FLOAT, normalize_vertices, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vPosition );

    size = 4;
    offset = sizeof(GLfloat)*m_data_c_offset*m_ndim;
    stride = 0;  
    GLboolean normalize_colors=GL_TRUE;

    glVertexAttribPointer( vColor, size, GL_FLOAT, normalize_colors, stride, (const GLvoid *)offset);
    glEnableVertexAttribArray( vColor );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray(0);    
}

XAxis::XAxis():Axis(){}
XAxis::~XAxis(){}
void XAxis::setData(void)
{
    const GLfloat v_x[2][4]={
        { 0.0f, 0.0f, 0.0f, 1 }, //vertex 0
        { 5.0f, 0.0f, 0.0f, 1 }  //vertex 1
    };
    const GLfloat c_x[2][4]={
        { 1.0f, 0.0f, 0.0f, 1 }, //color
        { 1.0f, 0.0f, 0.0f, 1 } //color
    };
    push_data(v_x,c_x);
#ifdef _DEBUG
    debug(std::string("X axis"));
#endif      
    GLint buffer_size = sizeof(GLfloat)*m_nelem*m_ndim*2;
    bind_data(buffer_size);
 
}    

YAxis::YAxis():Axis(){}
YAxis::~YAxis(){}
void YAxis::setData(void)
{
    const GLfloat v_y[2][4]={
        { 0.0f, 0.0f, 0.0f, 1 }, //vertex 0
        { 0.0f, 5.0f, 0.0f, 1 }  //vertex 1
    };
    const GLfloat c_y[2][4]={
        { 0.0f, 1.0f, 0.0f, 1 }, //color
        { 0.0f, 1.0f, 0.0f, 1 } //color
    };
    push_data(v_y,c_y);
#ifdef _DEBUG
    debug(std::string("Y axis"));
#endif            
    GLint buffer_size = sizeof(GLfloat)*m_nelem*m_ndim*2;
    bind_data(buffer_size);

}    

ZAxis::ZAxis():Axis(){}
ZAxis::~ZAxis(){}
void ZAxis::setData(void)
{
    const GLfloat v_z[2][4]={
        { 0.0f, 0.0f, 0.0f, 1 }, //vertex 0
        { 0.0f, 0.0f, 5.0f, 1 }  //vertex 1
    };
    const GLfloat c_z[2][4]={
        { 0.0f, 0.0f, 1.0f, 1 }, //color
        { 0.0f, 0.0f, 1.0f, 1 } //color
    };
    push_data(v_z,c_z);
#ifdef _DEBUG
    debug(std::string("Z axis"));
#endif       
    GLint buffer_size = sizeof(GLfloat)*m_nelem*m_ndim*2;
    bind_data(buffer_size);

}    

Plane::Plane(GLuint count):Mesh(count)
{
    m_data_v_offset=0;
    m_data_c_offset=count;
    m_data_n_offset= -1;  
    m_ndataset =(  (int)(m_data_v_offset != -1) + (int)(m_data_c_offset != -1) + (int)(m_data_n_offset != -1) );
}
Plane::~Plane(){}
void Plane::render(void)
{
    glBindVertexArray( m_VAO );
    glDrawArrays( GL_TRIANGLE_STRIP,  0, m_nelem );        
}    

void Plane::push_data(const GLfloat v[4][4], const GLfloat c[4][4])
{
    for(unsigned int j=0;j<m_nelem;j++)
    {
        for (unsigned int i=0;i<m_ndim;i++)
        {
            m_vertex[j][i] = v[j][i];
            m_color [j][i] = c[j][i];
        }
    }
   
}
void Plane::bind_data(GLint buffer_size)
{
    glGenVertexArrays( 1, &m_VAO );
    glBindVertexArray( m_VAO );        
    glCreateBuffers( 1, &m_BO );
    glBindBuffer( GL_ARRAY_BUFFER, m_BO );
    glNamedBufferData( m_BO, buffer_size , NULL, GL_STATIC_DRAW);
    GLint size=sizeof(GLfloat)*m_nelem*m_ndim;
    GLsizei offset=sizeof(GLfloat)*m_data_v_offset*m_ndim;
    glNamedBufferSubData( m_BO, offset,  size, m_vertex);

    offset=sizeof(GLfloat)*m_data_c_offset*m_ndim;
    glNamedBufferSubData( m_BO, offset,  size, m_color);

    size = 4;
    offset = sizeof(GLfloat)*m_data_v_offset*m_ndim;
    GLboolean normalize_vertices=GL_TRUE;
    GLsizei stride = 0;
    glVertexAttribPointer( vPosition, size, GL_FLOAT, normalize_vertices, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vPosition );

    size = 4;
    offset = sizeof(GLfloat)*m_data_c_offset*m_ndim;
    stride = 0;  
    GLboolean normalize_colors=GL_TRUE;

    glVertexAttribPointer( vColor, size, GL_FLOAT, normalize_colors, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vColor );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray(0);    
}

XYPlane::XYPlane(GLuint count):Plane(count){}
XYPlane::~XYPlane(){}
void XYPlane::setData(void)
{
    const GLfloat v_xy[4][4]={
        { 0.0f, 0.0f, 0.0f, 1 }, //vertex 0
        { 1.0f, 0.0f, 0.0f, 1 }, //vertex 1
        { 0.0f, 1.0f, 0.0f, 1 }, //vertex 3
        { 1.0f, 1.0f, 0.0f, 1 }  //vertex 2
    };
    const GLfloat c_xy[4][4]={
        { 1.0f, 1.0f, 0.0f, 1 }, //color
        { 1.0f, 1.0f, 0.0f, 1 }, //color
        { 1.0f, 1.0f, 0.0f, 1 }, //color      
        { 1.0f, 1.0f, 0.0f, 1 }  //color        
    };
    push_data(v_xy,c_xy);
#ifdef _DEBUG
    debug(std::string("XY plane"));
#endif     
    GLint buffer_size = sizeof(GLfloat)*m_nelem*m_ndim*2; // 2 one for vertices one for colors
    bind_data(buffer_size);
      

}

YZPlane::YZPlane(GLuint count):Plane(count){}
YZPlane::~YZPlane(){}
void YZPlane::setData(void)
{
    const GLfloat v_yz[4][4] ={
        { 0.0f, 0.0f, 0.0f, 1 }, //vertex 0
        { 0.0f, 1.0f, 0.0f, 1 }, //vertex 1
        { 0.0f, 0.0f, 1.0f, 1 }, //vertex 3 
        { 0.0f, 1.0f, 1.0f, 1 }  //vertex 2 
    };
    const GLfloat c_yz[4][4]={
        { 0.0f, 1.0f, 1.0f, 1 }, //color
        { 0.0f, 1.0f, 1.0f, 1 }, //color
        { 0.0f, 1.0f, 1.0f, 1 }, //color
        { 0.0f, 1.0f, 1.0f, 1 }  //color
    };
    push_data(v_yz,c_yz);
#ifdef _DEBUG
    debug(std::string("YZ plane"));
#endif          
    GLint buffer_size = sizeof(GLfloat)*m_nelem*m_ndim*2; // 2 one for vertices one for colors
    bind_data(buffer_size);
}

ZXPlane::ZXPlane(GLuint count):Plane(count){}
ZXPlane::~ZXPlane(){}
void ZXPlane::setData(void)
{
    const GLfloat v_zx[4][4]={
        { 0.0f, 0.0f, 0.0f, 1 }, //vertex
        { 1.0f, 0.0f, 0.0f, 1 }, //vertex   
        { 0.0f, 0.0f, 1.0f, 1 }, //vertex 
        { 1.0f, 0.0f, 1.0f, 1 }  //vertex  
    };
    const GLfloat c_zx[4][4]={
        { 1.0f, 0.0f, 1.0f, 1 }, //color
        { 1.0f, 0.0f, 1.0f, 1 }, //color  
        { 1.0f, 0.0f, 1.0f, 1 }, //color
        { 1.0f, 0.0f, 1.0f, 1 }  //color
    };
    push_data(v_zx,c_zx);
#ifdef _DEBUG
    debug(std::string("ZX plane"));
#endif
    GLint buffer_size = sizeof(GLfloat)*m_nelem*m_ndim*2; // 2 one for vertices one for colors
    bind_data(buffer_size);
}

Sphere::Sphere(GLuint lat_count, GLuint long_count, GLfloat r):Mesh((lat_count-1)*(long_count-1)*6),
    m_lat_count(lat_count),m_long_count(long_count),m_r(r),
    m_deg_to_rad(__pi/180.0f),
    m_lat_inc((360.0f/(float)lat_count)*m_deg_to_rad),m_long_inc((360.0f/(float)long_count)*m_deg_to_rad),
    m_position()
{
    m_data_v_offset=0;
    m_data_c_offset=m_nelem;
    m_data_n_offset= 2*m_nelem;   
    m_ndataset =(  (int)(m_data_v_offset != -1) + (int)(m_data_c_offset != -1) + (int)(m_data_n_offset != -1) );
    for(int k=0;k<4;k++)m_light_color[k]=1.0f;
}
Sphere::~Sphere(){}
void Sphere::render(void)
{
    glBindVertexArray( m_VAO );
    glDrawArrays( GL_TRIANGLES,  0, m_nelem );  
}



// given m_lat_count x m_long_count values  lat_i and long_j are the ith and jth vertex 
// i in [0,m_lat_count[ 
// j in [0,m_long_count]
// Compute coordinates for a point at latitude v*m_lat_inc and longitude long_j*m_long_inc for a unit radius sphere.
void Sphere::SphericalCoordinates(GLuint lat_i, GLuint long_j, vmath::vec4& P)const
{
    //la in [0;m_lat_count-1[  => u in 0;m_lat_inc*(m_lat_count-2)
    //lo in [0;m_long_count-1[     
    const GLfloat u=((GLfloat)lat_i)*m_lat_inc;
    const GLfloat v=((GLfloat)long_j)*m_long_inc;
#ifdef _DEBUG
    std::cout << "(u,v) = " << std::setprecision(5) << u << " ,"  << v << std::endl;
#endif    
    P[0]= std::cos(u)*sin(v);
    P[1]= std::cos(v);
    P[2]= std::sin(u)*sin(v);
    P[3] = 1.0f; 
}
void Sphere::PatchCoordinates(GLuint la, GLuint lo, GLuint& dv)
{
    //define triangle Pla1,Pla2,Plo2 (3 vertices) 0,1,2
    vmath::vec4 &Pla1(m_vertex[dv]),&Pla2(m_vertex[dv+1]),&Plo2(m_vertex[dv+2]);
    SphericalCoordinates(la,  lo,  Pla1);
    SphericalCoordinates(la+1,lo,  Pla2);
    SphericalCoordinates(la+1,lo+1,Plo2);

    //define triangle Pla1,Plo2,Plo1 (3 vertices) 0,2,3
    m_vertex[dv+3]=Pla1;
    m_vertex[dv+4]=Plo2;
    vmath::vec4 &Plo1(m_vertex[dv+5]);
    SphericalCoordinates(la,  lo+1,Plo1);

    dv+=6; //m_vertex[dv+5]=Plo1; //already computed

}
void Sphere::setData(void)
{
    // define data
    vmath::vec4 Pla1,Pla2,Plo1,Plo2;
    //the following double loop creates (m_lat_count-1)x(m_long_count-1)x6 vertices
    GLuint dv=0;
    const GLuint la_max=m_lat_count-1;
    const GLuint long_max=m_long_count-1;
    for(GLuint  la=0; la<la_max; la++)    
    {
        for(GLuint  lo=0; lo<long_max; lo++) 
        { 
            PatchCoordinates(la,lo,dv);
        }   
    }  
    for(GLuint  i=0; i<m_nelem; i++)    
    {
        m_normal[i]=m_vertex[i];
        m_color[i]=m_light_color;
    }


#ifdef _DEBUG
    debug(std::string("Sphere"));
#endif    
    GLint buffer_size = sizeof(GLfloat)*m_nelem*m_ndim*m_ndataset;
    bind_data(buffer_size);
}
// bind data using 3 different location on shader : vPosition, vColor, vNormal 
void Sphere::bind_data(GLint buffer_size) //refactor and use this method as a generic method setData  and call bind_data
{
    glGenVertexArrays( 1, &m_VAO );
    glBindVertexArray( m_VAO );        
    glCreateBuffers( 1, &m_BO );
 
    glBindBuffer( GL_ARRAY_BUFFER, m_BO );
    glNamedBufferData( m_BO, buffer_size , NULL, GL_STATIC_DRAW);
    GLint size=sizeof(GLfloat)*m_nelem*m_ndim;
    GLsizei offset=sizeof(GLfloat)*m_data_v_offset*m_ndim;
    glNamedBufferSubData( m_BO, offset,  size, m_vertex);

    offset=sizeof(GLfloat)*m_data_c_offset*m_ndim;
    glNamedBufferSubData( m_BO, offset,  size, m_color);

    offset=sizeof(GLfloat)*m_data_n_offset*m_ndim;
    glNamedBufferSubData( m_BO, offset,  size, m_normal);    

    size = 4;
    offset = sizeof(GLfloat)*m_data_v_offset*m_ndim;
    GLboolean normalize_vertices=GL_TRUE;
    GLsizei stride = 0;
    glVertexAttribPointer( vPosition, size, GL_FLOAT, normalize_vertices, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vPosition );

    size = 4;
    offset = sizeof(GLfloat)*m_data_c_offset*m_ndim;
    stride = 0;  
    GLboolean normalize_colors=GL_TRUE;

    glVertexAttribPointer( vColor, size, GL_FLOAT, normalize_colors, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vColor );

    size = 4;
    offset = sizeof(GLfloat)*m_data_n_offset*m_ndim;
    stride = 0;  
    GLboolean normalize_normals=GL_TRUE;

    glVertexAttribPointer( vNormal, size, GL_FLOAT, normalize_normals, stride, (const GLvoid *)offset );
    glEnableVertexAttribArray( vNormal );    

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray(0);    

}
#ifdef _DEBUG
void Sphere::debug(const std::string &s)
{
    std::cout << s.c_str() << std::endl;

    std::cout << "vertices" << std::endl;
    //vertices
    GLuint k=0;
    for(unsigned int j=0;j<m_nelem;j++)
    {
        if(k==0) std::cout << "["<<j<<","<<j+1<<","<<j+2<<"][x,y,z,w][x,y,z,w][x,y,z,w]=";
        std::cout << "[ ";
        for(unsigned int i=0;i<m_ndim;i++)
        {
            std::cout << std::setprecision(5) << m_vertex[j][i];
            if(i<3) std::cout << ",";
        }
        std::cout << "] ";
        k++;
        if(k>=3) {
            k=0;
            std::cout <<std::endl;
        }
    }

    //colors
    std::cout << "colors" << std::endl;
    unsigned int j=0;
    std::cout << "[*][RGBA]=[ ";
    for(unsigned int i=0;i<m_ndim;i++)
    {
        std::cout << std::setprecision(5) << m_color[j][i];
        if(i<3) std::cout << ",";
    }
    std::cout << "] " <<std::endl;

}
#endif  