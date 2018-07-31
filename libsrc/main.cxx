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

#include "glUtil.h"

#include <vector>
#include <iostream>

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

unsigned int myVertexBuffer;
unsigned int myIndexBuffer;
unsigned int myVs;
unsigned int myPs;
unsigned int myProgram;
unsigned int myVao;
unsigned int myTexture;
unsigned int myViewUniform;
unsigned int myProjectionUniform;
unsigned int myTextureUniform;

std::vector<DrawVertex> myVerts;
std::vector<unsigned short> myIndexes;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#pragma region shader
const char* vertexShader =
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

const char* fragmentShader =
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
"   vec4 outColor = texture(tex, texCoord);\n"
"   if(outColor.a < 0.1) \n"
"      discard;\n"
"\n"
"   FragColor = outColor;"
"}\n";
#pragma endregion

void initGL()
{
   GLenum err = glewInit();
   if(GLEW_OK != err)
   {
      std::cout << "Error initializing GLEW" << std::endl;
      exit(0);
   }

   myVerts.resize(4);
   myIndexes.resize(6);
   
   myVerts[0].position = Vector3(-1, -1, 0); myVerts[0].uv = Vector2(0, 0);
   myVerts[1].position = Vector3(1, -1, 0); myVerts[1].uv = Vector2(1, 0);
   myVerts[2].position = Vector3(1, 1, 0); myVerts[2].uv = Vector2(1, 1);
   myVerts[3].position = Vector3(-1, 1, 0); myVerts[3].uv = Vector2(0, 1);

   myIndexes[0] = 0; myIndexes[1] = 1; myIndexes[2] = 2;
   myIndexes[3] = 0; myIndexes[4] = 2; myIndexes[5] = 3;

   glGenBuffers(1, &myVertexBuffer);
   glGenBuffers(1, &myIndexBuffer);
   glGenVertexArrays(1, &myVao);
   myVs = glCreateShader(GL_VERTEX_SHADER);
   myPs = glCreateShader(GL_FRAGMENT_SHADER);
   myProgram = glCreateProgram();

   //create the shaders
   int len = strlen(vertexShader);
   glShaderSource(myVs, 1, (const char**)&vertexShader, &len);
   glCompileShader(myVs);
   //printShaderInfoLog(myVs);

   len = strlen(fragmentShader);
   glShaderSource(myPs, 1, (const char**)&fragmentShader, &len);
   glCompileShader(myPs);
   //printShaderInfoLog(myPs);

   //link to a program
   glAttachShader(myProgram, myVs);
   glAttachShader(myProgram, myPs);
   glLinkProgram(myProgram);
   //printProgramInfoLog(myProgram);

   //get uniform locations
   myViewUniform = glGetUniformLocation(myProgram, "view");
   myProjectionUniform = glGetUniformLocation(myProgram, "projection");
   myTextureUniform = glGetUniformLocation(myProgram, "tex");

   //setup the buffers
   glBindBuffer(GL_ARRAY_BUFFER, myVertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, myVerts.size() * sizeof(DrawVertex), &myVerts[0], GL_STREAM_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myIndexBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, myIndexes.size() * sizeof(unsigned short), &myIndexes[0], GL_STREAM_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   //Setup the VAO
   glBindVertexArray(myVao);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myIndexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, myVertexBuffer);
   int attrLoc;

   attrLoc = glGetAttribLocation(myProgram, "position");
   glVertexAttribPointer(attrLoc, 3, GL_FLOAT, GL_FALSE, sizeof(DrawVertex), BUFFER_OFFSET(0));
   glEnableVertexAttribArray(attrLoc);

   attrLoc = glGetAttribLocation(myProgram, "uv");
   glVertexAttribPointer(attrLoc, 2, GL_FLOAT, GL_FALSE, sizeof(DrawVertex), BUFFER_OFFSET(12));
   glEnableVertexAttribArray(attrLoc);

   //cleanup VAO
   glBindVertexArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   //setup texture
   glGenTextures(1, &myTexture);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, myTexture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glBindTexture(GL_TEXTURE_2D, 0);
}

void updateTexture(unsigned int id)
{
   if(!v)
      return;

   EA::WebKit::ISurface* surface = v->GetDisplaySurface();

   int w, h;
   surface->GetContentDimensions(&w, &h);
   
   EA::WebKit::ISurface::SurfaceDescriptor sd = {};
   surface->Lock(&sd);

   //flip the image for opengl style layout where first pixel is bottom left
   int bytesPerRow = 4 * w;
   unsigned char* flipBuffer = new unsigned char[w * h * 4];
   unsigned char* readhead = (unsigned char*)sd.mData + (w * h * 4) - bytesPerRow;
   unsigned char* writeHead = flipBuffer;
   for(int i = 0; i < h; i++)
   {
      memcpy(writeHead, readhead, bytesPerRow);
      writeHead += bytesPerRow;
      readhead -= bytesPerRow;
   }

   glBindTexture(GL_TEXTURE_2D, id);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, flipBuffer);

   delete[] flipBuffer;
   
   surface->Unlock();
}

void drawCube()
{
   glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void drawInterface()
{
   update();
   updateTexture(myTexture);

   //glm::mat4 proj = glm::perspective(60.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
   glm::mat4 proj = glm::ortho(-2.0f, (float)2.0f, -2.0f, (float)2.0, 0.1f, 100.0f);
   glm::mat4 view = glm::lookAt(glm::vec3(0.f, 0.f, -1.0f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f));
   
   glEnable(GL_BLEND);
   glBindTexture(GL_TEXTURE_2D, myTexture);
   glUseProgram(myProgram);
   glUniformMatrix4fv(myProjectionUniform, 1, GL_FALSE, glm::value_ptr(proj));
   glUniformMatrix4fv(myViewUniform, 1, GL_FALSE, glm::value_ptr(view));
   glBindVertexArray(myVao);
   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

int main(int argc, char** argv)
{
   void SDL_SetMainReady(void);

   //Initialize SDL
   if(SDL_Init(SDL_INIT_VIDEO) < 0)
   {
      printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
      return 0;
   }

   //Use OpenGL 4.5 core
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

   //Create window
   gWindow = SDL_CreateWindow("Webkit Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
   gContext = SDL_GL_CreateContext(gWindow);
   if(SDL_GL_SetSwapInterval(1) < 0)
   {
      printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
   }

   //Main loop flag
   bool quit = false;

   //Event handler
   SDL_Event e;

   //init eawebkit
   initGL();
   init();

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
      }      

      drawCube();
      drawInterface();

      //Update screen
      SDL_GL_SwapWindow(gWindow);
   }

   shutdown();

   //Destroy window
   SDL_DestroyWindow(gWindow);

   //Quit SDL subsystems
   SDL_Quit();

   return 0;
}