/* $URL$
   $Rev$
   $Author$
   $Date$
   $Id$
 */

#define USE_GL3W
#include <vermilion.h>

#include "vapp.h"
#include "vutils.h"
#include "vbm.h"

#include "vmath.h"

#include <stdio.h>

BEGIN_APP_DECLARATION(VolumeTextureExample)
    // Override functions from base class
    virtual void Initialize(const char * title);
    virtual void Display(bool auto_redraw);
    virtual void Finalize(void);
    virtual void Resize(int width, int height);

    // Member variables
    float aspect;
    GLuint base_prog;
    GLuint vao;

    GLuint quad_vbo;

    GLuint tex;
    GLint tc_rotate_loc;
    enum ids {VertexId=0, TextureId=1 };
END_APP_DECLARATION()

DEFINE_APP(VolumeTextureExample, "Volume Texture Example")

void VolumeTextureExample::Initialize(const char * title)
{
    base::Initialize(title);

    //load and compile shaders
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/volume/shader.vert.glsl" },
        { GL_FRAGMENT_SHADER, "media/shaders/volume/shader.frag.glsl" },
        { GL_NONE, NULL }
    };

    bool status(false);
    base_prog = LoadShaders( shaders);
    status=( base_prog != 0);
    if( !status) 
    { 
        std::cerr<<"shader program failed to build" << std::endl; 
    }
    else
    {
        tc_rotate_loc = glGetUniformLocation(base_prog, "tc_rotate");  
        if(tc_rotate_loc == -1 ){
            std::cout << "tc_rotate not fetched in shader" << std::endl;
            status=false;
        }              
    }
    if(!status) exit(0);

    // define buffer object, set data and bind buffer object
    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    const GLuint position_count=8;
    const GLuint texture_count=8;

    static const GLfloat quad_data[position_count+texture_count] =
    {
         1.0f, -1.0f,
        -1.0f, -1.0f,
        -1.0f, 1.0f,
         1.0f, 1.0f,

         0.0f, 0.0f,
         1.0f, 0.0f,
         1.0f, 1.0f,
         0.0f, 1.0f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    const GLsizei vertex_position_size =2; 
    const GLsizei texture_coord_size =2; 
    const GLboolean normalize = false;
    const GLsizei stride=0;
    const GLint att_type=GL_FLOAT;
    const GLuint position_offset=position_count*sizeof(GLfloat);

    glVertexAttribPointer(VertexId, vertex_position_size, att_type, normalize, stride, BUFFER_OFFSET(0));
    glVertexAttribPointer(TextureId, texture_coord_size, att_type, normalize, stride, BUFFER_OFFSET(position_offset));

    glEnableVertexAttribArray(VertexId);
    glEnableVertexAttribArray(TextureId);

    // load texture from file, define buffer and bind buffer
    vglImageData image_texture;
    tex = vglLoadTexture("media/cloud.dds", 0, &image_texture);

    glTexParameteri(image_texture.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    vglUnloadImage(&image_texture);
}

void VolumeTextureExample::Display(bool auto_redraw)
{
    float t = float(app_time()) / float(0x3FFF);
    static const vmath::vec3 X(1.0f, 0.0f, 0.0f);
    static const vmath::vec3 Y(0.0f, 1.0f, 0.0f);
    static const vmath::vec3 Z(0.0f, 0.0f, 1.0f);

    vmath::mat4 tc_matrix(vmath::mat4::identity());

    glClearColor(0.6f, 0.7f, 0.9f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glUseProgram(base_prog);

    tc_matrix = vmath::rotate(t * 170.0f, X) *
                vmath::rotate(t * 137.0f, Y) *
                vmath::rotate(t * 93.0f, Z);

    glUniformMatrix4fv(tc_rotate_loc, 1, GL_FALSE, tc_matrix);

    glBindVertexArray(vao);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisable(GL_BLEND);

    base::Display();
}

void VolumeTextureExample::Finalize(void)
{
    glUseProgram(0);
    glDeleteProgram(base_prog);
    glDeleteTextures(1, &tex);
    glDeleteVertexArrays(1, &tex);
}

void VolumeTextureExample::Resize(int width, int height)
{
    glViewport(0, 0 , width, height);

    aspect = float(height) / float(width);
}
