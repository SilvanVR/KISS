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
    LOG_ERROR("GLFW Error %d: %s", code, description);
  }

  ////////////////////////////////////////////////////////////////////
  IRenderer* CreateRenderer()
  {
    LOG("Initializing Renderer...");
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
    glfwSwapBuffers(m_pWindow);
    glfwPollEvents();
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::InitRenderer()
  {
    _RegisterCVars();

    m_bVSync = bool(CV_r_vsync);

    bool bSuccess = glfwInit();
    KISS_FATAL_COND(!bSuccess, "Failed to initialize glfw");

    glfwSetErrorCallback(GLFWErrorCallback); 

    int nVal = glfwVulkanSupported();
    assert(glfwVulkanSupported() == GLFW_TRUE);

    _InitVulkan();

    glfwWindowHint(GLFW_DOUBLEBUFFER, m_bVSync ? GLFW_TRUE : GLFW_FALSE);

    CVar* pAppName = CConsole::Instance().GetCVar("appName");
    m_pWindow = glfwCreateWindow((int)CV_r_width, (int)CV_r_height, pAppName->GetString().c_str(), NULL, NULL);
    KISS_FATAL_COND(m_pWindow, "Failed to create Window");

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

        const char* arrExtensions[] = {
          VK_KHR_SURFACE_EXTENSION_NAME,
  #ifdef _WIN32
          VK_KHR_WIN32_SURFACE_EXTENSION_NAME
  #endif
        };
        vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, {}, arrExtensions);
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
          LOG("[%u] GPU found: %s", i, props.deviceName);
        }

        KISS_FATAL_COND(physicalDevices.empty(), "No GPU found on your system. Please PLUG IN A FUCKING GPU!!!111!!");

        m_vkPhysicalDevice = physicalDevices.front();
        LOG("Using GPU: %s", m_vkPhysicalDevice.getProperties().deviceName);
      }

      // Logical Device
      {
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_vkPhysicalDevice.getQueueFamilyProperties();

        auto propertyIterator = std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(),
          [](vk::QueueFamilyProperties const& qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });
        size_t graphicsQueueFamilyIndex = std::distance(queueFamilyProperties.begin(), propertyIterator);
        assert(graphicsQueueFamilyIndex < queueFamilyProperties.size());

        float queuePriority = 0.0f;
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), static_cast<uint32_t>(graphicsQueueFamilyIndex), 1, &queuePriority);
        m_vkDevice = m_vkPhysicalDevice.createDevice(vk::DeviceCreateInfo(vk::DeviceCreateFlags(), deviceQueueCreateInfo));
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_vkDevice);
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
  void CRenderer::DeinitRenderer()
  {
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
    m_vkInstance.destroy();
  }
}