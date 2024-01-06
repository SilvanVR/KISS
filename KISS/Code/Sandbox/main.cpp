#include <stdafx.h>


#ifdef CONSOLE_APP
int main(int argc, char** argv)
#else
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR cmdline, int cmdshow)
#endif
{
  LOG("Initializing Engine...");

  // Define own assert (skippable)

  // Gameloop

  // Vulkan (Window + Colored Triangle)

  // Inengine renderdoc (Press a button to do a capture)

  // ImGUI

  // Multiview rendering / OpenXR


  return 0; 
}
