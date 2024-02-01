#include <stdafx.h>

#include "CoreEngine/CoreEngine.h"

#ifdef CONSOLE_APP
int main(int argc, char** argv)
#else
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR cmdline, int cmdshow)
#endif
{
  CCoreEngine* pCoreEngine = new CCoreEngine;
  pCoreEngine->InitEngine();
  pCoreEngine->Run();


  ////////////////////////////////////////////////////
  // TODO

	// Shaders (Colored Triangle)

  // Time management

  // Input manager

  // Vulkan 

  // Optik Profiler

  // Inengine renderdoc (Press a button to do a capture)

  // ImGUI 

  // Multiview rendering / OpenXR

  // Jemalloc

  return 0;
}
