/* $URL$
   $Rev$
   $Author$
   $Date$
   $Id$
 */

// #define USE_GL3W
#include <vermilion.h>

#include "vapp.h"
#include "vutils.h"
#include "vbm.h"

#include "vmath.h"

#include <stdio.h>
#include <iostream>
#include "LoadShaders.h"

BEGIN_APP_DECLARATION(ImageProcessingComputeExample)
    // Override functions from base class
    virtual void Initialize(const char * title);
    virtual void Display(bool auto_redraw);
    virtual void Finalize(void);
    virtual void Resize(int width, int height);

    // Member variables
    GLuint  compute_prog;
    GLuint  compute_shader;

    // Texture to process
    GLuint  input_image;

    // Texture for compute shader to write into
    GLuint  intermediate_image;
    GLuint  output_image;

    // Program, vao and vbo to render a full screen quad
    GLuint  render_prog;
    GLuint  render_vao;
    GLuint  render_vbo;
protected:
    bool BuildShaders(ShaderInfo  *shaders, GLuint &prog);
    bool InitializeCompute();
    bool InitializeRender();
END_APP_DECLARATION()

DEFINE_APP(ImageProcessingComputeExample, "Compute Shader Image Processing Example")

bool ImageProcessingComputeExample::BuildShaders(ShaderInfo  *shaders, GLuint &prog)
{
    bool status(false);
    prog = LoadShaders( shaders);
    status=( prog != 0);
    if( !status) 
    { 
        std::cerr<<"shader program failed to build" << std::endl; 
    }
    else
    {
        glUseProgram( prog );    
    }
    return status;
}

bool ImageProcessingComputeExample::InitializeCompute()
{
    ShaderInfo  compute_shaders[] =
    {
        { GL_COMPUTE_SHADER, "media/shaders/image-processing/image-processing.comp.glsl" },
        { GL_NONE, NULL }
    };
    bool status=BuildShaders(compute_shaders,compute_prog);
    if(status) 
    {
        // Load a texture to process
        input_image = vglLoadTexture("media/curiosity.dds", 0, NULL);

        glGenTextures(1, &intermediate_image);
        glBindTexture(GL_TEXTURE_2D, intermediate_image);
        glTexStorage2D(GL_TEXTURE_2D, 8, GL_RGBA32F, 1024, 1024);

        // This is the texture that the compute program will write into
        glGenTextures(1, &output_image);
        glBindTexture(GL_TEXTURE_2D, output_image);
        glTexStorage2D(GL_TEXTURE_2D, 8, GL_RGBA32F, 1024, 1024);
    }
    return status;
}
bool ImageProcessingComputeExample::InitializeRender()
{
    ShaderInfo  render_shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/image-processing/image-processing.vert.glsl" },
        { GL_FRAGMENT_SHADER, "media/shaders/image-processing/image-processing.frag.glsl" },
        { GL_NONE, NULL }
    };
    bool status=BuildShaders(render_shaders,render_prog);
    if(status)
    {
        // This is the VAO containing the data to draw the quad (including its associated VBO)
        glGenVertexArrays(1, &render_vao);
        glBindVertexArray(render_vao);
        glEnableVertexAttribArray(0);
        glGenBuffers(1, &render_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, render_vbo);
        static const float verts[] =
        {
            -1.0f, -1.0f, 0.5f, 1.0f,
            1.0f, -1.0f, 0.5f, 1.0f,
            1.0f,  1.0f, 0.5f, 1.0f,
            -1.0f,  1.0f, 0.5f, 1.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);        
    }
    return status;

}
void ImageProcessingComputeExample::Initialize(const char * title)
{
    base::Initialize(title);

    if( !InitializeCompute()) exit(0);
    if( !InitializeRender()) exit(0);

}

void ImageProcessingComputeExample::Display(bool auto_redraw)
{
    // Activate the compute program and bind the output texture image
    glUseProgram(compute_prog);
    glBindImageTexture(0, input_image, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, intermediate_image, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(1, 1024, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindImageTexture(0, intermediate_image, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, output_image, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(1, 1024, 1);

    // Now bind the texture for rendering _from_
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, output_image);

    // Clear, select the rendering program and draw a full screen quad
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(render_prog);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    base::Display();
}

void ImageProcessingComputeExample::Finalize(void)
{
    glUseProgram(0);
    if(compute_prog)glDeleteProgram(compute_prog);
    if(render_prog)glDeleteProgram(render_prog);
    if(output_image)glDeleteTextures(1, &output_image);
    if(render_vao)glDeleteVertexArrays(1, &render_vao);
}

void ImageProcessingComputeExample::Resize(int width, int height)
{
    glViewport(0, 0, width, height);
}
