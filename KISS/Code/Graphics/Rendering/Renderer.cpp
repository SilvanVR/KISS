#include "stdafx_common.h"

#include "Renderer.h"

#include "Utils/VkUtils.h"

#include <SDKs/vulkan/vulkan_extension_inspection.hpp>

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

    if (bool(CV_r_vsync) != m_bVSync)
    {
      m_bVSync = CV_r_vsync;
      // TODO: Recreate swapchain
    }

    vk::Semaphore imageAcquiredSemaphore = m_vkDevice.createSemaphore(vk::SemaphoreCreateInfo());
    vk::ResultValue<uint32_t> nexImage =
      m_vkDevice.acquireNextImage2KHR(vk::AcquireNextImageInfoKHR(m_vkSwapChain, UINT64_MAX, imageAcquiredSemaphore, {}, 1));
    assert(nexImage.result == vk::Result::eSuccess);
    m_nCurrentSwapchainBuffer = nexImage.value;

    m_vkPerFrameCmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)));
    
    VkUtils::SetImageLayout(m_vkPerFrameCmd, m_swapchainImages[m_nCurrentSwapchainBuffer], m_swapchainFormat,
      vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

    vk::ClearColorValue clearValue = vk::ClearColorValue(0.2f, 0.2f, 0.2f, 0.2f);
    m_vkPerFrameCmd.clearColorImage(
      m_swapchainImages[m_nCurrentSwapchainBuffer], 
      vk::ImageLayout::eColorAttachmentOptimal, 
      clearValue,
      vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS));

    VkUtils::SetImageLayout(m_vkPerFrameCmd, m_swapchainImages[m_nCurrentSwapchainBuffer], m_swapchainFormat,
      vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR);

    m_vkPerFrameCmd.end();

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo         submitInfo(imageAcquiredSemaphore, waitDestinationStageMask, m_vkPerFrameCmd);
    m_vkGraphicsQueue.submit(submitInfo);
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::EndFrame()
  {
    try
    {
      vk::Result result = m_vkPresentQueue.presentKHR(vk::PresentInfoKHR({}, m_vkSwapChain, m_nCurrentSwapchainBuffer));
      switch (result)
      {
      case vk::Result::eSuccess: break;
      case vk::Result::eSuboptimalKHR: KISS_LOG("vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !"); break;
      default: KISS_FATAL("Failed to present image");
      }
    }
    catch (vk::SystemError& err)
    {
      KISS_FATAL("vk::SystemError: %s ", err.what());
    }
    catch (std::exception& err)
    {
      KISS_FATAL("std::exception: %s ", err.what());
    }

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
    REGISTER_CVAR("r.vsync",  &CV_r_vsync,    1, "Toggles vsync of the rendering");
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
        std::vector<const char*> deviceExtensions{ 
          VK_KHR_SWAPCHAIN_EXTENSION_NAME,
          VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        };

        std::vector<vk::ExtensionProperties> extensionProperties = m_vkPhysicalDevice.enumerateDeviceExtensionProperties();
        for (const char* pDeviceExtension : deviceExtensions)
        {
          auto it = std::find_if(extensionProperties.begin(), extensionProperties.end(), [pDeviceExtension](const vk::ExtensionProperties& extensionsProps) {
            return string(pDeviceExtension) == extensionsProps.extensionName;
          });
          KISS_FATAL_COND(it != extensionProperties.end(), "Vulkan: Could not find device extension %s", pDeviceExtension);
        }

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

      // Queues
      {
        m_vkGraphicsQueue = m_vkDevice.getQueue(m_graphicsQueueFamilyIndex, 0);
        m_vkPresentQueue  = m_vkDevice.getQueue(m_presentQueueFamilyIndex, 0);
      }

      // Command buffer
      {
        m_vkCommandPool = m_vkDevice.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlags(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT), m_graphicsQueueFamilyIndex));
        m_vkPerFrameCmd = m_vkDevice.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_vkCommandPool, vk::CommandBufferLevel::ePrimary, 1)).front();
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
    m_swapchainFormat = (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;

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
      m_vkSurface, minImageCount, m_swapchainFormat,
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
    m_swapchainImages = m_vkDevice.getSwapchainImagesKHR(m_vkSwapChain);

    m_swapchainImageViews.resize(m_swapchainImages.size());
    vk::ImageViewCreateInfo imageViewCreateInfo({}, {}, vk::ImageViewType::e2D, m_swapchainFormat, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    for (const vk::Image& image : m_swapchainImages)
    {
      imageViewCreateInfo.image = image;
      m_swapchainImageViews.push_back(m_vkDevice.createImageView(imageViewCreateInfo));
    }
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::DeinitRenderer()
  {
    m_vkDevice.destroyCommandPool(m_vkCommandPool);
    for (const vk::ImageView& imageView : m_swapchainImageViews)
      m_vkDevice.destroyImageView(imageView);
    m_vkDevice.destroySwapchainKHR(m_vkSwapChain);
    m_vkInstance.destroySurfaceKHR(m_vkSurface);
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
    m_vkInstance.destroy();
  }

}