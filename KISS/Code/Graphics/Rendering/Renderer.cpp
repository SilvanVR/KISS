#include "stdafx_common.h"

#include "Renderer.h"

namespace Graphics
{
  ////////////////////////////////////////////////////////////////////
  int64 CRenderer::CV_r_width;
  int64 CRenderer::CV_r_height;

  ////////////////////////////////////////////////////////////////////
  IRenderer* CreateRenderer()
  {
    LOG("Initializing Renderer...");
    CRenderer* pRenderer = new CRenderer;
    bool bSuccess = pRenderer->InitRenderer();
    if (!bSuccess)
    {
      LOG_ERROR("Failed to create renderer");
      return nullptr;
    }
    return pRenderer;
  }

  ////////////////////////////////////////////////////////////////////
  bool CRenderer::InitRenderer()
  {
    _RegisterCVars();

    VkResult result = volkInitialize();
    if (result != VK_SUCCESS)
    {
      LOG_ERROR("Failed to initialize Volk. Please install the vulkan loader.");
      assert(false);
      return false;
    }

    result = _CreateVulkanInstance();
    volkLoadInstance(m_instance);

    return true;
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::_RegisterCVars()
  {
    REGISTER_CVAR("r.width",  &CV_r_width, 1280, "Width of the rendering window");
    REGISTER_CVAR("r.height", &CV_r_height, 720, "Height of the rendering window");
  }

  ////////////////////////////////////////////////////////////////////
  VkResult CRenderer::_CreateVulkanInstance()
  {
    // Application information
    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "KISS";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "KissEngine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    LOG("Available Vulkan extensions:");
    for (const auto& extension : extensions) {
      LOG(extension.extensionName);
    }

    const char* requiredExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
    #ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    #endif
    };
    createInfo.enabledExtensionCount = sizeof(requiredExtensions) / sizeof(requiredExtensions[0]);
    createInfo.ppEnabledExtensionNames = requiredExtensions;

    return vkCreateInstance(&createInfo, nullptr, &m_instance);
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::DeinitRenderer()
  {
    vkDestroyInstance(m_instance, nullptr);
  }
}