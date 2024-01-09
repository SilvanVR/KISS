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
  REGISTER_CVAR("e.engineName", &s_name, "KISSEngine", "Name of the engine");
  REGISTER_CVAR("appName", &s_appName, "NoName", "Name of the program");
   
  LOG("Initializing Engine...");

  Graphics::IRenderer* pRenderer = Graphics::CreateRenderer(); 

  LOG("Renderer initialized..."); 

  while (!pRenderer->ShouldClose())
  {
    pRenderer->BeginFrame();

    pRenderer->EndFrame();
  }

  // Vulkan (Colored Triangle)

  // Inengine renderdoc (Press a button to do a capture)

  // ImGUI 

  // Multiview rendering / OpenXR

  // Jemalloc

  return 0;
}
