
#include "glWebkitRenderer.h"

#include <EAWebKit/EAWebKit.h>
#include <EAWebKit/EAWebkitAllocator.h>
#include <EAWebKit/EAWebKitFileSystem.h>
#include <EAWebKit/EAWebKitClient.h>
#include <EAWebKit/EAWebKitView.h>
#include "EAWebkit/EAWebKitTextInterface.h"

#include <vector>
#include <array>
#include <iostream>

GLRenderer::GLRenderer()
{

}

GLRenderer::~GLRenderer()
{

}

EA::WebKit::ISurface * GLRenderer::CreateSurface(EA::WebKit::SurfaceType surfaceType, const void* data /*= 0*/, size_t length /*= 0*/)
{
    GLSurface* res = new GLSurface();
    if (data && length)
    {
        EA::WebKit::ISurface::SurfaceDescriptor sd = {};
        res->Lock(&sd);
        memcpy(sd.mData, data, length);
        res->Unlock();
    }
    return res;
}

void GLRenderer::SetRenderTarget(EA::WebKit::ISurface *target)
{
    std::cout << __FUNCTION__ << std::endl;
}

void GLRenderer::RenderSurface(EA::WebKit::ISurface *surface, EA::WebKit::FloatRect &target, EA::WebKit::TransformationMatrix &matrix, float opacity, EA::WebKit::CompositOperator op, EA::WebKit::TextureWrapMode wrap, EA::WebKit::Filters &filters)
{
   std::cout << __FUNCTION__ << std::endl;
}

void GLRenderer::FillColor(uint32_t premultiplied_rgba32, EA::WebKit::FloatRect &target, EA::WebKit::TransformationMatrix &matrix, EA::WebKit::CompositOperator op)
{
    std::cout << __FUNCTION__ << std::endl;
}

void GLRenderer::DrawOutline(uint32_t premultiplied_rgba32, EA::WebKit::FloatRect &target, EA::WebKit::TransformationMatrix &matrix)
{
    std::cout << __FUNCTION__ << std::endl;
}

int32_t GLRenderer::MaxTextureSize(void)
{
    return 4096;
}

void GLRenderer::Clear(EA::WebKit::ClearFlags flags, uint32_t premultiplied_rgba32, float z, uint32_t stencil)
{
    std::cout << __FUNCTION__ << std::endl;
}

void GLRenderer::ScissorClip(EA::WebKit::IntRect axisAlignedRect)
{
    std::cout << __FUNCTION__ << std::endl;
}

void GLRenderer::DrawStencil(EA::WebKit::TransformationMatrix &matrix, EA::WebKit::FloatRect &target, uint32_t stencilIndex)
{
    std::cout << __FUNCTION__ << std::endl;
}

void GLRenderer::ClipAgainstStencil(uint32_t stencilIndex)
{
    std::cout << __FUNCTION__ << std::endl;
}

bool GLRenderer::UseCustomClip()
{
    return false;
}

void GLRenderer::BeginClip(EA::WebKit::TransformationMatrix &matrix, EA::WebKit::FloatRect &target)
{
    std::cout << __FUNCTION__ << std::endl;
}

void GLRenderer::EndClip(void)
{
    std::cout << __FUNCTION__ << std::endl;
}

EA::WebKit::IntRect GLRenderer::CurrentClipBound()
{
    return EA::WebKit::IntRect(0, 0, 800, 600);
}

void GLRenderer::BeginPainting(void)
{
   std::cout << __FUNCTION__ << std::endl;
}

void GLRenderer::EndPainting(void)
{
    std::cout << __FUNCTION__ << std::endl;
}



//------------------------GL Surface ------------------------------------


GLSurface::GLSurface()
{

}

GLSurface::~GLSurface()
{

}

void GLSurface::Lock(SurfaceDescriptor *pSDOut, const EA::WebKit::IntRect *rect /*= NULL*/)
{

}

void GLSurface::Unlock(void)
{

}

void GLSurface::Release(void)
{

}

bool GLSurface::IsAllocated(void) const
{
   return false;
}

void GLSurface::Reset(void)
{
    // no idea what this is supposed to do
}

void GLSurface::AllocateSurface(int width, int height)
{
 
}
