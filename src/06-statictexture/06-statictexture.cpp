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

BEGIN_APP_DECLARATION(LoadTextureExample)
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
    enum AttribIndex { VertexCoordID, TextureCoordID, MaxAttrib};
    enum dataSize { vertexCoordCount =4, textureCoordCount=4};

protected:
    void InitializeShaders(void);

END_APP_DECLARATION()

DEFINE_APP(LoadTextureExample, "Simple Static Texture Example")

void LoadTextureExample::InitializeShaders(void)
{
    base_prog = glCreateProgram();
    vglAttachShaderSourceFromFile(base_prog, GL_VERTEX_SHADER,      "media/shaders/static-texture/quad.vert");
    vglAttachShaderSourceFromFile(base_prog, GL_FRAGMENT_SHADER,    "media/shaders/static-texture/quad.frag");
    glLinkProgram(base_prog);
}
void LoadTextureExample::Initialize(const char * title)
{
    base::Initialize(title);

    InitializeShaders();

    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    static const GLfloat quad_data[2*(vertexCoordCount+textureCoordCount)] =
    {
         0.75f, -0.75f,
        -0.75f, -0.75f,
        -0.75f, 0.75f,
         0.75f, 0.75f,

         1.0f, 1.0f,
         1.0f, 0.0f, 
         0.0f, 0.0f,   //going out of +/-[0;1] interval => extrapolation
         0.0f, 1.0f

    };
/*
         0.0f, 0.0f,
        -0.75f, 0.75f,
         0.75f, 0.75f,

         0.5f, 0.0f,
         0.0f, 1.5f,
         1.5f, 1.5f,

         0.75f, -0.75f,
        -0.75f, -0.75f,
        -0.75f, 0.75f,
         0.75f, 0.75f,

         0.0f, 0.0f,   //going out of +/-[0;1] interval => extrapolation
         0.5f, 0.0f,
         0.5f, 0.5f,
         0.0f, 0.5f

         0.0f, 0.0f,
         1.0f, 0.0f,
         1.0f, 1.0f,
         0.0f, 1.0f
*/
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //8 first float in quad_data
    const GLint attribSize=2; // vertex coord completed with static const for Z and w in vertex shader
    const GLboolean normalize= GL_FALSE;
    const GLsizei stride=0;
    GLuint offset=0;
    const GLint att_type=GL_FLOAT;
    glVertexAttribPointer(VertexCoordID, attribSize, att_type, normalize, stride, BUFFER_OFFSET(offset));
    //8 and last following float in quad_data
    offset=8*sizeof(GLfloat);
    glVertexAttribPointer(TextureCoordID, attribSize, att_type, normalize, stride, BUFFER_OFFSET(offset));

    glEnableVertexAttribArray(VertexCoordID);
    glEnableVertexAttribArray(TextureCoordID);


    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    const GLsizei levelCount=1;
    const GLsizei texWidth=8,texHeigth=8;

    const GLint internal_format=GL_RGBA8;
    //const GLint internal_format=GL_RGBA16; 
    glTexStorage2D(GL_TEXTURE_2D, levelCount, internal_format, texWidth, texHeigth);

    // texture_data is a 1D array of texWidth x texHeigth values 
    static const unsigned char texture_data[texWidth*texHeigth] =
        {
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF
    };    
    /*
    static const GLuint texture_data[texWidth*texHeigth] =
    {
        0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000,
        0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
        0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000,
        0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
        0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000,
        0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF,
        0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000,
        0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF
    };
    */
    /*
    const GLsizei texWidth=4,texHeigth=4;

    {
        0xFF, 0x00, 0xFF, 0x00, 
        0x00, 0xFF, 0x00, 0xFF, 
        0xFF, 0x00, 0xFF, 0x00, 
        0x00, 0xFF, 0x00, 0xFF
    };
    const GLsizei texWidth=8,texHeigth=8;
    
    {
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF
    };    */
    const GLint textureLevel=0, 
                x_offset=0,y_offset=0,
                external_format=GL_GREEN,
                //external_format=GL_RED_INTEGER,
                data_type=GL_UNSIGNED_BYTE;
                //data_type=GL_UNSIGNED_INT;
    glTexSubImage2D(GL_TEXTURE_2D,
                    textureLevel,
                    x_offset, y_offset,texWidth, texHeigth,
                    external_format, data_type,texture_data);

    static const GLint swizzles[] = { GL_GREEN, GL_ZERO, GL_ZERO, GL_ONE };//swizzle display red from green external values
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzles);//use grey image with value defined in R component
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //method to minify
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //method to magnify
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//GL_CLAMP_TO_EDGE
    float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 
    glGenerateMipmap(GL_TEXTURE_2D);
}

void LoadTextureExample::Display(bool auto_redraw)
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glUseProgram(base_prog);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCoordCount);
    //glDrawArrays(GL_TRIANGLES, 0, vertexCoordCount);

    base::Display();
}

void LoadTextureExample::Finalize(void)
{
    glUseProgram(0);
    glDeleteProgram(base_prog);
    glDeleteTextures(1, &tex);
    glDeleteVertexArrays(1, &tex);
}

void LoadTextureExample::Resize(int width, int height)
{
    glViewport(0, 0 , width, height);

    aspect = float(height) / float(width);
}
