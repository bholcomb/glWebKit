#include "glUtil.h"

#include <GL/glew.h>
#include <gl/GLU.h>

#include <iostream>

#pragma region helpers
void checkOpenGLError(const std::string& name)
{
   GLenum err=glGetError();
   if(err)
   {
      std::cout << "GL error" << std::endl;
      //std::cout << name << " error: " << gluErrorString(err) << std::endl;
   }
}

#if 0
#define CHECK_GL_ERROR(x) checkOpenGLError(x)

#else
#define CHECK_GL_ERROR(x)
#endif


class ScopedEnableMode
{
public: 
   ScopedEnableMode(int mode, bool enable)
      :myMode(mode)
   {
      //get previous value
      int val;
      glGetIntegerv(myMode, &val);
      CHECK_GL_ERROR("glGetIntegerv error");
      myValue = val != 0;

      //set new value
      enable ? glEnable(myMode) : glDisable(myMode);
      CHECK_GL_ERROR("enable/disable error");
   }

   ~ScopedEnableMode()
   {
      //restore value
      myValue ? glEnable(myMode) : glDisable(myMode);
      CHECK_GL_ERROR("enable/disable error");
   }

protected:
   bool myValue;
   int myMode;
};

void printShaderInfoLog(GLuint obj)
{
   int infologLength = 0;
   int charsWritten = 0;
   char *infoLog;

   glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

   if(infologLength > 0)
   {
      infoLog = (char *)malloc(infologLength);
      glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
      printf("%s\n", infoLog);
      free(infoLog);
   }
}

void printProgramInfoLog(GLuint obj)
{
   int infologLength = 0;
   int charsWritten = 0;
   char *infoLog;

   glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

   if(infologLength > 0)
   {
      infoLog = (char *)malloc(infologLength);
      glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
      printf("%s\n", infoLog);
      free(infoLog);
   }
}

#pragma endregion

#pragma region shaders
const char* webkitVertexShader =
"#version 330\n"
"\n"
"layout(location = 0) in vec3 position;\n"
"layout(location = 1) in vec2 uv;\n"
"\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"\n"
"smooth out vec2 texCoord;\n"
"\n"
"void main()\n"
"{\n"
"   texCoord = uv;\n"
"   gl_Position = projection * view * vec4(position, 1);\n"
"}\n";

const char* webkitFragmentShader =
"#version 330\n"
"\n"
"uniform sampler2D tex;\n"
"\n"
"smooth in vec2 texCoord;\n"
"\n"
"out vec4 FragColor;\n"
"\n"
"void main()\n"
"{\n"
"   //flip the y value since we're getting the data upside down from webkit \n"
"   vec2 uv = vec2(texCoord.x, 1.0f - texCoord.y);\n"
"   vec4 outColor = texture(tex, uv);\n"
"   if(outColor.a < 0.1) \n"
"      discard;\n"
"\n"
"   FragColor = outColor;"
"}\n";

const char* cubeVertexShader =
"#version 330\n"
"\n"
"layout(location = 0) in vec3 position;\n"
"layout(location = 1) in vec4 color;\n"
"\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"uniform mat4 model;\n"
"\n"
"smooth out vec4 vColor;\n"
"\n"
"void main()\n"
"{\n"
"   vColor = color;\n"
"   gl_Position = projection * view * model * vec4(position, 1);\n"
"}\n";

const char* cubeFragmentShader =
"#version 330\n"
"\n"
"smooth in vec4 vColor;\n"
"\n"
"out vec4 FragColor;\n"
"\n"
"void main()\n"
"{\n"
"   FragColor = vColor;"
"}\n";
#pragma endregion

unsigned int createShader(const char* vert, const char* frag)
{
   unsigned int vs;
   unsigned int ps;
   unsigned int prog;

   vs = glCreateShader(GL_VERTEX_SHADER);
   ps = glCreateShader(GL_FRAGMENT_SHADER);
   prog = glCreateProgram();

   //create the shaders
   int len = strlen(vert);
   glShaderSource(vs, 1, (const char**)&vert, &len);
   glCompileShader(vs);
   printShaderInfoLog(vs);

   len = strlen(frag);
   glShaderSource(ps, 1, (const char**)&frag, &len);
   glCompileShader(ps);
   printShaderInfoLog(ps);

   //link to a program
   glAttachShader(prog, vs);
   glAttachShader(prog, ps);
   glLinkProgram(prog);
   printProgramInfoLog(prog);

   return prog;
}