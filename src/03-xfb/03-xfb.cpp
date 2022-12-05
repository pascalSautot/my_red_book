/* $URL$
   $Rev$
   $Author$
   $Date$
   $Id$
 */

#include "vapp.h"
#include "vutils.h"

#include "vmath.h"

#include "vbm.h"

#include <stdio.h>
#include <iostream>

BEGIN_APP_DECLARATION(TransformFeedbackExample)
    // Override functions from base class
    virtual void Initialize(const char * title);
    virtual void Display(bool auto_redraw);
    virtual void Finalize(void);
    virtual void Resize(int width, int height);

    typedef struct  {
        vmath::vec4 position;
        vmath::vec3 velocity;
    } buffer_t;

    // Member variables
    float aspect;

    GLuint collision_prog;
    GLuint particule_vao[2];
    GLuint particule_vbo[2];
    GLint particle_model_matrix_loc;
    GLint particle_projection_matrix_loc;
    GLint particle_triangle_count_loc;
    GLint particle_time_step_loc;

    GLuint armadillo_prog;
    GLuint armadillo_geometry_tbo;
    GLuint armadillo_vao;
    GLint armadillo_model_matrix_loc;
    GLint armadillo_projection_matrix_loc;
    GLuint armadillo_geometry_tex;

    VBObject armadillo_mesh;
protected:
    GLboolean InitProg(GLuint &prog, ShaderInfo* shaders, const GLsizei count_v, const char * varyings_variables[], GLboolean use_varyings=false);
    GLboolean InitArmadilloProg(void);
    GLboolean InitParticleCollisionProg(void);   
    GLboolean CheckProg(GLuint prog, ShaderInfo *shaders);
    void BindParticleCollision(void);
    void BindDataArmadillo(void);
    void ParticleCollisionDisplay(bool auto_redraw, float t, vmath::mat4 &model_matrix, vmath::mat4 &projection_matrix);
    void ArmadilloDisplay(bool auto_redraw, float t, vmath::mat4 &model_matrix, vmath::mat4 &projection_matrix);

END_APP_DECLARATION()

DEFINE_APP(TransformFeedbackExample, "TransformFeedback Example")

const int point_count = 5000;
static unsigned int seed = 0x13371337;

static inline float random_float()
{
    float res;
    unsigned int tmp;

    seed *= 16807;

    tmp = seed ^ (seed >> 4) ^ (seed << 15);

    *((unsigned int *) &res) = (tmp >> 9) | 0x3F800000;

    return (res - 1.0f);
}

static vmath::vec3 random_vector(float minmag = 0.0f, float maxmag = 1.0f)
{
    vmath::vec3 randomvec(random_float() * 2.0f - 1.0f, random_float() * 2.0f - 1.0f, random_float() * 2.0f - 1.0f);
    randomvec = normalize(randomvec);
    randomvec *= (random_float() * (maxmag - minmag) + minmag);

    return randomvec;
}

GLboolean TransformFeedbackExample::CheckProg(GLuint prog, ShaderInfo *shaders)
{
    GLboolean status=true;
    GLint linked;
    glGetProgramiv( prog, GL_LINK_STATUS, &linked );
    if ( !linked ) {
        status=false;
#ifdef _DEBUG
        GLsizei len;
        glGetProgramiv( prog, GL_INFO_LOG_LENGTH, &len );

        GLchar* log = new GLchar[len+1];
        glGetProgramInfoLog( prog, len, &len, log );
        std::cerr << "Shader linking failed: " << log << std::endl;
        delete [] log;
#endif /* DEBUG */
        ShaderInfo *entry=shaders;
        for ( entry = shaders; entry->type != GL_NONE; ++entry ) {
            glDeleteShader( entry->shader );
            entry->shader = 0;
        }
    }
    return(status);
}
#undef USE_VARSYINGS        

