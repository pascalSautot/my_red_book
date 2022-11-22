#define _CRT_SECURE_NO_WARNINGS

#include "vbm.h"
#include "vgl.h"

#include <stdio.h>
#include <limits>
BaseObject::BaseObject(void):
    m_vao(0),
    m_attribute_buffer(0),
    m_index_buffer(0)
{}
BaseObject::~BaseObject(void)
{

}
GLint SimpleObject::m_dim=4;

SimpleObject::SimpleObject(void):BaseObject(),
    m_count(0),m_first(0)
{
   for(int i=0;i<3;i++)m_data[i]=NULL;
}
SimpleObject::~SimpleObject(void)
{
    Destroy(); 
}
void SimpleObject::Render(unsigned int frame_index, unsigned int instances)
{
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, m_first, m_count);
    glBindVertexArray(0);
}
void SimpleObject::Build(GLfloat *vertices[4], GLfloat *colors[4], GLfloat *normals[4],const GLint count)
{
    m_first=0;

    GLint size=0;
    GLsizei offset = 0;

    // planes 
    size =  sizeof(GLfloat)*count*m_dim;
    offset = 0;
    GLfloat **data[3];   
    data[vertexIndex]=vertices;
    data[colorIndex]=colors;
    data[normalIndex]=normals;     
    const bool some_data= (vertices!=NULL) || (colors!=NULL) || ( normals!=NULL);
    
    if(count && some_data)
    {
        // define buffers and associate data to buffers
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        glCreateBuffers( 1, &m_attribute_buffer );   

        GLint total_size=0;
        for(unsigned int j=0;j<numIndex;j++)
        {
            if(data[j]!=NULL)
            {
                total_size+=size;
            }
        }

        glBindBuffer( GL_ARRAY_BUFFER, m_attribute_buffer );
        glNamedBufferData( m_attribute_buffer, total_size , NULL, GL_STATIC_DRAW);

        const size_t n(sizeof(GLfloat)*m_dim);
        m_count= count;
        //foreach chunk of data (coords,colors,normals)
        for(unsigned int j=0;j<numIndex;j++)
        {
            if(data[j]!=NULL)
            {
                m_data[j]= new (GLfloat* [m_count]);
                // for each vertex copy data coord,color,normal
                for(unsigned int i=0;i<m_count;i++)
                {
                    m_data[j][i]= new GLfloat[m_dim];
                    memcpy(m_data[j][i],data[j][i],n);
                }
                size= n*m_count;
                glNamedBufferSubData( m_attribute_buffer, offset,  size, m_data[j]); 
                GLboolean normalize_vertices=GL_TRUE;
                GLsizei stride = 0;    
                GLint attrib_size=4;            
                glVertexAttribPointer(j, attrib_size, GL_FLOAT, normalize_vertices, stride,(GLvoid *)offset);
                glEnableVertexAttribArray(j);         

                offset += size;
            }         
        }
    }
    glBindVertexArray(0);
}
void SimpleObject::Destroy(void)
{
    for(unsigned int j=0;j<numIndex;j++)
    {
        if(m_data[j])
        {
            GLfloat **p=m_data[j];
            for(unsigned int i=0;i<m_count;i++) delete [] p[i];
            delete [] p;
            m_data[j]=NULL;
        }        
    }
     
    if(m_index_buffer)glDeleteBuffers(1, &m_index_buffer);
    m_index_buffer = 0;
    if(m_attribute_buffer)glDeleteBuffers(1, &m_attribute_buffer);
    m_attribute_buffer = 0;
    if(m_vao)glDeleteVertexArrays(1, &m_vao);
    m_vao = 0;      
}

VBObject::VBObject(void): BaseObject(),
      m_attrib(0),
      m_frame(0),
      m_material(0)
{
    for(int i=0; i<3; i++)
    {
    m_bb.min[i]= std::numeric_limits<float>::max();
    m_bb.max[i]= std::numeric_limits<float>::min();
    }

}

VBObject::~VBObject(void)
{
    Free();
}

