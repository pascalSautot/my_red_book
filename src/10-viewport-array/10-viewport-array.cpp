/* $URL$
   $Rev$
   $Author$
   $Date$
   $Id$
 */

#include "vapp.h"
#include "vutils.h"
#include "vbm.h"

#include "vmath.h"

#include <stdio.h>
#include <iostream>

#include "LoadShaders.h"

BEGIN_APP_DECLARATION(ViewportArrayApplication)
    // Override functions from base class
    virtual void Initialize(const char * title);
    virtual void Display(bool auto_redraw);
    virtual void Finalize(void);
    virtual void Resize(int width, int height);

    // Member variables
    float aspect;
    GLuint prog;
    GLuint vao;
    GLuint vbo;
    VBObject object;

    GLint model_matrix_loc;
    GLint projection_matrix_loc;
END_APP_DECLARATION()

DEFINE_APP(ViewportArrayApplication, "Viewport Array")

void ViewportArrayApplication::Initialize(const char * title)
{
    base::Initialize(title);

    // load shaders in program
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/viewport-array/mutipleviewport.vert" },
        { GL_GEOMETRY_SHADER, "media/shaders/viewport-array/mutipleviewport.glsl" },
        { GL_FRAGMENT_SHADER, "media/shaders/viewport-array/mutipleviewport.frag" },
        { GL_NONE, NULL }
    };

    prog = LoadShaders( shaders );
    if( prog == 0) { std::cerr<<"shader program failed to build" << std::endl; }
    glUseProgram(prog);

    model_matrix_loc = glGetUniformLocation(prog, "model_matrix");
    projection_matrix_loc = glGetUniformLocation(prog, "projection_matrix");

    object.LoadFromVBM("media/ninja.vbm", 0, 1, 2);
}

void ViewportArrayApplication::Display(bool auto_redraw)
{
    float t = float(app_time() & 0x3FFF) / float(0x3FFF);
    static const vmath::vec3 X(1.0f, 0.0f, 0.0f);
    static const vmath::vec3 Y(0.0f, 1.0f, 0.0f);
    static const vmath::vec3 Z(0.0f, 0.0f, 1.0f);

    glClearColor( 0.3f, 0.1f, 0.2f, 1.0f );
    glClearDepth( 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram(prog);

    vmath::mat4 p(vmath::frustum(-1.0f, 1.0f, aspect, -aspect, 1.0f, 5000.0f));
    vmath::mat4 m[4];

    for (int i = 0; i < 4; i++)
    {
        m[i] = vmath::mat4(vmath::translate(0.0f, 0.0f, 100.0f * sinf(6.28318531f * t + i) - 230.0f) *
                           vmath::rotate(360.0f * t * float(i + 1), X) *
                           vmath::rotate(360.0f * t * float(i + 2), Y) *
                           vmath::rotate(360.0f * t * float(5 - i), Z) *
                           vmath::translate(0.0f, -80.0f, 0.0f));
    }

    glUniformMatrix4fv(model_matrix_loc, 4, GL_FALSE, m[0]);
    glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, p);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    object.Render();

    base::Display();
}

void ViewportArrayApplication::Finalize(void)
{
    glUseProgram(0);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);
    glDeleteVertexArrays(1, &vao);
}

void ViewportArrayApplication::Resize(int width, int height)
{
    const float wot = float(width) * 0.5f;
    const float hot = float(height) * 0.5f;

    glViewportIndexedf(0, 0.0f, 0.0f, wot, hot);
    glViewportIndexedf(1, wot, 0.0f, wot, hot);
    glViewportIndexedf(2, 0.0f, hot, wot, hot);
    glViewportIndexedf(3, wot, hot, wot, hot);

    aspect = hot / wot;
}