GLboolean TransformFeedbackExample::InitProg(GLuint &prog, ShaderInfo* shaders, const GLsizei count_v, const char *variables[], GLboolean use_varyings)
{
    GLboolean status=true;
    // render 
    prog = glCreateProgram();

    ShaderInfo *entry= shaders;
    // render loop
    while ( entry->type != GL_NONE ) {
        status = status && vglAttachShaderSourceFromFile(prog, entry->type, entry->filename);
        ++entry;
    }  
    if(status)
    {
        if(use_varyings)      
            glTransformFeedbackVaryings(prog, count_v, variables, GL_INTERLEAVED_ATTRIBS);
        glLinkProgram(prog);
        glUseProgram(prog);
        status=CheckProg(prog,shaders);
    }
    return status;
}

GLboolean TransformFeedbackExample::InitParticleCollisionProg()
{
    GLboolean status=true;
#ifdef USE_VARSYINGS        
    GLboolean use_varyings=true;     
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/xfb/particle-collision.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/xfb/particle.frag" },
        { GL_NONE, NULL }
    };
#else
    GLboolean use_varyings=false;     
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/xfb/particle-collision-xfb.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/xfb/particle.frag" },
        { GL_NONE, NULL }
    };
#endif    
    const GLsizei count_v=2;
    const char * varying_variables[count_v] =
    {
        "position_out", "velocity_out"
    };    
    status=InitProg(collision_prog, shaders,count_v,varying_variables,use_varyings);
    
    if(status)
    {
        particle_model_matrix_loc = glGetUniformLocation(collision_prog, "model_matrix");
        particle_projection_matrix_loc = glGetUniformLocation(collision_prog, "projection_matrix");
        particle_triangle_count_loc = glGetUniformLocation(collision_prog, "triangle_count");
        particle_time_step_loc = glGetUniformLocation(collision_prog, "time_step");

        if(particle_model_matrix_loc == -1){
            std::cout << "scale model_matrix not fetched in shader" << std::endl;
            status=false;
        }           
        if(particle_projection_matrix_loc == -1){
            std::cout << "scale projection_matrix not fetched in shader" << std::endl;
            status=false;
        }                    
        if(particle_triangle_count_loc == -1){
            std::cout << "scale triangle_count not fetched in shader" << std::endl;
            status=false;
        }                    
        if(particle_time_step_loc == -1){
            std::cout << "scale time_step not fetched in shader" << std::endl;
            status=false;
        }                     
    }
    return(status);
}

GLboolean TransformFeedbackExample::InitArmadilloProg()
{
    GLboolean status=true;
    // render 
    armadillo_prog = glCreateProgram();
#ifdef USE_VARSYINGS   
    GLboolean use_varyings=true;     
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/xfb/render.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/xfb/blue.frag" },
        { GL_NONE, NULL }
    };
#else
    GLboolean use_varyings=false;     
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "media/shaders/xfb/render-xfb.vert" },
        { GL_FRAGMENT_SHADER, "media/shaders/xfb/blue.frag" },
        { GL_NONE, NULL }
    };
