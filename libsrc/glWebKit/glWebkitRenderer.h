
#pragma once

#include <EAWebKit\EAWebKitSurface.h>
#include <EAWebKit\EAWebKitHardwareRenderer.h>

#include <cstdint>

class GLSurface : public EA::WebKit::ISurface
{
public:
   unsigned int tex = 0;
public:
   GLSurface();
   virtual ~GLSurface();
   virtual void Lock(SurfaceDescriptor *pSDOut, const EA::WebKit::IntRect *rect = NULL) override;
   virtual void Unlock(void) override;
   virtual void Release(void) override;
   virtual bool IsAllocated(void) const override;
   virtual void Reset(void) override;
protected:
   virtual void AllocateSurface(int width, int height) override;
};

class GLRenderer : public EA::WebKit::IHardwareRenderer
{
public:
   GLRenderer();
   virtual ~GLRenderer();
   virtual EA::WebKit::ISurface * CreateSurface(EA::WebKit::SurfaceType surfaceType, const void* data = 0, size_t length = 0) override;
   virtual void SetRenderTarget(EA::WebKit::ISurface *target) override;
   virtual void RenderSurface(EA::WebKit::ISurface *surface, EA::WebKit::FloatRect &target, EA::WebKit::TransformationMatrix &matrix, float opacity, EA::WebKit::CompositOperator op, EA::WebKit::TextureWrapMode wrap, EA::WebKit::Filters &filters) override;
   virtual void FillColor(uint32_t premultiplied_rgba32, EA::WebKit::FloatRect &target, EA::WebKit::TransformationMatrix &matrix, EA::WebKit::CompositOperator op) override;
   virtual void DrawOutline(uint32_t premultiplied_rgba32, EA::WebKit::FloatRect &target, EA::WebKit::TransformationMatrix &matrix) override;
   virtual int32_t MaxTextureSize(void) override;
   virtual void Clear(EA::WebKit::ClearFlags flags, uint32_t premultiplied_rgba32, float z, uint32_t stencil) override;
   virtual void ScissorClip(EA::WebKit::IntRect axisAlignedRect) override;
   virtual void DrawStencil(EA::WebKit::TransformationMatrix &matrix, EA::WebKit::FloatRect &target, uint32_t stencilIndex) override;
   virtual void ClipAgainstStencil(uint32_t stencilIndex) override;
   virtual bool UseCustomClip() override;
   virtual void BeginClip(EA::WebKit::TransformationMatrix &matrix, EA::WebKit::FloatRect &target) override;
   virtual void EndClip(void) override;
   virtual EA::WebKit::IntRect CurrentClipBound() override;
   virtual void BeginPainting(void) override;
   virtual void EndPainting(void)  override;
};