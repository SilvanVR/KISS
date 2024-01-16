#pragma once

#include "IRenderer.h"

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define GLFW_INCLUDE_VULKAN
#include <SDKs/glfw/glfw3.h>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <SDKs/vulkan/vulkan.hpp>
#undef max
#undef min

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
    static int64 CV_r_vulkanValidation;

  private:
    bool   m_bVSync = true;
    uint32 m_width  = 0;
    uint32 m_height = 0;

    // GLFW
    GLFWwindow* m_pWindow;

    // Vulkan related members
    vk::Instance       m_vkInstance;
    vk::PhysicalDevice m_vkPhysicalDevice; 
    vk::Device         m_vkDevice;
    vk::SurfaceKHR     m_vkSurface;
    vk::SwapchainKHR   m_vkSwapChain;
    vk::CommandPool    m_vkCommandPool;
    vk::Queue          m_vkGraphicsQueue;
    vk::Queue          m_vkPresentQueue;
    std::vector<vk::CommandBuffer> m_vkPerFrameCmd;
    std::vector<vk::Fence>         m_vkPerFrameFence;

    vk::Semaphore m_imageAcquiredSemaphore;
    vk::Semaphore m_renderCompletedSemaphore;

    vk::DebugUtilsMessengerEXT m_debugUtilsMessenger;

    uint32 m_nCurSwapchainIdx = 0;
    uint32 m_graphicsQueueFamilyIndex = 0;
    uint32 m_presentQueueFamilyIndex  = 0;

    vk::Format                   m_swapchainFormat = vk::Format::eUndefined;
    std::vector<vk::Image>       m_swapchainImages;
    std::vector<vk::ImageView>   m_swapchainImageViews;
    std::vector<vk::Framebuffer> m_framebuffers;

    void _RegisterCVars();
    void _InitVulkan();
    void _CreateWindow();
    void _CreateSwapchain();
    void _CreateSwapchainImages();
  };
};