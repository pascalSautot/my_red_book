#include "mesh.h"
using namespace mesh;

#include <iostream>
#include <iomanip>

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

Quadric::Quadric(GLuint lat_count, GLuint long_count, GLfloat r):Mesh(lat_count*long_count*6),
    __pi(acos(0)*2.0f),
    m_lat_count(lat_count),m_long_count(long_count),
    m_deg_to_rad(__pi/180.0f),
    m_lat_inc((360.0f/(float)lat_count)*m_deg_to_rad),m_long_inc((360.0f/(float)long_count)*m_deg_to_rad),
    m_position(),
    m_r(r)    
{
    m_data_v_offset=0;
    m_data_c_offset=m_nelem;
    m_data_n_offset= 2*m_nelem;   
    m_ndataset =(  (int)(m_data_v_offset != -1) + (int)(m_data_c_offset != -1) + (int)(m_data_n_offset != -1) );
    for(int k=0;k<4;k++)m_light_color[k]=1.0f;        
}
Quadric::~Quadric(){}

void Quadric::render(void)
{
    glBindVertexArray( m_VAO );
    glDrawArrays( GL_TRIANGLES,  0, m_nelem );  
}
void Quadric::PatchCoordinates(GLuint la, GLuint lo, GLuint& dv)
{
    //define triangle Pla1,Pla2,Plo2 (3 vertices) 0,1,2
    vmath::vec4 &Pla1(m_vertex[dv]),&Pla2(m_vertex[dv+1]),&Plo2(m_vertex[dv+2]);
    Coordinates(la,  lo,  Pla1);
    Coordinates(la+1,lo,  Pla2);
    Coordinates(la+1,lo+1,Plo2);

    //define triangle Pla1,Plo2,Plo1 (3 vertices) 0,2,3
    m_vertex[dv+3]=Pla1;
    m_vertex[dv+4]=Plo2;
    vmath::vec4 &Plo1(m_vertex[dv+5]);
    Coordinates(la,  lo+1,Plo1);

    dv+=6; //m_vertex[dv+5]=Plo1; //already computed
}
void Quadric::setData(void)
{
    // define data
    vmath::vec4 Pla1,Pla2,Plo1,Plo2;
    //the following double loop creates (m_lat_count-1)x(m_long_count-1)x6 vertices
    GLuint dv=0;
    const GLuint la_max=m_lat_count;
    const GLuint long_max=m_long_count;
    for(GLuint  la=0; la<la_max; la++)    
    {
        for(GLuint  lo=0; lo<long_max; lo++) 
        { 
            PatchCoordinates(la,lo,dv);
        }   
    }  
    for(GLuint  i=0; i<m_nelem; i++)    
    {
        m_color[i]=m_light_color;
    }    
    GLint buffer_size = sizeof(GLfloat)*m_nelem*m_ndim*m_ndataset;
    bind_data(buffer_size);
}
// bind data using 3 different location on shader : vPosition, vColor, vNormal 
void Quadric::bind_data(GLint buffer_size) //refactor and use this method as a generic method setData  and call bind_data
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
void Quadric::debug(const std::string &s)
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
Sphere::Sphere(GLuint lat_count, GLuint long_count, GLfloat r):Quadric(lat_count,long_count,r)
{
    m_lat_inc=(360.0f/(float)lat_count)*m_deg_to_rad;
    m_long_inc=(360.0f/(float)long_count)*m_deg_to_rad;
}
Sphere::~Sphere(){}

