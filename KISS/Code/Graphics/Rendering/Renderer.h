#pragma once

#include "IRenderer.h"

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <SDKs/Volk/volk.h>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <vulkan/vulkan.hpp>

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
    static int64 CV_r_vsync;

  private:
    bool m_bVSync = true;

    // GLFW
    GLFWwindow* m_pWindow;

    // Vulkan related members
    vk::Instance m_vkInstance;

    void _RegisterCVars();
    void _CreateVulkanInstance();
  };
};