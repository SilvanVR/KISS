#pragma once

#include "IRenderer.h"

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace Graphics
{
  ////////////////////////////////////////////////////////////////////
  class CRenderer : public IRenderer
  {
  public:
    ~CRenderer() { DeinitRenderer(); }

    void InitRenderer();
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
    vk::Instance       m_vkInstance;
    vk::PhysicalDevice m_vkPhysicalDevice; 
    vk::Device         m_vkDevice;

    void _RegisterCVars();
    void _InitVulkan();
  };
};