// given m_lat_count x m_long_count values  lat_i and long_j are the ith and jth vertex 
// i in [0,m_lat_count[ 
// j in [0,m_long_count]
// Compute coordinates for a point at latitude v*m_lat_inc and longitude long_j*m_long_inc for a unit radius sphere.
void Sphere::Coordinates(GLuint lat_i, GLuint long_j, vmath::vec4& P)const
{
    //la in [0;m_lat_count-1[  => u in 0;m_lat_inc*(m_lat_count-2)
    //lo in [0;m_long_count-1[     
    const GLfloat u=((GLfloat)lat_i)*m_lat_inc;
    const GLfloat v=((GLfloat)long_j)*m_long_inc;
#ifdef _DEBUG
    std::cout << "(u,v) = " << std::setprecision(5) << u << " ,"  << v << std::endl;
#endif    
    const float cu= cos(u);
    const float su= sin(u);
    const float cv= cos(v);
    const float sv= sin(v);

    P[0]= cv*su; //X
    P[1]= sv;    //Y
    P[2]= cu*cv; //Z
    P[3] = 1.0f; 
    
}
void Sphere::setData(void)
{
    Quadric::setData();
#ifdef _DEBUG
    debug(std::string("Sphere"));
#endif    
    for(GLuint  i=0; i<m_nelem; i++)    
    {
        m_normal[i]=m_vertex[i];
    }    
}
Cylinder::Cylinder(GLuint lat_count, GLuint long_count, GLfloat r, GLfloat h):Quadric(lat_count,long_count,r),m_h(h),
    m_nelem_cyl(lat_count*long_count),m_nelem_lid(lat_count) 
{
    m_lat_count=lat_count;
    m_long_count=long_count;

    m_nelem= Nelem();

    if(m_nelem)
    {
        if(m_vertex) delete [] m_vertex; m_vertex= new VertexCoord[m_nelem];
        if(m_color) delete [] m_color; m_color= new VertexColor[m_nelem];
        if(m_normal) delete [] m_normal; m_normal= new VertexColor[m_nelem];
    }    
    m_data_v_offset=0;
    m_data_c_offset=m_nelem;
    m_data_n_offset= 2*m_nelem;   
    m_ndataset =(  (int)(m_data_v_offset != -1) + (int)(m_data_c_offset != -1) + (int)(m_data_n_offset != -1) );

    m_lat_inc=(360.0f/(float)lat_count)*m_deg_to_rad;
    m_long_inc=(m_h/(float)long_count);    
}
Cylinder::~Cylinder(){}

// given m_lat_count x m_long_count values  lat_i and long_j are the ith and jth vertex 
// i in [0,m_lat_count[ 
// j in [0,m_long_count]
// Compute coordinates for a point at latitude v*m_lat_inc and longitude long_j*m_long_inc for a unit radius sphere.
void Cylinder::Coordinates(GLuint lat_i, GLuint long_j, vmath::vec4& P)const
{
    //la in [0;m_lat_count-1[  => u in 0;m_lat_inc*(m_lat_count-2)
    //lo in [0;m_long_count-1[     
    const GLfloat u=((GLfloat)lat_i)*m_lat_inc;
    const GLfloat v=((GLfloat)long_j)*m_long_inc; //here long designate heigth
#ifdef _DEBUG
    std::cout << "(u,v) = " << std::setprecision(5) << u << " ,"  << v << std::endl;
#endif    
    const float cu= cos(u);
    const float su= sin(u);

    P[0]= su;    //X
    P[1]= v*m_h; //Y
    P[2]= cu;    //Z
    P[3] = 1.0f; 
}
void Cylinder::setData(void)
{
    //cylinder surface
    Quadric::setData();

    //upper and lower lids
    //the following double loop creates (m_lat_count-1)x(m_long_count-1)x6 vertices
    const GLuint n=m_nelem_cyl*6;
    GLuint dv=n;
    const GLuint la_max=m_lat_count;
    const GLuint long_max=m_long_count;
    for(GLuint i=0,lo=0;i<2;i++,lo+=long_max)
    {
        std::cout << "lid heigth = " << lo << std::endl;
        for(GLuint  la=0; la<la_max; la++)    
        {
            //define triangle Pl1,Pl2,Pl3 (3 vertices) 0,1,2
            vmath::vec4 &Pl1(m_vertex[dv]),&Pl2(m_vertex[dv+1]),&Pl3(m_vertex[dv+2]);
            Coordinates(la,  lo,  Pl1);
            Coordinates(la+1,lo,  Pl2);
            for(GLuint k=0;k<wDim;k++)Pl3[k]=0.0f;Pl3[yDim]=Pl2[yDim];Pl3[wDim]=1.0f;
#ifdef _DEBUG            
            for(GLuint k=0;k<3;k++)
            {
                std::cout << "[id](x,y,z) = [" << dv+k << "](";
                for(GLuint l=0;l<wDim;l++)
                {
                    std::cout << std::setprecision(5) << m_vertex[dv+k][l];
                    if(l<wDim-1)std::cout << ", ";
                }
                std::cout << ")" << std::endl;
            }
#endif            
            dv+=3;         
        }  
    }

#ifdef _DEBUG
    debug(std::string("Sphere"));
#endif        
    for(GLuint  i=0; i<m_nelem; i++)    
    {
        m_normal[i]=m_vertex[i];
    }    
}
