#ifndef _MESH_H_
#define _MESH_H_

#include "vgl.h"
#include "vmath.h"

#include <string>

namespace mesh {


enum VertexDim { xDim, yDim, zDim, wDim, VertexDim};
enum ColorDim { rDim, gDim, bDim, aDim, ColorDim};
typedef vmath::vec4 VertexCoord;
typedef vmath::vec4 VertexColor;

class Mesh 
{
public:
    Mesh(GLuint count=0);
    virtual void setData(void)=0;
    virtual void bind_data(GLint buffer_size)=0;
    
    virtual ~Mesh();
    const VertexCoord *Vertices(void)const{return m_vertex;}
    const VertexCoord *Colors(void)const{return m_color;}
    enum Attrib_IDs { vPosition = 0, vNormal=1, vColor=3 };
protected:
#ifdef _DEBUG
    void debug(const std::string &s);
#endif  
 

    VertexCoord *m_vertex;
    VertexColor *m_color;
    VertexColor *m_normal;
    
    GLuint  m_VAO; //vertex array object 
    GLuint  m_BO;  //buffer object    
    GLint m_data_v_offset;
    GLint m_data_c_offset;
    GLint m_data_n_offset;

    GLuint m_nelem;
    const GLuint m_ndim;
    GLuint m_ndataset; // vertex? normal? color ?    
};


class Axis : public Mesh
{
public:
    Axis(GLuint count=2);
    virtual ~Axis();
    void render(void);

protected:
    void push_data(const GLfloat v[2][4], const GLfloat c[2][4]);
    void bind_data(GLint buffer_size);

};

class XAxis : public Axis 
{
public:
    XAxis();
    virtual ~XAxis();
    void setData(void);
};
class YAxis : public Axis 
{
public:
    YAxis();
    virtual ~YAxis();
    void setData(void);
};
class ZAxis : public Axis 
{
public:
    ZAxis();
    virtual ~ZAxis();
    void setData(void);
};
class Plane : public Mesh
{
public:
    Plane(GLuint count=4);
    virtual ~Plane();
    void render(void);

protected:
    void push_data(const GLfloat v[4][4], const GLfloat c[4][4]);
    void bind_data(GLint buffer_size);
};
class XYPlane : public Plane
{
public:
    XYPlane(GLuint count=4);
    virtual ~XYPlane();
    void setData(void);
};
class YZPlane : public Plane
{
public:
    YZPlane(GLuint count=4);
    virtual ~YZPlane();
    void setData(void);

};
class ZXPlane : public Plane
{
public:
    ZXPlane(GLuint count=4);
    virtual ~ZXPlane();
    void setData(void);
};

class Quadric : public Mesh 
{
public:
    Quadric(GLuint lat_count=36, GLuint long_count=36, GLfloat r=1.0);
    virtual ~Quadric();

    void setPosition(vmath::vec4 &position){m_position=position;}
    vmath::vec3 Position()const{return vmath::vec3(m_position[0],m_position[1],m_position[2]);}

    void setRadius(GLfloat r){m_r=r;}
    GLfloat Radius()const{return m_r;}     

    void setColor(vmath::vec4 &color){m_light_color=color;}
    void render(void);

    void setData(void);
protected:
#ifdef _DEBUG
    void debug(const std::string &s);
#endif  
    void bind_data(GLint buffer_size);
 
    //compute triangles for two successive latitude increments
    void PatchCoordinates(GLuint la, GLuint lo, GLuint& dv);  
    //compute triangle for couple indices
    virtual void Coordinates(GLuint lat_i, GLuint long_j, vmath::vec4& P)const=0;
    GLuint m_lat_count,m_long_count;    
    const GLfloat __pi;
    const GLfloat m_deg_to_rad;  
    GLfloat m_lat_inc,m_long_inc;
    vmath::vec4 m_position;    // use in vertex shader to translate vertex coords
    vmath::vec4 m_light_color;    
    GLfloat m_r;   // use in vertex shader to scale vertex coords    
}; // class Quadric

class Sphere : public Quadric 
{
public:
    Sphere(GLuint lat_count=36, GLuint long_count=36, GLfloat r=1.0);
    virtual ~Sphere();
    void setData(void);
protected:
    //compute triangle for couple angles
    void Coordinates(GLuint lat_i, GLuint long_j, vmath::vec4& P)const;
}; // class Sphere

class Cylinder : public Quadric 
{
public:
    Cylinder(GLuint lat_count=36, GLuint long_count=36, GLfloat r=1.0, GLfloat h=1.0);
    virtual ~Cylinder();

    void setRadius(GLfloat r){m_r=r;}
    GLfloat Radius()const{return m_r;}   
    void setHeigth(GLfloat h){m_h=h;}
    GLfloat Heigth()const{return m_h;}   

    void setData(void);

protected:

    //compute triangle for couple angles
    void Coordinates(GLuint lat_i, GLuint long_j, vmath::vec4& P)const;
 
    GLfloat m_h;
    GLuint Nelem(void)const{ return(m_nelem_cyl*6+(2*m_nelem_lid)*3);}
    GLuint m_nelem_cyl,m_nelem_lid; // to be used by Nelem 

}; // class Cyclinder

class OrientedAxis : public Axis
{
public:
    OrientedAxis(vmath::vec3 &base, vmath::vec3 &tip);
    virtual ~OrientedAxis();
    void setData(void);
protected:
    vmath::vec3 m_base,m_tip,m_axis;
};
class RobotAxis 
{
public:
    RobotAxis(vmath::vec3 &base, vmath::vec3 &tip, GLfloat r=1.0, GLfloat length=1.0);
    virtual ~RobotAxis();
protected:
    Cylinder *cylinder;
    OrientedAxis *axis;    
};

}; //namespace mesh

#endif _MESH_H_