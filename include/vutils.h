#ifndef __VUTILS_H__
#define __VUTILS_H__

#include "vgl.h"
#include "LoadShaders.h"
#include <iostream>

void vglAttachShaderSource(GLuint prog, GLenum type, const char * source)
{
    GLuint sh;

    sh = glCreateShader(type);
    glShaderSource(sh, 1, &source, NULL);
    glCompileShader(sh);
    char buffer[4096];
    glGetShaderInfoLog(sh, sizeof(buffer), NULL, buffer);
    glAttachShader(prog, sh);
    glDeleteShader(sh);
}
GLboolean vglAttachShaderSourceFromFile(GLuint prog, GLenum type, const char *source_file )
{
    GLboolean status=true;
    GLuint shader=0;
    const GLchar* source = NULL;
    
    source=ReadShader( source_file );

    if ( source == NULL ) 
    {
        status=false;
    }  
    else
    {
        shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        GLint compile_status;
        glGetShaderiv( shader, GL_COMPILE_STATUS, &compile_status );
        if ( !compile_status ) {
#ifdef _DEBUG
            GLsizei len;
            glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &len );

            GLchar *log = new GLchar[len+1];
            glGetShaderInfoLog( shader, len, &len, log );
            std::cerr << "Shader compilation failed: " << log << std::endl;
            delete [] log;
#endif 
            status= false;;
        }
        else
        {
            glAttachShader(prog, shader);
        }
    }
    if(shader) glDeleteShader( shader );
    if(source) delete [] source; 

   return status;
}
#endif /* __VUTILS_H__ */
