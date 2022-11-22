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
#include <iostream>

#include "LoadShaders.h"

BEGIN_APP_DECLARATION(LoadTextureExample)
    // Override functions from base class
    virtual void Initialize(const char * title);
    virtual void Display(bool auto_redraw);
    virtual void Finalize(void);
    virtual void Reshape(int width, int height);

    // Member variables
    float aspect;
    GLuint base_prog;
    GLuint vao;

    GLuint quad_vbo;

    GLuint tex;
END_APP_DECLARATION()

DEFINE_APP(LoadTextureExample, "Simple LoadTexture Example")

void LoadTextureExample::Initialize(const char * title)
{
    base::Initialize(title);


    // load shaders in program
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/textures/quad.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/textures/quad.frag" },
        { GL_NONE, NULL }
    };

    base_prog = LoadShaders( shaders );
    if( base_prog == 0) { std::cerr<<"shader program failed to build" << std::endl; }
    glUseProgram( base_prog );

    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

    static const GLfloat quad_data[] =
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

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(8 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glLinkProgram(base_prog);

    char buf[1024];
    glGetProgramInfoLog(base_prog, 1024, NULL, buf);

    vglImageData image;

    tex = vglLoadTexture("media/test.dds", 0, &image);

    glTexParameteri(image.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    vglUnloadImage(&image);
}

void LoadTextureExample::Display(bool auto_redraw)
{
    float t = float(app_time() & 0x3FFF) / float(0x3FFF);
    static const vmath::vec3 X(1.0f, 0.0f, 0.0f);
    static const vmath::vec3 Y(0.0f, 1.0f, 0.0f);
    static const vmath::vec3 Z(0.0f, 0.0f, 1.0f);

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glUseProgram(base_prog);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    base::Display();
}

void LoadTextureExample::Finalize(void)
{
    glUseProgram(0);
    glDeleteProgram(base_prog);
    glDeleteTextures(1, &tex);
    glDeleteVertexArrays(1, &tex);
}

void LoadTextureExample::Reshape(int width, int height)
{
    glViewport(0, 0 , width, height);

    aspect = float(height) / float(width);
}
