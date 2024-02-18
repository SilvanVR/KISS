#pragma once

#include "IRenderer.h"

#define GLFW_INCLUDE_VULKAN
#include <SDKs/glfw/glfw3.h>

namespace Graphics
{
  class CRenderer;
  static CRenderer* g_pRenderer = nullptr;

  ////////////////////////////////////////////////////////////////////
  class CRenderer : public IRenderer
  {
  public:
    ~CRenderer() { DeinitRenderer(); }

    void InitRenderer();
    void DeinitRenderer();

    void OnWindowSizeChanged(int nWidth, int nHeight);

    //////////////////////////
    // IRenderer Interface
    virtual bool ShouldClose() override;
    virtual void BeginFrame() override;
    virtual void EndFrame() override;
    virtual void SetWindowTitle(const char* pName) override;

    //////////////////////////
    // CVARS
    static int64 CV_r_width;
    static int64 CV_r_height;
    static int64 CV_r_vsync;
    static int64 CV_r_vulkanValidation;
    static int64 CV_r_renderdoc;
    static int64 CV_r_renderdocCapture;
    static string CV_r_renderdocInstallPath;

  private:
    bool   m_bVSync = true;
    uint32 m_nWidth  = 0;
    uint32 m_nHeight = 0;

    // GLFW
    GLFWwindow* m_pWindow = nullptr;

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

		vk::CommandBuffer& Cmd()   { return m_vkPerFrameCmd[m_nCurSwapchainIdx]; }
		vk::Image& CurBackbuffer() { return m_swapchainImages[m_nCurSwapchainIdx]; }
		vk::Fence& CurFence()      { return m_vkPerFrameFence[m_nCurSwapchainIdx]; }

    void _RegisterCVars();
    void _InitVulkan();
    void _CreateWindow();
    void _CreateSwapchain();
    void _CreateSwapchainImages();
		void _InitRenderdoc();
  };
};