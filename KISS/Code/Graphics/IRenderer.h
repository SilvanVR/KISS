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
  };

  IRenderer* CreateRenderer();
};