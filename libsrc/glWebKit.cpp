
#include "glWebkit.h"
#include "glWebkitUtils.h"
#include "glWebkitRenderer.h"
#include "glWebkitThreading.h"

#include <EAWebKit\EAWebKit>
// #include <EAText\EAText.h>
// #include <EAText\EATextFontServer.h>

#include <windows.h>

#include <iostream>

EA::WebKit::EAWebKitLib* wk = nullptr;
EA::WebKit::View* v = nullptr;


// Callbacks
double timerCallback() 
{ 
    LARGE_INTEGER frequency;
    ::QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER start;
    ::QueryPerformanceCounter(&start);

    return static_cast<double>(start.QuadPart) / frequency.QuadPart;
}

double monotonicTimerCallback() 
{
    return timerCallback();
};

bool cryptographicallyRandomValueCallback(unsigned char *buffer, size_t length)
{
    HCRYPTPROV hCryptProv = 0;
    CryptAcquireContext(&hCryptProv, 0, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
    CryptGenRandom(hCryptProv, length, buffer);
    CryptReleaseContext(hCryptProv, 0);
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

}

bool setCookieCallback(const EA::WebKit::CookieEx& cookie)
{
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

class GLWebkitClient : public EA::WebKit::EAWebKitClient 
{
public:
   virtual void DebugLog(EA::WebKit::DebugLogInfo& l) override
   {
      std::cout << l.mType << ": " << l.mpLogText << std::endl;
   }

protected:
};

bool init()
{  
    // init the systems: using DefaultAllocator, DefaultFileSystem, no text/font support, DefaultThreadSystem
    struct EA::WebKit::AppSystems systems = { nullptr };
    
    systems.mThreadSystem = new StdThreadSystem;
    systems.mEAWebkitClient = new GLWebkitClient();

    typedef EA::WebKit::EAWebKitLib* (*PF_CreateEAWebkitInstance)(void);
    PF_CreateEAWebkitInstance create_Webkit_instance = nullptr;

#ifdef _DEBUG
    HMODULE wdll = LoadLibraryA("EAWebkitd.dll");
#else
    HMODULE wdll = LoadLibraryA("EAWebkit.dll");
#endif // _DEBUG
    if (wdll != nullptr) 
    {
        create_Webkit_instance = reinterpret_cast<PF_CreateEAWebkitInstance>(GetProcAddress(wdll, "CreateEAWebkitInstance"));
    }

    if (!create_Webkit_instance) 
    {
        printf("EAWebkit.dll missing\n");
        exit(1);
    }

    // init winsock manually, this is required
    WSADATA wsadata = {};
    WSAStartup(MAKEWORD(2, 0), &wsadata);


    wk = create_Webkit_instance();
    
    //check that dll is same version as our headers
    const char* verStr = wk->GetVersion();
    if(strcmp(verStr, EAWEBKIT_VERSION_S) != 0)
    {
       std::cout << "Error!  Mismatched versions of EA Webkit" << std::endl;
       exit(1);
    }

    wk->Init(&callbacks, &systems);

    EA::WebKit::Parameters& params = wk->GetParameters();
    params.mEAWebkitLogLevel = 4;
    params.mHttpManagerLogLevel = 4;
    params.mRemoteWebInspectorPort = 1234;
    params.mReportJSExceptionCallstacks = true;
    params.mJavaScriptStackSize = 1024 * 1024;
    params.mVerifySSLCert = false;

    wk->SetParameters(params);
    
    //initialize the text system (if it hasn't been already)
    EA::WebKit::ITextSystem* ts = wk->GetTextSystem();
    ts->Init();
    
    init_system_fonts(wk);
    
    
    v = wk->CreateView();
    EA::WebKit::ViewParameters vp;
    vp.mHardwareRenderer = nullptr; // use default renderer
    vp.mDisplaySurface = nullptr; // use default surface
    vp.mWidth = 1280;
    vp.mHeight = 720;
    vp.mBackgroundColor = 0; //clear  0xffffffff; //white  
    vp.mTileSize = 256;
    vp.mUseTiledBackingStore = false;
    vp.mpUserData = v;
    v->InitView(vp);
    v->SetSize(EA::WebKit::IntSize(vp.mWidth, vp.mHeight));
    
    
    //v->SetURI("test.html");
    //const char test[] = "<div style='border:10px dashed red;'> </div>";
    const char test[] = "<div style='border:10px dashed red;'> </div><h1>My First Heading</h1><p>My first paragraph.</p>";
    
    v->SetHtml(test, sizeof(test));

    return true;
}

void update() 
{
    if (!wk || !v) 
       return;

    wk->Tick();
    
    v->ForceInvalidateFullView();
    v->Paint();
}

void resize(int width, int height)
{
    if (!v) 
       return;
    v->SetSize(EA::WebKit::IntSize(width, height));
}

void mousemove(int x, int y) 
{
    if (!v) 
       return;
    EA::WebKit::MouseMoveEvent e = {};
    e.mX = x;
    e.mY = y;
    v->OnMouseMoveEvent(e);
}

void mousebutton(int x, int y, int btn, bool depressed) 
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

void mousewheel(int x, int y, int keys, int delta) 
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

void keyboard(int id, bool ischar, bool depressed)
{
    if (!v) 
       return;
    EA::WebKit::KeyboardEvent e = {};
    e.mId = id;
    e.mbChar = ischar;
    e.mbDepressed = depressed;
    v->OnKeyboardEvent(e);
}

void reload() 
{
    if (!v)
       return;
    v->Refresh();
}

void shutdown()
{
   if(!wk || !v)
      return;

   wk->DestroyView(v);
}
