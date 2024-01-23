#pragma once

namespace Graphics
{
  ////////////////////////////////////////////////////////////////////
  class IRenderer
  {
  public:
    virtual ~IRenderer() {}

    virtual bool ShouldClose() = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void SetWindowTitle(const char* pName) = 0;
  };

  void CreateRenderer();
};

extern Graphics::IRenderer* g_pIRenderer;