bool VBObject::LoadFromVBM(const char * filename, int vertexIndex, int normalIndex, int texCoord0Index)
{
    FILE * f = NULL;

    f = fopen(filename, "rb");
    if(f == NULL)
        return false;

    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char * data = new unsigned char [filesize];
    unsigned char * raw_data;
    fread(data, filesize, 1, f);
    fclose(f);

    VBM_HEADER_OLD * oldHeader = (VBM_HEADER_OLD *)data;
    VBM_HEADER * header = (VBM_HEADER *)data;
    raw_data = data + header->size + header->num_attribs * sizeof(VBM_ATTRIB_HEADER) + header->num_frames * sizeof(VBM_FRAME_HEADER);
    VBM_ATTRIB_HEADER * attrib_header = (VBM_ATTRIB_HEADER *)(data + header->size);
    VBM_FRAME_HEADER * frame_header = (VBM_FRAME_HEADER *)(data + header->size + header->num_attribs * sizeof(VBM_ATTRIB_HEADER));
    unsigned int total_data_size = 0;

    if (header->magic == 0x314d4253)
    {
        memset(&m_header, 0, sizeof(m_header));
        memcpy(&m_header, header, header->size > sizeof(VBM_HEADER) ? sizeof(VBM_HEADER) : header->size);
    }
    else
    {
        memcpy(&m_header, oldHeader, sizeof(VBM_HEADER));
        m_header.num_vertices = oldHeader->num_vertices;
        m_header.num_indices = oldHeader->num_indices;
        m_header.index_type = oldHeader->index_type;
        m_header.num_materials = oldHeader->num_materials;
        m_header.flags = oldHeader->flags;
    }
    m_attrib = new VBM_ATTRIB_HEADER[m_header.num_attribs];
    memcpy(m_attrib, attrib_header, m_header.num_attribs * sizeof(VBM_ATTRIB_HEADER));
    m_frame = new VBM_FRAME_HEADER[m_header.num_frames];
    memcpy(m_frame, frame_header, m_header.num_frames * sizeof(VBM_FRAME_HEADER));

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glGenBuffers(1, &m_attribute_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_attribute_buffer);

    unsigned int i;

    for (i = 0; i < m_header.num_attribs; i++) {
        total_data_size += m_attrib[i].components * sizeof(GLfloat) * m_header.num_vertices;
    }

    GLfloat *pv=(GLfloat *) raw_data;
    for (i = 0; i < m_header.num_vertices; i++) 
    {
        for(int k=0;k<3;k++)
        {
            if(m_bb.min[k]> *pv)m_bb.min[k]= *pv;
            if(m_bb.max[k]< *pv)m_bb.max[k]= *pv;
            pv++;
        }
    }  
    m_bb.l_min= std::numeric_limits<float>::max();
    m_bb.l_max= std::numeric_limits<float>::min();
    for(int k=1;k<3;k++)
    {
        float l=m_bb.max[k]-m_bb.min[k];
        if(m_bb.l_min> l)m_bb.l_min= l;
        if(m_bb.l_max< l)m_bb.l_max= l;
    }
    glBufferData(GL_ARRAY_BUFFER, total_data_size, raw_data, GL_STATIC_DRAW);

    total_data_size = 0;

    for (i = 0; i < m_header.num_attribs; i++) {
        int attribIndex = i;

        if(attribIndex == 0)
            attribIndex = vertexIndex;
        else if(attribIndex == 1)
            attribIndex = normalIndex;
         else if(attribIndex == 2)
            attribIndex = texCoord0Index;

        glVertexAttribPointer(attribIndex, m_attrib[i].components, m_attrib[i].type, GL_FALSE, 0, (GLvoid *)total_data_size);
        glEnableVertexAttribArray(attribIndex);
        total_data_size += m_attrib[i].components * sizeof(GLfloat) * header->num_vertices;
    }

    if (m_header.num_indices) {
        glGenBuffers(1, &m_index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
        unsigned int element_size;
        switch (header->index_type) {
            case GL_UNSIGNED_SHORT:
                element_size = sizeof(GLushort);
                break;
            default:
                element_size = sizeof(GLuint);
                break;
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_header.num_indices * element_size, raw_data + total_data_size, GL_STATIC_DRAW);
    }

    glBindVertexArray(0);

    if (m_header.num_materials != 0)
    {
        m_material = new VBM_MATERIAL[m_header.num_materials];
        memcpy(m_material, raw_data + total_data_size, m_header.num_materials * sizeof(VBM_MATERIAL));
        total_data_size += m_header.num_materials * sizeof(VBM_MATERIAL);
        m_material_textures = new VBObject::material_texture[m_header.num_materials];
        memset(m_material_textures, 0, m_header.num_materials * sizeof(*m_material_textures));
    }

    /*
    if (m_header.num_chunks != 0)
    {
        m_chunks = new VBM_RENDER_CHUNK[m_header.num_chunks];
        memcpy(m_chunks, raw_data + total_data_size, m_header.num_chunks * sizeof(VBM_RENDER_CHUNK));
        total_data_size += m_header.num_chunks * sizeof(VBM_RENDER_CHUNK);
    }
    */

    delete [] data;

    return true;
}

bool VBObject::Free(void)
{
    glDeleteBuffers(1, &m_index_buffer);
    m_index_buffer = 0;
    glDeleteBuffers(1, &m_attribute_buffer);
    m_attribute_buffer = 0;
    glDeleteVertexArrays(1, &m_vao);
    m_vao = 0;

    delete [] m_attrib;
    m_attrib = NULL;

    delete [] m_frame;
    m_frame = NULL;

    delete [] m_material;
    m_material = NULL;

    return true;
}

void VBObject::Render(unsigned int frame_index, unsigned int instances)
{
    if (frame_index >= m_header.num_frames)
        return;

    glBindVertexArray(m_vao);

    /*
    if (m_header.num_chunks)
    {
        unsigned int chunk = 6; // (t >> 1) % m_header.num_chunks;

        for (chunk = 0; chunk < m_header.num_chunks; chunk++)
        {
            unsigned int material_index = m_chunks[chunk].material_index;
            // if (m_material_textures[material_index].normal != 0)
            {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, m_material_textures[material_index].normal);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, m_material_textures[material_index].specular);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_material_textures[material_index].diffuse);
                VBM_MATERIAL * material = &m_material[m_chunks[chunk].material_index];
                glDrawArrays(GL_TRIANGLES, m_chunks[chunk].first, m_chunks[chunk].count);
            }
        }
    }
    else
    */
    {
        if (instances) {
            if (m_header.num_indices)
                glDrawElementsInstanced(GL_TRIANGLES, m_frame[frame_index].count, GL_UNSIGNED_INT, (GLvoid *)(m_frame[frame_index].first * sizeof(GLuint)), instances);
            else
                glDrawArraysInstanced(GL_TRIANGLES, m_frame[frame_index].first, m_frame[frame_index].count, instances);
        } else {
            if (m_header.num_indices)
                glDrawElements(GL_TRIANGLES, m_frame[frame_index].count, GL_UNSIGNED_INT, (GLvoid *)(m_frame[frame_index].first * sizeof(GLuint)));
            else
                glDrawArrays(GL_TRIANGLES, m_frame[frame_index].first, m_frame[frame_index].count);
        }
    }
    glBindVertexArray(0);
}
