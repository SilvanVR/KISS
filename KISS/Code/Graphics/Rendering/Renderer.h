#pragma once

#include "IRenderer.h"

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <SDKs/Volk/volk.h>

#define GLFW_INCLUDE_VULKAN
#include "glfw/glfw3.h"

namespace Graphics
{
  ////////////////////////////////////////////////////////////////////
  class CRenderer : public IRenderer
  {
  public:
    ~CRenderer() { DeinitRenderer(); }

    bool InitRenderer();
    void DeinitRenderer();

    //////////////////////////
    // IRenderer Interface
    virtual bool ShouldClose() override;
    virtual void BeginFrame() override;
    virtual void EndFrame() override;

    //////////////////////////
    // CVARS
    static int64 CV_r_width;
    static int64 CV_r_height;

  private:

    // GLFW
    GLFWwindow* m_pWindow;

    // Vulkan related members
    VkInstance m_instance = VK_NULL_HANDLE;

    void _RegisterCVars();
    VkResult _CreateVulkanInstance();
  };
};