#endif
    const GLsizei count_v=1;
    const char * varying_variables[count_v] =
    {
        "world_space_position"
    };   
    status=InitProg(armadillo_prog, shaders,count_v,varying_variables,use_varyings);    

    if(status)
    {
        armadillo_model_matrix_loc = glGetUniformLocation(armadillo_prog, "model_matrix");
        armadillo_projection_matrix_loc = glGetUniformLocation(armadillo_prog, "projection_matrix");

        if(armadillo_model_matrix_loc == -1){
            std::cout << "scale model_matrix not fetched in shader" << std::endl;
            status=false;
        }  
        if(armadillo_projection_matrix_loc == -1){
            std::cout << "scale projection_matrix not fetched in shader" << std::endl;
            status=false;
        }                           
    }
    return(status);
}
void TransformFeedbackExample::Initialize(const char * title)
{
    base::Initialize(title);

    GLboolean status=InitParticleCollisionProg();
    status = status && InitArmadilloProg();
    if(!status) 
    {
        Finalize();
        exit(0);
    }

    BindParticleCollision();
    BindDataArmadillo();
}
void TransformFeedbackExample::BindParticleCollision(void)
{
    enum { positionId=0, velocityId=1};

    glGenVertexArrays(2, particule_vao);
    glGenBuffers(2, particule_vbo); // particules' position and velocity  

    const GLsizei buffer_t_size=sizeof(buffer_t); 
    const GLsizei position_size =4; // vec4
    const GLsizei velocity_size =3; // vec3
    const GLboolean normalize = false;
    const GLsizei buffer_t_stride=buffer_t_size;
    const GLint att_type=GL_FLOAT;

    const GLsizeiptr points_size= point_count * (buffer_t_size);
    const vmath::vec3 particule_flow_direction=vmath::vec3(-0.5f, 40.0f, 0.0f);
    const GLuint position_offset=sizeof(vmath::vec4);
    const int vbo_count= sizeof(particule_vbo)/sizeof(particule_vbo[0]);
    GLboolean set_buffer_data=true;

    for (int i = 0; i < vbo_count; i++)
    {
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, particule_vbo[i]);
        glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, points_size, NULL, GL_DYNAMIC_COPY);
        if (set_buffer_data)
        {
            buffer_t * buffer = (buffer_t *)glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_WRITE_ONLY);
            for (int j = 0; j < point_count; j++)
            {
                buffer[j].velocity = random_vector();
                buffer[j].position = vmath::vec4(buffer[j].velocity + particule_flow_direction, 1.0f);
                buffer[j].velocity = vmath::vec3(buffer[j].velocity[0], buffer[j].velocity[1] * 0.3f, buffer[j].velocity[2] * 0.3f);
            }
            glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
            set_buffer_data=false;
        }

        //match located data for position and velocity in shader article-collision-xfb/particle-collision
        glBindVertexArray(particule_vao[i]);
        glBindBuffer(GL_ARRAY_BUFFER, particule_vbo[i]);
        glVertexAttribPointer(positionId, position_size, att_type, normalize, buffer_t_stride, NULL); // position one set each buffer_t
        glVertexAttribPointer(velocityId, velocity_size, att_type, normalize, buffer_t_stride, BUFFER_OFFSET(position_offset)); // velocity one set each buffer_t + offset for position (vec4)
        glEnableVertexAttribArray(positionId);
        glEnableVertexAttribArray(velocityId);
    }
}
void TransformFeedbackExample::BindDataArmadillo(void)
{
    enum { positionId=0};
    const GLsizei vertex_coord_size =4; // vec4
    const GLboolean normalize = false;
    const GLint att_type=GL_FLOAT;
    
    // load armadillo mesh from VBM file
    armadillo_mesh.LoadFromVBM("media/armadillo_low.vbm", 0, 1, 2);
    const GLint vertex_count(armadillo_mesh.GetVertexCount());

    // define a buffer to pass on mesh vertices from the vertex shader
    glGenBuffers(1, &armadillo_geometry_tbo);
    // use a texture buffer as a generic purpose data storage (se Ch11 Kessenich)
    glBindBuffer(GL_TEXTURE_BUFFER, armadillo_geometry_tbo);
    // allocate storage for texture buffer enough to fit all vertices
    glBufferData(GL_TEXTURE_BUFFER, vertex_count * sizeof(vmath::vec4), NULL, GL_DYNAMIC_COPY);
    
    // create a texure object (name saved in armadillo_geometry_tex)
    glGenTextures(1, &armadillo_geometry_tex); 
    //bind texture object to the texture buffer object (TBO)
    glBindTexture(GL_TEXTURE_BUFFER, armadillo_geometry_tex);
    
    //attach a buffer object's data store to a buffer texture object
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, armadillo_geometry_tbo); // GL_RGBA32F => storage format is 4 x float to fit a vec4

    glGenVertexArrays(1, &armadillo_vao);
    glBindVertexArray(armadillo_vao);
    glBindBuffer(GL_ARRAY_BUFFER, armadillo_geometry_tbo);
    glVertexAttribPointer(positionId, vertex_coord_size, att_type, normalize, 0, NULL); //  position in render-xfb.vert / render.vert shader
    glEnableVertexAttribArray(positionId);
}

