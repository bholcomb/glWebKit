#include "glWebkitUtils.h"
#include "glWebKit.h"


#include <GL/glew.h>

#include <EAWebKit/EAWebKit.h>
#include <EAWebKit/EAWebkitAllocator.h>
#include <EAWebKit/EAWebKitFileSystem.h>
#include <EAWebKit/EAWebKitClient.h>
#include <EAWebKit/EAWebKitView.h>
#include "EAWebkit/EAWebKitTextInterface.h"

#include <stdio.h>

#include <windows.h> // LoadLibraryA
#include <assert.h>
#include <array>

#include <Shlwapi.h>
#include <vector>

#include <iostream>
#include <algorithm>

unsigned int frame = 0;

std::string getExePath()
{
    char path[MAX_PATH] = "";
    DWORD length = GetModuleFileNameA(NULL, path, MAX_PATH);
    PathRemoveFileSpecA(path);
    return std::string(path);
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to) 
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

int getSystemFonts(std::vector<std::string>& fonts) 
{
    static const LPWSTR fontRegistryPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    HKEY hKey;
    LONG result;

    // Open Windows font registry key
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, fontRegistryPath, 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        return 1;
    }

    DWORD maxValueNameSize = 0, maxValueDataSize = 0;
    result = RegQueryInfoKey(hKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0);
    if (result != ERROR_SUCCESS) {
        return 1;
    }

    DWORD valueIndex = 0;
    LPSTR valueName = new CHAR[maxValueNameSize];
    LPBYTE valueData = new BYTE[maxValueDataSize];
    DWORD valueNameSize, valueDataSize, valueType;


    // Build full font file path
    char winDir_[MAX_PATH] = "";
    GetWindowsDirectoryA(winDir_, MAX_PATH);
    std::string winDir = std::string(winDir_);
    fonts.clear();

    do {
        valueDataSize = maxValueDataSize;
        valueNameSize = maxValueNameSize;

        result = RegEnumValueA(hKey, valueIndex, valueName, &valueNameSize, 0, &valueType, valueData, &valueDataSize);

        valueIndex++;

        if (result != ERROR_SUCCESS || valueType != REG_SZ) {
            continue;
        }

        std::string wsValueName(valueName, valueNameSize);
        std::string wsValueData((LPSTR)valueData, valueDataSize-1); //remove trailing \0 in data

        std::string ext = wsValueData.substr(wsValueData.length() - 3);
        std::transform(ext.begin(), ext.end(),ext.begin(), ::tolower);

        if(ext != std::string("ttf") && ext != std::string("ttc") )
           continue;

        if(wsValueData.substr(0, 2) == std::string("C:"))
           continue;

        std::string fontPath = winDir + "\\Fonts\\" + wsValueData;
        fonts.push_back(fontPath);

    } while (result != ERROR_NO_MORE_ITEMS);

    delete[] valueName;
    delete[] valueData;

    RegCloseKey(hKey);
    return 0;
}

int add_ttf_font(EA::WebKit::EAWebKitLib* wk, const char* ttfFile) 
{
    EA::WebKit::ITextSystem* ts = wk->GetTextSystem();

    FILE* f = 0;
    fopen_s(&f, ttfFile, "rb");
    if (!f) return 1;
    fseek(f, 0L, SEEK_END);
    size_t fileSize = ftell(f);
    fseek(f, 0L, SEEK_SET);
    char* buffer = (char*)calloc(fileSize + 6, 1);
    size_t read_bytes = fread(buffer, 1, fileSize, f);
    if (read_bytes != fileSize) 
    {
        // error!
        free(buffer);
        return 0;
    }

    //Text system will take ownership of this memory
    int numFaces = ts->AddFace(buffer, fileSize);

    return numFaces;
}

int init_system_fonts(EA::WebKit::EAWebKitLib* wk) 
{
    std::vector<std::string> fonts;
    if (getSystemFonts(fonts)) 
    {
        return 1;
    }
    int fonts_installed = 0;
    for (int i = 0; i < fonts.size(); ++i) 
    {
        add_ttf_font(wk, fonts[i].c_str());
    }
    return 0;
}

unsigned int vPbo[2] = { 0, 0 };
unsigned char* buffer[2] = { 0, 0 };
int index = 0;
int nextIndex = 0;

void updateGLTexture(EA::WebKit::View* v, unsigned int id)
{
   if(!v)
      return;

   int w, h;
   EA::WebKit::ISurface* surface = v->GetDisplaySurface();
   surface->GetContentDimensions(&w, &h);
   int dataSize = w * h * 4;

   index = (index + 1) % 2;
   nextIndex = (index + 1) % 2;
   double start, stop;

   //lazily create and map the PBOs
   if(vPbo[0] == 0)
   {
      glGenBuffers(2, vPbo);

      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vPbo[0]);
      glBufferStorage(GL_PIXEL_UNPACK_BUFFER, dataSize, 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
      buffer[0] = (unsigned char*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, dataSize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vPbo[1]);
      glBufferStorage(GL_PIXEL_UNPACK_BUFFER, dataSize, 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
      buffer[1] = (unsigned char*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, dataSize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
   }

   EA::WebKit::ISurface::SurfaceDescriptor sd = {};
   surface->Lock(&sd);

   //copy the last frame's data to the texture.  This is a frame of lag, but should allow for fast transfers
   start = timerCallback();
   glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vPbo[index]);
   glTextureSubImage2D(id, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 0);
   stop = timerCallback();
   //if(frame % 100 == 0) std::cout << "glTextureSubImage2D time: " << (stop - start) * 1000.0 << "ms" << std::endl;

   //copy the current data to the next buffer
   start = timerCallback();
   if(buffer)
   {
      //This image is actually flipped, but we're going to handle that in the shader instead of trying to actually flip
      //it on the CPU
      memcpy(buffer[nextIndex], sd.mData, w * h * 4);
   }
   stop = timerCallback();
   //if(frame % 100 == 0) std::cout << "Upload to PBO time: " << (stop - start) * 1000.0 << "ms" << std::endl;

   surface->Unlock();   
}
