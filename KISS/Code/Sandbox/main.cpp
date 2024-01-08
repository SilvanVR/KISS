#include <stdafx.h>

#include "Graphics/IRenderer.h"

#ifdef CONSOLE_APP
int main(int argc, char** argv)
#else
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR cmdline, int cmdshow)
#endif
{
  static string s_name;
  REGISTER_CVAR("e.engineName", &s_name, "KISSEngine", "Name of the engine");

  LOG("Initializing Engine...");

  Graphics::IRenderer* pRenderer = Graphics::CreateRenderer();

  LOG("Renderer initialized...");

  // C++ modules
  
  // GLWF or SDL?

  // Gameloop

  // Vulkan (Window + Colored Triangle)

  // Inengine renderdoc (Press a button to do a capture)

  // ImGUI 

  // Multiview rendering / OpenXR

  // Jemalloc

  return 0;
}
