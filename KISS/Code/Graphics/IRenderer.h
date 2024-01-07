#pragma once

namespace Graphics
{
  ////////////////////////////////////////////////////////////////////
  class IRenderer
  {
  public:
    virtual ~IRenderer() {}
  };

  IRenderer* CreateRenderer();
};