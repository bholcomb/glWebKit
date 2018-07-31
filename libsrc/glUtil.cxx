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

#pragma endregion