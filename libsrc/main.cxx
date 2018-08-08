#include "windows.h"

#define SDL_MAIN_HANDLED
#include "SDL/SDL.h"

#include <GL/glew.h>

#include "glm/glm.hpp"
#include "glm/matrix.hpp"
#include "glm/trigonometric.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "glWebKit.h"
#include "glWebkitUtils.h"

#include "glUtil.h"

#include <vector>
#include <iostream>
#include <sstream>

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

EA::WebKit::View* v;

//OpenGL context
SDL_GLContext gContext;
DrawCall screenQuad;
DrawCall cube;

void initGL()
{
   GLenum err = glewInit();
   if(GLEW_OK != err)
   {
      std::cout << "Error initializing GLEW" << std::endl;
      exit(0);
   }

   glEnable(GL_BLEND);
   glBlendEquation(GL_FUNC_ADD);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glEnable(GL_CULL_FACE);
}

void initScreenQuad()
{
   std::vector<DrawVertex> verts;
   std::vector<unsigned short> indexes;
   
   verts.resize(4);
   indexes.resize(6);
   
   verts[0].position = Vector3(0, 0, 0);                        verts[0].uv = Vector2(0, 0);
   verts[1].position = Vector3(SCREEN_WIDTH, 0, 0);             verts[1].uv = Vector2(1, 0);
   verts[2].position = Vector3(SCREEN_WIDTH, SCREEN_HEIGHT, 0); verts[2].uv = Vector2(1, 1);
   verts[3].position = Vector3(0, SCREEN_HEIGHT, 0);            verts[3].uv = Vector2(0, 1);

   indexes[0] = 0; indexes[1] = 1; indexes[2] = 2;
   indexes[3] = 0; indexes[4] = 2; indexes[5] = 3;

   screenQuad.indexCount = indexes.size();

   glGenBuffers(1, &screenQuad.myVertexBuffer);
   glGenBuffers(1, &screenQuad.myIndexBuffer);
   glGenVertexArrays(1, &screenQuad.myVao);
   
   screenQuad.myProgram = createWebkitShader();
   
   //get uniform locations
   screenQuad.myViewUniform = glGetUniformLocation(screenQuad.myProgram, "view");
   screenQuad.myProjectionUniform = glGetUniformLocation(screenQuad.myProgram, "projection");
   screenQuad.myTextureUniform = glGetUniformLocation(screenQuad.myProgram, "tex");

   //setup the buffers
   glBindBuffer(GL_ARRAY_BUFFER, screenQuad.myVertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(DrawVertex), &verts[0], GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuad.myIndexBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(unsigned short), &indexes[0], GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   //Setup the VAO
   glBindVertexArray(screenQuad.myVao);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuad.myIndexBuffer);
   glBindVertexBuffer(0, screenQuad.myVertexBuffer, 0, sizeof(DrawVertex));
   int attrLoc;

   attrLoc = glGetAttribLocation(screenQuad.myProgram, "position");
   glVertexAttribFormat(attrLoc, 3, GL_FLOAT, GL_FALSE, 0);
   glVertexAttribBinding(attrLoc, 0);
   glEnableVertexAttribArray(attrLoc);

   attrLoc = glGetAttribLocation(screenQuad.myProgram, "uv");
   glVertexAttribFormat(attrLoc, 2, GL_FLOAT, GL_FALSE, 12);
   glVertexAttribBinding(attrLoc, 0);
   glEnableVertexAttribArray(attrLoc);

   //cleanup VAO
   glBindVertexArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   //setup texture
   glCreateTextures(GL_TEXTURE_2D, 1, &screenQuad.myTexture);
   glTextureParameteri(screenQuad.myTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTextureParameteri(screenQuad.myTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTextureParameteri(screenQuad.myTexture, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTextureParameteri(screenQuad.myTexture, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTextureStorage2D(screenQuad.myTexture, 1, GL_RGBA8, SCREEN_WIDTH, SCREEN_HEIGHT);
   glBindTexture(GL_TEXTURE_2D, 0);
}

void initCube()
{
   std::vector<DrawVertex> verts;
   std::vector<unsigned short> indexes;

   verts.resize(8);
   indexes.resize(36);

   verts[0].position = Vector3(-1, -1, -1);  verts[0].color = 0xff0000ff; //red
   verts[1].position = Vector3( 1, -1, -1);  verts[1].color = 0xff00ff00; //green
   verts[2].position = Vector3(-1,  1, -1);  verts[2].color = 0xffff0000; //blue
   verts[3].position = Vector3( 1,  1, -1);  verts[3].color = 0xffffffff; //white
   verts[4].position = Vector3(-1, -1,  1);  verts[4].color = 0xff000000; //black
   verts[5].position = Vector3( 1, -1,  1);  verts[5].color = 0xff00ffff; //yellow
   verts[6].position = Vector3(-1,  1,  1);  verts[6].color = 0xffffff00; //aqua
   verts[7].position = Vector3( 1,  1,  1);  verts[7].color = 0xffff00ff; //fuchsia

   int i = 0;
   indexes[i++] = 1; indexes[i++] = 0; indexes[i++] = 2; //front
   indexes[i++] = 1; indexes[i++] = 2; indexes[i++] = 3;
   indexes[i++] = 0; indexes[i++] = 4; indexes[i++] = 6; //left
   indexes[i++] = 0; indexes[i++] = 6; indexes[i++] = 2;
   indexes[i++] = 5; indexes[i++] = 1; indexes[i++] = 3; //right
   indexes[i++] = 5; indexes[i++] = 3; indexes[i++] = 7;
   indexes[i++] = 4; indexes[i++] = 5; indexes[i++] = 7; //back
   indexes[i++] = 4; indexes[i++] = 7; indexes[i++] = 6;
   indexes[i++] = 6; indexes[i++] = 7; indexes[i++] = 3; //top
   indexes[i++] = 6; indexes[i++] = 3; indexes[i++] = 2;
   indexes[i++] = 0; indexes[i++] = 1; indexes[i++] = 5; //bottom
   indexes[i++] = 0; indexes[i++] = 5; indexes[i++] = 4;

   cube.indexCount = indexes.size();

   glGenBuffers(1, &cube.myVertexBuffer);
   glGenBuffers(1, &cube.myIndexBuffer);
   glGenVertexArrays(1, &cube.myVao);

   cube.myProgram = createCubeShader();

   //get uniform locations
   cube.myViewUniform = glGetUniformLocation(cube.myProgram, "view");
   cube.myProjectionUniform = glGetUniformLocation(cube.myProgram, "projection");
   cube.myModelUniform = glGetUniformLocation(cube.myProgram, "model");

   //setup the buffers
   glBindBuffer(GL_ARRAY_BUFFER, cube.myVertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(DrawVertex), &verts[0], GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.myIndexBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(unsigned short), &indexes[0], GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   //Setup the VAO
   glBindVertexArray(cube.myVao);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.myIndexBuffer);
   glBindVertexBuffer(0, cube.myVertexBuffer, 0, sizeof(DrawVertex));
   int attrLoc;

   attrLoc = glGetAttribLocation(cube.myProgram, "position");
   glVertexAttribFormat(attrLoc, 3, GL_FLOAT, GL_FALSE, 0);
   glVertexAttribBinding(attrLoc, 0);
   glEnableVertexAttribArray(attrLoc);

   attrLoc = glGetAttribLocation(cube.myProgram, "color");
   glVertexAttribFormat(attrLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 20);
   glVertexAttribBinding(attrLoc, 0);
   glEnableVertexAttribArray(attrLoc);

   //cleanup VAO
   glBindVertexArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void drawCube()
{
   static float rotx = 0;
   static float roty = 0;

   rotx += 0.001;
   roty += 0.001;

   glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glm::mat4 proj = glm::perspective(45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
   glm::mat4 model = glm::mat4(1.0f);
   model = glm::rotate(model, rotx, glm::vec3(1, 0, 0));
   model = glm::rotate(model, roty, glm::vec3(0, 1, 0));
   glm::mat4 view = glm::lookAt<float>(glm::vec3(0.f, 0.f, 5.0f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f));

   
   glEnable(GL_BLEND);
   glBindTexture(GL_TEXTURE_2D, cube.myTexture);
   glUseProgram(cube.myProgram);
   glUniformMatrix4fv(cube.myProjectionUniform, 1, GL_FALSE, glm::value_ptr(proj));
   glUniformMatrix4fv(cube.myViewUniform, 1, GL_FALSE, glm::value_ptr(view));
   glUniformMatrix4fv(cube.myModelUniform, 1, GL_FALSE, glm::value_ptr(model));
   glBindVertexArray(cube.myVao);
   glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_SHORT, 0);
}

void drawInterface(EA::WebKit::View* v)
{
   updateGLTexture(v, screenQuad.myTexture);

   glm::mat4 proj = glm::ortho<float>(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT, 0.1f, 100.0f);
   glm::mat4 view = glm::lookAt<float>(glm::vec3(0.f, 0.f, 5.0f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f));
   
   glEnable(GL_BLEND);
   glBindTexture(GL_TEXTURE_2D, screenQuad.myTexture);
   glUseProgram(screenQuad.myProgram);
   glUniformMatrix4fv(screenQuad.myProjectionUniform, 1, GL_FALSE, glm::value_ptr(proj));
   glUniformMatrix4fv(screenQuad.myViewUniform, 1, GL_FALSE, glm::value_ptr(view));
   glBindVertexArray(screenQuad.myVao);
   glDrawElements(GL_TRIANGLES, screenQuad.indexCount, GL_UNSIGNED_SHORT, 0);
}

int main(int argc, char** argv)
{
   void SDL_SetMainReady(void);

   //Initialize SDL
   if(SDL_Init(SDL_INIT_VIDEO) < 0)
   {
      std::cout << "SDL could not initialize! SDL_Error:" << SDL_GetError() << std::endl;;
      return 0;
   }

   //Use OpenGL 4.5 core
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

   //Create window
   gWindow = SDL_CreateWindow("Webkit Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
   gContext = SDL_GL_CreateContext(gWindow);
   if(SDL_GL_SetSwapInterval(0) < 0)
   {
      std::cout << "Warning: Unable to set VSync! SDL Error: " << SDL_GetError() << std::endl;
   }

   //Main loop flag
   bool quit = false;

   //Event handler
   SDL_Event e;

   //initialization
   initGL();
   initScreenQuad();
   initCube();
   initWebkit();
   v = createView();

   //While application is running
   while(!quit)
   {
      //Handle events on queue
      while(SDL_PollEvent(&e) != 0)
      {
         //User requests quit
         if(e.type == SDL_QUIT)
         {
            quit = true;
         }

         if(e.type == SDL_KEYUP)
         {
            if(e.key.keysym.sym == SDLK_ESCAPE)
            {
               quit == true;
            }

            if(e.key.keysym.sym == SDLK_F5)
            {
               v->Refresh();
            }
         }

         if(e.type == SDL_MOUSEMOTION)
         {
            mousemove(v, e.motion.x, e.motion.y);
         }

         if(e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEBUTTONDOWN)
         {
            mousebutton(v, e.button.x, e.button.y, e.button.button, e.button.state);
         }
      }

      double start, stop;

      if(frame % 500 == 0) std::cout << "------------------------------------------------------" << std::endl;
      
      start = timerCallback();
      updateWebkit();
      updateView(v);
      stop = timerCallback();
      if(frame % 500 == 0) std::cout << "Webview update time: " << (stop - start) * 1000.0 << "ms" << std::endl;

      start = timerCallback();
      drawCube();
      drawInterface(v);
      stop = timerCallback();
      if(frame % 500 == 0) std::cout << "Update OpenGL and render: " << (stop - start) * 1000.0 << "ms" << std::endl;


      frame++;

      //Update screen
      SDL_GL_SwapWindow(gWindow);
   }

   destroyView(v);

   //Destroy window
   SDL_DestroyWindow(gWindow);

   //Quit SDL subsystems
   SDL_Quit();

   return 0;
}