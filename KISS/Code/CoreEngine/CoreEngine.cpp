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
	_UpdateWindowTitle(fTimeStep);

  g_pIRenderer->BeginFrame();

	

  g_pIRenderer->EndFrame();
}

////////////////////////////////////////////////////////////////////
void CCoreEngine::_UpdateWindowTitle(f64 fTimeStep)
{
	static f64 nUpdateTimerCountSecs = 0.0f;
	static f64 nUpdateTimerPerSecondFrequency = (1.0 / 2.0f);
	static std::deque<f64> frameTimes; // Keep track of the frame times
	static const size_t numFrames = 30; // Number of frames to consider for the moving average
	static const f64 maxFrameTimeDeviation = 2.0; // Maximum deviation from the average frame time

	f64 fDeltaMS = 1000 * fTimeStep;
	f64 averageFrameTime = frameTimes.empty() ? 0.0 : std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0) / frameTimes.size();

	// Only add the new frame time if it's within the threshold
	if (frameTimes.empty() || std::abs(fDeltaMS - averageFrameTime) < maxFrameTimeDeviation * averageFrameTime)
	{
		frameTimes.push_back(fDeltaMS); // Add the current frame time to the deque
		if (frameTimes.size() > numFrames) // If we have more than numFrames frame times, remove the oldest one
			frameTimes.pop_front();
	}

	nUpdateTimerCountSecs += fTimeStep;
	if (nUpdateTimerCountSecs > nUpdateTimerPerSecondFrequency)
	{
		nUpdateTimerCountSecs -= nUpdateTimerPerSecondFrequency;

		averageFrameTime = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0) / frameTimes.size();
		char buf[256];
		sprintf_s(buf, "%.1fms (%d FPS)", averageFrameTime, int(1000.0 / averageFrameTime));
		g_pIRenderer->SetWindowTitle(buf);
	}
}