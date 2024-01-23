#include "stdafx_common.h"

#include "CoreEngine.h"

#include "Graphics/IRenderer.h"
#include "IVirtualFileSystem.h"

#include <SDKs/glfw/glfw3.h>

////////////////////////////////////////////////////////////////////
void GLFWErrorCallback(int code, const char* description)
{
  KISS_LOG_ERROR("GLFW Error %d: %s", code, description);
}

string CCoreEngine::CV_e_engineName;
string CCoreEngine::CV_e_appName;

////////////////////////////////////////////////////////////////////
void CCoreEngine::InitEngine()
{
  VFS::CreateVirtualFileSystem();
  CreateConsole();
  KISS_LOG_ALWAYS("Virtual File System initialized...");
  KISS_LOG_ALWAYS("Console initialized...");

  _RegisterCVars();

  KISS_LOG_ALWAYS("Initializing Engine...");

  bool bSuccess = glfwInit();
  KISS_FATAL_COND(bSuccess, "Failed to initialize glfw");

  glfwSetErrorCallback(GLFWErrorCallback);

  Graphics::CreateRenderer();
  KISS_LOG_ALWAYS("Renderer initialized...");
}

////////////////////////////////////////////////////////////////////
void CCoreEngine::_RegisterCVars()
{
  REGISTER_CVAR("engineName", &CV_e_engineName, "KISSEngine", "Name of the engine");
  REGISTER_CVAR("appName",    &CV_e_appName,    "NoName",     "Name of the program");
}

////////////////////////////////////////////////////////////////////
void CCoreEngine::Run()
{
  static f64 fLastTimeSecs = glfwGetTime();

  while (!g_pIRenderer->ShouldClose())
  {
    glfwPollEvents();

    f64 fCurTimeSecs = glfwGetTime();
    f64 fDeltaSecs = fCurTimeSecs - fLastTimeSecs;
    fLastTimeSecs = fCurTimeSecs;
    Heartbeat(fDeltaSecs);
  }
}

////////////////////////////////////////////////////////////////////
void CCoreEngine::Heartbeat(f64 fTimeStep)
{
  static f64 nUpdateTimerCountSecs = 0.0f;
  static f64 nUpdateTimerPerSecondFrequency = (1.0 / 2.0f);
  nUpdateTimerCountSecs += fTimeStep;
  if (nUpdateTimerCountSecs > nUpdateTimerPerSecondFrequency)
  {
    // TODO: Average/Smooth out framerate
    nUpdateTimerCountSecs -= nUpdateTimerPerSecondFrequency;
    f64 fDeltaMS = 1000 * fTimeStep;
    char buf[256];
    sprintf_s(buf, "%.1fms (%d FPS)", fDeltaMS, int(1000.0 / fDeltaMS));
    g_pIRenderer->SetWindowTitle(buf);
  }
   
  g_pIRenderer->BeginFrame();

 

  g_pIRenderer->EndFrame();
}