#pragma once

#include "IRenderer.h"

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <SDKs/Volk/volk.h>

namespace Graphics
{
  ////////////////////////////////////////////////////////////////////
  class CRenderer : public IRenderer
  {
  public:
    ~CRenderer() { DeinitRenderer(); }

    bool InitRenderer();
    void DeinitRenderer();

  private:
    int64 m_width  = 0;
    int64 m_height = 0;

    // Vulkan related stuff
    VkInstance m_instance = VK_NULL_HANDLE;

    void _RegisterCVars();
    VkResult _CreateVulkanInstance();
  };
};