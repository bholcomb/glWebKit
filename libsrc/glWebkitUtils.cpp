#include "glWebkitUtils.h"


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

#include <algorithm>



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

void updateGLTexture(EA::WebKit::View* v, unsigned int id)
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
