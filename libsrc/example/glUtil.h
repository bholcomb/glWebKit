struct Vector2
{
   Vector2() {x=0; y=0;}
   Vector2(float xx, float yy) {x=xx; y=yy;}
   float x;
   float y;
};

struct Vector3
{
   Vector3() {x=0; y=0; z=0;}
   Vector3(float xx, float yy, float zz) {x=xx; y=yy; z=zz;}
   Vector3(const Vector2& v) {x = v.x; y= v.y; z=0;}
   float x;
   float y;
   float z;
};

struct DrawVertex
{
   Vector3 position;
   Vector2 uv;
   int color;
};

struct DrawCall
{
   unsigned int myVertexBuffer;
   unsigned int myIndexBuffer;
   unsigned int myProgram;
   unsigned int myVao;
   unsigned int myTexture;
   unsigned int myViewUniform;
   unsigned int myProjectionUniform;
   unsigned int myModelUniform;
   unsigned int myTextureUniform;
   unsigned int indexCount;
};

extern const char* webkitVertexShader;
extern const char* webkitFragmentShader;
extern const char* cubeVertexShader;
extern const char* cubeFragmentShader;

void printShaderInfoLog(unsigned int obj);
void printProgramInfoLog(unsigned int obj);

unsigned int createShader(const char* vert, const char* frag);