void TransformFeedbackExample::ArmadilloDisplay(bool auto_redraw, float t,vmath::mat4 &model_matrix, vmath::mat4 &projection_matrix)
{
    glUseProgram(armadillo_prog);
    glUniformMatrix4fv(armadillo_model_matrix_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(armadillo_projection_matrix_loc, 1, GL_FALSE, projection_matrix);

    glBindVertexArray(armadillo_vao);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, armadillo_geometry_tbo);

    // get transformed vertex coord from render.vert/render-xfb.vert 
    glBeginTransformFeedback(GL_TRIANGLES);
    armadillo_mesh.Render(); // drawn primitves must be GL_TRIANGLES
    glEndTransformFeedback();

    glBindVertexArray(0);
}

void TransformFeedbackExample::ParticleCollisionDisplay(bool auto_redraw, float tick, vmath::mat4 &model_matrix, vmath::mat4 &projection_matrix)
{
    static int frame_count = 0;
    static float prev_tick = 0.0f;

    glUseProgram(collision_prog);
    glUniformMatrix4fv(particle_model_matrix_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(particle_projection_matrix_loc, 1, GL_FALSE, projection_matrix);

    const GLint triangle_count(armadillo_mesh.GetVertexCount() / 3);
    glUniform1i(particle_triangle_count_loc, triangle_count);

    const float time_step(tick - prev_tick);
    if (time_step > 0)
    {
        glUniform1f(particle_time_step_loc, time_step * 2000.0f);
    }
    prev_tick = tick;

    //manage double buffer one as input the other to output updated position and velocity via the feedback transform
    const int vao_index= frame_count%2;
    const int vbo_index( vao_index==1 ? 0 : 1);

    // use buffer to get position and velocity
    glBindVertexArray(particule_vao[vao_index]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particule_vbo[vbo_index]); 

    glBeginTransformFeedback(GL_POINTS); // transfer particules' position and velocity

    //display as many points as frames displayed but no more than point_count
    const int c(frame_count >> 3);
    const int count( std::min(point_count, c));
    glDrawArrays(GL_POINTS, 0, count);

    glEndTransformFeedback();

    glBindVertexArray(0);

    frame_count++;

}
void TransformFeedbackExample::Display(bool auto_redraw)
{
    float tick = float(app_time() & 0x3FFFF) / float(0x3FFFF);

    vmath::mat4 projection_matrix(vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 5000.0f) * vmath::translate(0.0f, 0.0f, -100.0f));

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    vmath::mat4 model_matrix(vmath::scale(0.3f) *
                             vmath::rotate(tick * 360.0f, 0.0f, 1.0f, 0.0f) *
                             vmath::rotate(tick * 360.0f * 3.0f, 0.0f, 0.0f, 1.0f));

    //display armadillo and get the geometry via transform feedback
    ArmadilloDisplay(auto_redraw,tick,model_matrix, projection_matrix);

    //display particles and compute collision with armadillo to splash around
    model_matrix = vmath::mat4::identity();
    ParticleCollisionDisplay(auto_redraw,tick,model_matrix, projection_matrix);

    base::Display();
}

void TransformFeedbackExample::Finalize(void)
{
    glUseProgram(0);
    glDeleteProgram(collision_prog);
    glDeleteVertexArrays(2, particule_vao);
    glDeleteBuffers(2, particule_vbo);
}

void TransformFeedbackExample::Resize(int width, int height)
{
    glViewport(0, 0 , width, height);

    aspect = float(height) / float(width);
}
