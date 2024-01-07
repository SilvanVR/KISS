#include <stdafx.h>

#include "Graphics/IRenderer.h"

#ifdef CONSOLE_APP
int main(int argc, char** argv)
#else
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR cmdline, int cmdshow)
#endif
{ 
  LOG("Initializing Engine...");

  Graphics::IRenderer* pRenderer = Graphics::CreateRenderer();

  CVar* pVar = CConsole::Instance().GetCVar("r_width");

  LOG("Renderer initialized...");

  static string s_name;
  REGISTER_CVAR(&s_name, "e_name", "KISS", "Name of the program");
  CVar* pName = CConsole::Instance().GetCVar("e_name");

  int kggg = 42;

  // CVars
  // set them via config file e.g.: .ini file with r_vulkanDebug=1


  // GLWF or SDL?

  // Gameloop

  // Vulkan (Window + Colored Triangle)

  // Inengine renderdoc (Press a button to do a capture)

  // ImGUI 

  // Multiview rendering / OpenXR

  // Jemalloc

  return 0;
}
