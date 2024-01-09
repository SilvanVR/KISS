#include "stdafx_common.h"

#include "Renderer.h"

namespace Graphics
{
  ////////////////////////////////////////////////////////////////////
  int64 CRenderer::CV_r_width;
  int64 CRenderer::CV_r_height;

  ////////////////////////////////////////////////////////////////////
  void GLFWErrorCallback(int code, const char* description)
  {
    LOG_ERROR("GLFW Error %d: %s", code, description);
  }

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
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::EndFrame()
  {
    glfwSwapBuffers(m_pWindow);
    glfwPollEvents();
  }

  ////////////////////////////////////////////////////////////////////
  bool CRenderer::InitRenderer()
  {
    _RegisterCVars();

    if (!glfwInit())
    {
      LOG_ERROR("Failed to initialize glfw");
      return false;
    }

    glfwSetErrorCallback(GLFWErrorCallback); 

    int nVal = glfwVulkanSupported();
    assert(glfwVulkanSupported() == GLFW_TRUE);

    VkResult result = volkInitialize();
    if (result != VK_SUCCESS)
    {
      LOG_ERROR("Failed to initialize Volk. Please install the vulkan loader.");
      assert(false);
      return false;
    }

    result = _CreateVulkanInstance();
    volkLoadInstance(m_instance);

    CVar* pAppName = CConsole::Instance().GetCVar("appName");
    m_pWindow = glfwCreateWindow((int)CV_r_width, (int)CV_r_height, pAppName->GetString().c_str(), NULL, NULL);
    if (!m_pWindow)
    {
      LOG_ERROR("Failed to create Window");
      return false;
    }

    // Borderless fullscreen
    //const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    //glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

    static f64 nUpdateTimerPerSecondFrequency = (1.0 / 2.0f);
    static f64 nUpdateTimerCountSecs = 0.0f;

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
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
    vkDestroyInstance(m_instance, nullptr);
  }
}