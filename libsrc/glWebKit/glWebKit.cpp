#include "glWebKit/glWebKit.h"

#include "glWebkitUtils.h"
#include "glWebkitRenderer.h"
#include "glWebkitThreading.h"
#include "glWebkitClient.h"

#include <EAWebKit\EAWebKit>

#include <windows.h>
#include <bcrypt.h>

#include <list>
#include <iostream>

EA::WebKit::EAWebKitLib* wk = nullptr;

// Callbacks
double timerCallback() 
{ 
    LARGE_INTEGER frequency;
    ::QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER start;
    ::QueryPerformanceCounter(&start);

    return static_cast<double>(start.QuadPart) / (double)frequency.QuadPart;
}

double monotonicTimerCallback() 
{
    return timerCallback();
};

bool cryptographicallyRandomValueCallback(unsigned char *buffer, size_t length)
{
   //deprecated API, using newer api below
//     HCRYPTPROV hCryptProv = 0;
//     CryptAcquireContext(&hCryptProv, 0, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
//     CryptGenRandom(hCryptProv, length, buffer);
//     CryptReleaseContext(hCryptProv, 0);

    BCRYPT_ALG_HANDLE algorithm;
    NTSTATUS ret;
    ret = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_RNG_ALGORITHM, 0, 0);
    if(ret != 0)
    {
       std::cout << "Failed to initialize cryto algorithm provider.  Return code: " << ret << std::endl;
       return false;
    }

    ret = BCryptGenRandom(algorithm, buffer, length, 0);
    if(ret != 0)
    {
       std::cout << "Failed to get cryto random number.  Return code: " << ret << std::endl;
       return false;
    }
    
    return true;  // Returns true if no error, else false
}

void* stackBaseCallback() 
{
   //taken from: https://github.com/adobe/webkit/blob/master/Source/WTF/wtf/StackBounds.cpp#L228
   PNT_TIB64 pTib = reinterpret_cast<PNT_TIB64>(NtCurrentTeb());
   return reinterpret_cast<void*>(pTib->StackBase);
}

void getCookiesCallback(const char16_t* pUrl, EA::WebKit::EASTLFixedString16Wrapper& result, uint32_t flags)
{
   std::cout << __FUNCTION__ << std::endl;
}

bool setCookieCallback(const EA::WebKit::CookieEx& cookie)
{
   std::cout << __FUNCTION__ << std::endl;
   return false;  
}

struct EA::WebKit::AppCallbacks callbacks = {
   timerCallback,
   monotonicTimerCallback,
   stackBaseCallback,
   cryptographicallyRandomValueCallback,
   getCookiesCallback,
   setCookieCallback
};

// init the systems: using DefaultAllocator, DefaultFileSystem, no text/font support, DefaultThreadSystem
struct EA::WebKit::AppSystems systems = { nullptr };

bool initWebkit()
{
   systems.mThreadSystem = new StdThreadSystem;
   systems.mEAWebkitClient = new GLWebkitClient();

   typedef EA::WebKit::EAWebKitLib* (*PF_CreateEAWebkitInstance)(void);
   PF_CreateEAWebkitInstance create_Webkit_instance = nullptr;

#ifdef _DEBUG
   HMODULE wdll = LoadLibraryA("EAWebkitd.dll");
#else
   HMODULE wdll = LoadLibraryA("EAWebkit.dll");
#endif // _DEBUG
   if(wdll != nullptr)
   {
      create_Webkit_instance = reinterpret_cast<PF_CreateEAWebkitInstance>(GetProcAddress(wdll, "CreateEAWebkitInstance"));
   }

   // init winsock manually, this is required
   WSADATA wsadata = {};
   WSAStartup(MAKEWORD(2, 0), &wsadata);

   if(!create_Webkit_instance)
   {
      std::cout << "EAWebkit.dll missing" << std::endl;
      return false;
   }

   wk = create_Webkit_instance();

   //check that dll is same version as our headers
   const char* verStr = wk->GetVersion();
   if(strcmp(verStr, EAWEBKIT_VERSION_S) != 0)
   {
      std::cout << "Error!  Mismatched versions of EA Webkit" << std::endl;
      return false;
   }

   //initialize the system
   wk->Init(&callbacks, &systems);

   EA::WebKit::Parameters& params = wk->GetParameters();
   params.mEAWebkitLogLevel = 4;
   params.mHttpManagerLogLevel = 4;
   params.mRemoteWebInspectorPort = 1234;
   params.mReportJSExceptionCallstacks = true;
   params.mVerifySSLCert = true;
   params.mJavaScriptStackSize = 1024 * 1024; //1MB of stack space

   wk->SetParameters(params);

   //should be pulling these from the OS by their family type
   //times new roman is the default fallback if a font isn't found, so we need 
   //to at least load this (should probably be built in)
   int ret = add_ttf_font(wk, "times.ttf");
   if (ret == 0)
   {
      std::cout << "Error adding times.ttf font. " << std::endl;
   }

   return true;
}

