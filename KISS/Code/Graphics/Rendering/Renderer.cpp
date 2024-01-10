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

    _CreateVulkanInstance();
    volkLoadInstance(m_vkInstance);

    m_bVSync = bool(CV_r_vsync);
    glfwWindowHint(GLFW_DOUBLEBUFFER, m_bVSync ? GLFW_TRUE : GLFW_FALSE);

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

    return true;
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::_RegisterCVars()
  {
    REGISTER_CVAR("r.width",  &CV_r_width, 1280, "Width of the rendering window");
    REGISTER_CVAR("r.height", &CV_r_height, 720, "Height of the rendering window");
    REGISTER_CVAR("r.vsync",  &CV_r_vsync,  1,   "Toggles vsync of the rendering");
  }

  ////////////////////////////////////////////////////////////////////
  void CRenderer::_CreateVulkanInstance()
  {
    VULKAN_HPP_DEFAULT_DISPATCHER.init();

    try
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
    }
    catch (vk::SystemError& err)
    {
      LOG_ERROR("vk::SystemError: %s ", err.what());
      exit(-1);
    }
    catch (std::exception& err)
    {
      LOG_ERROR("std::exception: %s ", err.what());
      exit(-1);
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