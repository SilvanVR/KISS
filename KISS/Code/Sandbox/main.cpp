#include <stdafx.h>

#include "Graphics/IRenderer.h"

#ifdef CONSOLE_APP
int main(int argc, char** argv)
#else
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR cmdline, int cmdshow)
#endif
{
  static string s_name;
  static string s_appName;
  REGISTER_CVAR("engineName", &s_name, "KISSEngine", "Name of the engine");
  REGISTER_CVAR("appName", &s_appName, "NoName", "Name of the program");

  KISS_LOG_ALWAYS("Initializing Engine...");

  Graphics::IRenderer* pRenderer = Graphics::CreateRenderer(); 

  KISS_LOG_ALWAYS("Renderer initialized...");

  while (!pRenderer->ShouldClose())
  {
    pRenderer->BeginFrame();

    pRenderer->EndFrame();
  }

  // Restructuring

  // Input manager

  // Shaders (Colored Triangle)

  // Vulkan 

  // Optik Profiler

  // Inengine renderdoc (Press a button to do a capture)

  // ImGUI 

  // Multiview rendering / OpenXR

  // Jemalloc

  return 0;
}
