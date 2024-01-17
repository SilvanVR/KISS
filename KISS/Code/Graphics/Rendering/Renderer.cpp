#include "stdafx_common.h"

#include "Renderer.h"

#include "Utils/VkUtils.h"

#include <SDKs/vulkan/vulkan_extension_inspection.hpp>

#include <sstream>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Graphics
{
  ////////////////////////////////////////////////////////////////////
  int64 CRenderer::CV_r_width            = 0;
  int64 CRenderer::CV_r_height           = 0;
  int64 CRenderer::CV_r_vsync            = 0;
  int64 CRenderer::CV_r_vulkanValidation = 0;

  ////////////////////////////////////////////////////////////////////
  void GLFWErrorCallback(int code, const char* description)
  {
    KISS_LOG_ERROR("GLFW Error %d: %s", code, description);
  }
  
  ////////////////////////////////////////////////////////////////////
  void FramebufferSizeCallback(GLFWwindow* window, int width, int height) 
  {
    g_pRenderer->OnWindowSizeChanged(width, height);
  }

  ////////////////////////////////////////////////////////////////////
  void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE); // Close the window when Escape is pressed
    }
    else {
      std::cout << "Key: " << key << " Action: " << action << std::endl;
    }
  }

  ////////////////////////////////////////////////////////////////////
  IRenderer* CreateRenderer()
  {
    KISS_LOG_ALWAYS("Initializing Renderer...");
    g_pRenderer = new CRenderer;
    g_pRenderer->InitRenderer();
    return g_pRenderer;
  }

  ////////////////////////////////////////////////////////////////////
  VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
    void* /*pUserData*/)
  {
    std::stringstream message;

    vk::DebugUtilsMessageSeverityFlagBitsEXT eVerbosity = static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity);
    message << vk::to_string(eVerbosity) << ": " << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) << ":\n";
    message << std::string("\t") << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
    message << std::string("\t") << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
    message << std::string("\t") << "message         = <" << pCallbackData->pMessage << ">\n";
    if (0 < pCallbackData->queueLabelCount)
    {
      message << std::string("\t") << "Queue Labels:\n";
      for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++)
      {
        message << std::string("\t\t") << "labelName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
      }
    }
    if (0 < pCallbackData->cmdBufLabelCount)
    {
      message << std::string("\t") << "CommandBuffer Labels:\n";
      for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++)
      {
        message << std::string("\t\t") << "labelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
      }
    }
    if (0 < pCallbackData->objectCount)
    {
      message << std::string("\t") << "Objects:\n";
      for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
      {
        message << std::string("\t\t") << "Object " << i << "\n";
        message << std::string("\t\t\t") << "objectType   = " << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) << "\n";
        message << std::string("\t\t\t") << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
        if (pCallbackData->pObjects[i].pObjectName)
        {
          message << std::string("\t\t\t") << "objectName   = <" << pCallbackData->pObjects[i].pObjectName << ">\n";
        }
      }
    }

    switch (eVerbosity)
    {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:    KISS_LOG(message.str().c_str()); break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose: KISS_LOG(message.str().c_str()); break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning: KISS_LOG_WARN(message.str().c_str()); break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:   KISS_LOG_ERROR(message.str().c_str()); break;
    }

    return false;
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::OnWindowSizeChanged(int nWidth, int nHeight)
  {
    m_bVSync    = CV_r_vsync;
    m_nWidth    = nWidth;
    m_nHeight   = nHeight;
    CV_r_width  = int64(nWidth);
    CV_r_height = int64(nHeight);
    if (nWidth && nHeight)
    {
      _CreateSwapchain();
      _CreateSwapchainImages();
      KISS_LOG("Window resized. New width/height: %d, %d", nWidth, nHeight);
    }
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

    glfwPollEvents();

    if (bool(CV_r_vsync) != m_bVSync || m_nWidth != CV_r_width || m_nHeight != CV_r_height)
      OnWindowSizeChanged(int(CV_r_width), int(CV_r_height));

    if (m_nWidth == 0 || m_nHeight == 0)
      return;

    vk::ResultValue<uint32_t> nexImage =
      m_vkDevice.acquireNextImage2KHR(vk::AcquireNextImageInfoKHR(m_vkSwapChain, UINT64_MAX, m_imageAcquiredSemaphore, {}, 1));
    assert(nexImage.result == vk::Result::eSuccess);
    m_nCurSwapchainIdx = nexImage.value;

    while (vk::Result::eTimeout == m_vkDevice.waitForFences(m_vkPerFrameFence[m_nCurSwapchainIdx], VK_TRUE, UINT64_MAX));
    m_vkDevice.resetFences(m_vkPerFrameFence[m_nCurSwapchainIdx]);

    m_vkPerFrameCmd[m_nCurSwapchainIdx].begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)));
    
    VkUtils::SetImageLayout(m_vkPerFrameCmd[m_nCurSwapchainIdx], m_swapchainImages[m_nCurSwapchainIdx], m_swapchainFormat,
      vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

    //vk::ClearColorValue clearValue = vk::ClearColorValue(0.2f, 0.2f, 0.2f, 0.2f);
    //m_vkPerFrameCmd[m_nCurSwapchainIdx]..clearColorImage(
    //  m_swapchainImages[m_nCurSwapchainIdx], 
    //  vk::ImageLayout::eColorAttachmentOptimal, 
    //  clearValue,
    //  vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS));

    VkUtils::SetImageLayout(m_vkPerFrameCmd[m_nCurSwapchainIdx], m_swapchainImages[m_nCurSwapchainIdx], m_swapchainFormat,
      vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR);

    m_vkPerFrameCmd[m_nCurSwapchainIdx].end();

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo         submitInfo(m_imageAcquiredSemaphore, waitDestinationStageMask, m_vkPerFrameCmd[m_nCurSwapchainIdx], m_renderCompletedSemaphore);
    m_vkGraphicsQueue.submit(submitInfo, m_vkPerFrameFence[m_nCurSwapchainIdx]);
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::EndFrame()
  {
    if (m_nWidth == 0 || m_nHeight == 0)
      return;

    try
    {
      vk::Result result = m_vkPresentQueue.presentKHR(vk::PresentInfoKHR(m_renderCompletedSemaphore, m_vkSwapChain, m_nCurSwapchainIdx));
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
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::InitRenderer()
  {
    _RegisterCVars();

    m_bVSync = bool(CV_r_vsync);
    m_nWidth  = uint32(CV_r_width);
    m_nHeight = uint32(CV_r_height);

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
    REGISTER_CVAR("r.width",             &CV_r_width,           1280, "Width of the rendering window");
    REGISTER_CVAR("r.height",            &CV_r_height,           720, "Height of the rendering window");
    REGISTER_CVAR("r.vsync",             &CV_r_vsync,              1, "Toggles vsync of the rendering");
    REGISTER_CVAR("r.vulkan.validation", &CV_r_vulkanValidation,   0, 
      "0: Validation off\n"
      "1: Info+Warnings+Errors\n"
      "2: Warnings+Errors\n"
      "3: Errors\n"
    );
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::_CreateWindow()
  {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    CVar* pAppName = CConsole::Instance().GetCVar("appName");
    m_pWindow = glfwCreateWindow((int)CV_r_width, (int)CV_r_height, pAppName->GetString().c_str(), NULL, NULL);
    KISS_FATAL_COND(m_pWindow, "Failed to create Window");
    glfwSetFramebufferSizeCallback(m_pWindow, FramebufferSizeCallback);
    glfwSetKeyCallback(m_pWindow, KeyCallback);
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
        if (CV_r_vulkanValidation)
          arrLayers.push_back("VK_LAYER_KHRONOS_validation");
        std::vector<char const*> arrExtensions;
        arrExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        if (CV_r_vulkanValidation)
          arrExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  #ifdef _WIN32
        arrExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
  #endif
        vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, arrLayers, arrExtensions);
        m_vkInstance = vk::createInstance(instanceCreateInfo);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_vkInstance);

        if (CV_r_vulkanValidation)
        {
          vk::DebugUtilsMessageSeverityFlagsEXT severityFlags = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
          //severityFlags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
          if (CV_r_vulkanValidation <= 2) severityFlags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
          if (CV_r_vulkanValidation <= 1) severityFlags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
          vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | 
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
          m_debugUtilsMessenger = m_vkInstance.createDebugUtilsMessengerEXT(
            vk::DebugUtilsMessengerCreateInfoEXT({}, severityFlags, messageTypeFlags, &debugMessageFunc));
        }
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

      _CreateSwapchain();
      _CreateSwapchainImages();

      // Command buffer
      {
        assert(m_swapchainImages.size());
        m_vkCommandPool = m_vkDevice.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlags(
          VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT), m_graphicsQueueFamilyIndex));
        m_vkPerFrameCmd = m_vkDevice.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_vkCommandPool, vk::CommandBufferLevel::ePrimary, (uint32)m_swapchainImages.size()));

        for (size_t i = 0; i < m_swapchainImages.size(); i++)
          m_vkPerFrameFence.push_back(m_vkDevice.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
      }

      // Semaphores
      {
        m_imageAcquiredSemaphore   = m_vkDevice.createSemaphore(vk::SemaphoreCreateInfo());
        m_renderCompletedSemaphore = m_vkDevice.createSemaphore(vk::SemaphoreCreateInfo());
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
      swapchainExtent.width  = Utils::Clamp(m_nWidth,  surfaceCapabilities.minImageExtent.width,  surfaceCapabilities.maxImageExtent.width);
      swapchainExtent.height = Utils::Clamp(m_nHeight, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }
    else
    {
      // If the surface size is defined, the swap chain size must match
      swapchainExtent = surfaceCapabilities.currentExtent;
    }

    // The FIFO present mode is guaranteed by the spec to be supported
    vk::PresentModeKHR swapchainPresentMode = CV_r_vsync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate;
    const std::vector<vk::PresentModeKHR>& presentModes = m_vkPhysicalDevice.getSurfacePresentModesKHR(m_vkSurface);
    // vk::PresentModeKHR::eMailbox

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
    if (m_vkSwapChain)
      m_vkDevice.destroySwapchainKHR(m_vkSwapChain);
    m_vkSwapChain = m_vkDevice.createSwapchainKHR(swapChainCreateInfo);
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::_CreateSwapchainImages()
  {
    m_swapchainImages = m_vkDevice.getSwapchainImagesKHR(m_vkSwapChain);

    m_swapchainImageViews.resize(m_swapchainImages.size());
    vk::ImageViewCreateInfo imageViewCreateInfo({}, {}, vk::ImageViewType::e2D, m_swapchainFormat, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    for (uint32 i = 0; i < m_swapchainImages.size(); i++)
    {
      imageViewCreateInfo.image = m_swapchainImages[i];
      if (m_swapchainImageViews[i])
        m_vkDevice.destroyImageView(m_swapchainImageViews[i]);
      m_swapchainImageViews[i] = m_vkDevice.createImageView(imageViewCreateInfo);
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
    m_vkInstance.destroyDebugUtilsMessengerEXT(m_debugUtilsMessenger);
    m_vkInstance.destroy();
  }

}