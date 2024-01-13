#include "stdafx_common.h"

#include "Renderer.h"

#include <vulkan/vulkan_extension_inspection.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Graphics
{
  ////////////////////////////////////////////////////////////////////
  int64 CRenderer::CV_r_width  = 0;
  int64 CRenderer::CV_r_height = 0;
  int64 CRenderer::CV_r_vsync  = 0;

  ////////////////////////////////////////////////////////////////////
  void GLFWErrorCallback(int code, const char* description)
  {
    KISS_LOG_ERROR("GLFW Error %d: %s", code, description);
  }

  ////////////////////////////////////////////////////////////////////
  IRenderer* CreateRenderer()
  {
    KISS_LOG_ALWAYS("Initializing Renderer...");
    CRenderer* pRenderer = new CRenderer;
    pRenderer->InitRenderer();
    return pRenderer;
  }

  ////////////////////////////////////////////////////////////////////
  bool CRenderer::ShouldClose()
  {
    return glfwWindowShouldClose(m_pWindow);
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::BeginFrame()
  {
    static f64 nUpdateTimerPerSecondFrequency = (1.0 / 2.0f);
    static f64 nUpdateTimerCountSecs = 0.0f;
    static f64 fLastTimeSecs = glfwGetTime();

    if (bool(CV_r_vsync) != m_bVSync)
    {
      m_bVSync = CV_r_vsync;
      // TODO: Recreate swapchain
    }

    f64 fCurTimeSecs = glfwGetTime();
    f64 fDeltaSecs = fCurTimeSecs - fLastTimeSecs;
    fLastTimeSecs = fCurTimeSecs;

    f64 fDeltaMS = 1000 * fDeltaSecs;
    nUpdateTimerCountSecs += fDeltaSecs;
    if (nUpdateTimerCountSecs > nUpdateTimerPerSecondFrequency)
    {
      // TODO: Average/Smooth out framerate
      nUpdateTimerCountSecs -= nUpdateTimerPerSecondFrequency;
      char buf[256];
      sprintf_s(buf, "%.1fms (%d FPS)", fDeltaMS, int(1000.0 / fDeltaMS));
      glfwSetWindowTitle(m_pWindow, buf);
    }
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::EndFrame()
  {
    glfwPollEvents();
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::InitRenderer()
  {
    _RegisterCVars();

    m_bVSync = bool(CV_r_vsync);
    m_width  = uint32(CV_r_width);
    m_height = uint32(CV_r_height);

    bool bSuccess = glfwInit();
    KISS_FATAL_COND(bSuccess, "Failed to initialize glfw");

    glfwSetErrorCallback(GLFWErrorCallback); 

    assert(glfwVulkanSupported() == GLFW_TRUE);

    _CreateWindow();
    _InitVulkan();

    // Borderless fullscreen
    //const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    //glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::_RegisterCVars()
  {
    REGISTER_CVAR("r.width",  &CV_r_width, 1280, "Width of the rendering window");
    REGISTER_CVAR("r.height", &CV_r_height, 720, "Height of the rendering window");
    REGISTER_CVAR("r.vsync",  &CV_r_vsync,  1,   "Toggles vsync of the rendering");
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::_CreateWindow()
  {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_DOUBLEBUFFER, m_bVSync ? GLFW_TRUE : GLFW_FALSE);
    CVar* pAppName = CConsole::Instance().GetCVar("appName");
    m_pWindow = glfwCreateWindow((int)CV_r_width, (int)CV_r_height, pAppName->GetString().c_str(), NULL, NULL);
    KISS_FATAL_COND(m_pWindow, "Failed to create Window");
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::_InitVulkan()
  {
    VULKAN_HPP_DEFAULT_DISPATCHER.init();

    try
    {
      // Instance
      {
        CVar* pAppName    = CConsole::Instance().GetCVar("appName");
        CVar* pEngineName = CConsole::Instance().GetCVar("engineName");
        vk::ApplicationInfo applicationInfo(
          pAppName    ? pAppName->GetString().c_str()     : "NoName", 1, 
          pEngineName ? pEngineName->GetString().c_str() : "NoEngineName", 
          1, VK_API_VERSION_1_3
        );
        
        std::vector<char const*> arrLayers;
        // arrLayers.push_back( "VK_LAYER_KHRONOS_validation" );
        std::vector<char const*> arrExtensions = {
          VK_KHR_SURFACE_EXTENSION_NAME,
  #ifdef _WIN32
          VK_KHR_WIN32_SURFACE_EXTENSION_NAME
  #endif
        };
        vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, arrLayers, arrExtensions);
        m_vkInstance = vk::createInstance(instanceCreateInfo);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_vkInstance);
      }

      // Physical Device
      {
        std::vector<vk::PhysicalDevice> physicalDevices = m_vkInstance.enumeratePhysicalDevices();
        for (uint32 i = 0; i < physicalDevices.size(); i++)
        {
          const vk::PhysicalDevice& physicalDevice = physicalDevices[i];
          vk::PhysicalDeviceProperties props = physicalDevice.getProperties();
          KISS_LOG_ALWAYS("[%u] GPU found: %s", i, props.deviceName);
        }

        KISS_FATAL_COND(!physicalDevices.empty(), "No GPU found on your system. Please PLUG IN A FUCKING GPU!!!111!!");

        m_vkPhysicalDevice = physicalDevices.front();
        KISS_LOG_ALWAYS("Using GPU: %s", m_vkPhysicalDevice.getProperties().deviceName);
      }

      // Logical Device
      std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_vkPhysicalDevice.getQueueFamilyProperties();
      {
        auto propertyIterator = std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(),
          [](vk::QueueFamilyProperties const& qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });
        m_graphicsQueueFamilyIndex = (uint32)std::distance(queueFamilyProperties.begin(), propertyIterator);
        assert(m_graphicsQueueFamilyIndex < queueFamilyProperties.size());

        std::vector<float> queuePriorities = { 0.0f };
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), m_graphicsQueueFamilyIndex, queuePriorities);

        std::vector<const char*> layerNames{};
        std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        m_vkDevice = m_vkPhysicalDevice.createDevice(vk::DeviceCreateInfo(vk::DeviceCreateFlags(), deviceQueueCreateInfo, layerNames, deviceExtensions));
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_vkDevice);
      }

      // Surface
      {
        VkSurfaceKHR _surface;
        glfwCreateWindowSurface(static_cast<VkInstance>(m_vkInstance), m_pWindow, nullptr, &_surface);
        m_vkSurface = vk::SurfaceKHR(_surface);
      
        auto propertyIterator = std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(),
          [&](vk::QueueFamilyProperties const& qfp) { return m_vkPhysicalDevice.getSurfaceSupportKHR(m_graphicsQueueFamilyIndex, m_vkSurface); });
        m_presentQueueFamilyIndex = (uint32)std::distance(queueFamilyProperties.begin(), propertyIterator);
      }

      _CreateSwapchain();
      _CreateSwapchainImages();
    }
    catch (vk::SystemError& err)
    {
      KISS_FATAL("vk::SystemError: %s ", err.what());
    }
    catch (std::exception& err)
    {
      KISS_FATAL("std::exception: %s ", err.what());
    }
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::_CreateSwapchain()
  {
    // Gget the supported VkFormats
    std::vector<vk::SurfaceFormatKHR> formats = m_vkPhysicalDevice.getSurfaceFormatsKHR(m_vkSurface);
    assert(!formats.empty());
    m_vkSwapchainFormat = (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;

    vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_vkPhysicalDevice.getSurfaceCapabilitiesKHR(m_vkSurface);
    vk::Extent2D               swapchainExtent;
    if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
    {
      // If the surface size is undefined, the size is set to the size of the images requested.
      swapchainExtent.width  = Utils::Clamp(m_width,  surfaceCapabilities.minImageExtent.width,  surfaceCapabilities.maxImageExtent.width);
      swapchainExtent.height = Utils::Clamp(m_height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }
    else
    {
      // If the surface size is defined, the swap chain size must match
      swapchainExtent = surfaceCapabilities.currentExtent;
    }

    // The FIFO present mode is guaranteed by the spec to be supported
    vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;
    const std::vector<vk::PresentModeKHR>& presentModes = m_vkPhysicalDevice.getSurfacePresentModesKHR(m_vkSurface);
    for (const auto& presentMode : presentModes)
    {
      if (!CV_r_vsync && presentMode == vk::PresentModeKHR::eImmediate)
      {
        swapchainPresentMode = presentMode;
        break;
      }
      else if (presentMode == vk::PresentModeKHR::eMailbox)
      {
        swapchainPresentMode = presentMode;
        break;
      }
    }

    vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
      ? vk::SurfaceTransformFlagBitsKHR::eIdentity : surfaceCapabilities.currentTransform;

    vk::CompositeAlphaFlagBitsKHR compositeAlpha =
      (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
      : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
      : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) ? vk::CompositeAlphaFlagBitsKHR::eInherit
      : vk::CompositeAlphaFlagBitsKHR::eOpaque;

    // Try to use tripple buffering
    uint32 minImageCount = Utils::Clamp(3u, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
    vk::SwapchainCreateInfoKHR swapChainCreateInfo(vk::SwapchainCreateFlagsKHR(),
      m_vkSurface, minImageCount, m_vkSwapchainFormat,
      vk::ColorSpaceKHR::eSrgbNonlinear,
      swapchainExtent, 1,
      vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eExclusive,
      {}, preTransform, compositeAlpha,
      swapchainPresentMode, true);

    uint32_t queueFamilyIndices[2] = { m_graphicsQueueFamilyIndex, m_presentQueueFamilyIndex };
    if (m_graphicsQueueFamilyIndex != m_presentQueueFamilyIndex)
    {
      // If the graphics and present queues are from different queue families, we either have to explicitly transfer
      // ownership of images between the queues, or we have to create the swapchain with imageSharingMode as
      // VK_SHARING_MODE_CONCURRENT
      swapChainCreateInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
      swapChainCreateInfo.queueFamilyIndexCount = 2;
      swapChainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
      KISS_LOG_ALWAYS("Graphics queue does not support present. Using a separate queue family for presenting instead.");
    }

    m_vkSwapChain = m_vkDevice.createSwapchainKHR(swapChainCreateInfo);
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::_CreateSwapchainImages()
  {
    std::vector<vk::Image> swapChainImages = m_vkDevice.getSwapchainImagesKHR(m_vkSwapChain);

    m_swapchainImageViews.reserve(swapChainImages.size());
    vk::ImageViewCreateInfo imageViewCreateInfo({}, {}, vk::ImageViewType::e2D, m_vkSwapchainFormat, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    for (const vk::Image& image : swapChainImages)
    {
      imageViewCreateInfo.image = image;
      m_swapchainImageViews.push_back(m_vkDevice.createImageView(imageViewCreateInfo));
    }
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::DeinitRenderer()
  {
    m_vkDevice.destroySwapchainKHR(m_vkSwapChain);
    m_vkInstance.destroySurfaceKHR(m_vkSurface);
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
    m_vkInstance.destroy();
  }
}