EA::WebKit::View* createView(int x, int y)
{
   EA::WebKit::View* v = 0;

   v = wk->CreateView();
   EA::WebKit::ViewParameters vp;
   vp.mHardwareRenderer = nullptr; // use default renderer
   vp.mDisplaySurface = nullptr; // use default surface
   vp.mWidth = x;
   vp.mHeight = y;
   vp.mBackgroundColor = 0; //clear  0xffffffff; //white
   vp.mTileSize = 256;
   vp.mUseTiledBackingStore = false;
   vp.mpUserData = v;
   v->InitView(vp);
   v->SetSize(EA::WebKit::IntSize(vp.mWidth, vp.mHeight));

   return v;
}

bool shutdownWebKit()
{
   wk->Shutdown();

   return true;
}

void updateWebkit()
{
   if (!wk) 
       return;

    wk->Tick();
    
}

void setViewUrl(EA::WebKit::View* v, const char* url)
{
   v->SetURI(url);
}

void updateView(EA::WebKit::View* v)
{
    v->Paint();
}

void resize(EA::WebKit::View* v, int width, int height)
{
    if (!v) 
       return;

    v->SetSize(EA::WebKit::IntSize(width, height));
}

void mousemove(EA::WebKit::View* v, int x, int y)
{
    if (!v) 
       return;

    EA::WebKit::MouseMoveEvent e = {};
    e.mX = x;
    e.mY = y;
    v->OnMouseMoveEvent(e);
}

void mousebutton(EA::WebKit::View* v, int x, int y, int btn, bool depressed)
{
    if (!v) 
       return;

    EA::WebKit::MouseButtonEvent e = {};
    e.mId = btn;
    e.mX = x;
    e.mY = y;
    e.mbDepressed = depressed;
    v->OnMouseButtonEvent(e);
}

void mousewheel(EA::WebKit::View* v, int x, int y, int keys, int delta)
{
    if (!v) 
       return;

    EA::WebKit::MouseWheelEvent e = {};
    e.mX = x;
    e.mY = y;
    e.mZDelta = delta;

    UINT scrollLines = 1;
    SystemParametersInfoA(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, 0);
    e.mNumLines = ((delta * (int32_t)scrollLines) / (int32_t)WHEEL_DELTA);
    v->OnMouseWheelEvent(e);
}

void keyboard(EA::WebKit::View* v, int id, bool ischar, bool depressed)
{
    if (!v) 
       return;

    EA::WebKit::KeyboardEvent e = {};
    e.mId = id;
    e.mbChar = ischar;
    e.mbDepressed = depressed;
    v->OnKeyboardEvent(e);
}

void reload(EA::WebKit::View* v)
{
    if (!v)
       return;

    v->Refresh();
}

void destroyView(EA::WebKit::View* v)
{
   if(!wk || !v)
      return;

   wk->DestroyView(v);
}
