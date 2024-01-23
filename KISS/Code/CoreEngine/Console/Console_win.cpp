#include "stdafx_common.h"
#include "Console.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

////////////////////////////////////////////////////////////////////
void CConsole::SetColor(Color color, Color backgroundColor) const
{
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

  // Set Text-Color
  int col = (color.GetMax() > 127 ? FOREGROUND_INTENSITY : 0);
  if (color.GetRed() > 0)
    col |= FOREGROUND_RED;
  if (color.GetGreen() > 0)
    col |= FOREGROUND_GREEN;
  if (color.GetBlue() > 0)
    col |= FOREGROUND_BLUE;

  // Set Background-Color
  if (backgroundColor.GetRed() > 0)
    col |= BACKGROUND_RED;
  if (backgroundColor.GetGreen() > 0)
    col |= BACKGROUND_GREEN;
  if (backgroundColor.GetBlue() > 0)
    col |= BACKGROUND_BLUE;
  if (backgroundColor.GetMax() > 127)
    col |= BACKGROUND_INTENSITY;

  SetConsoleTextAttribute(hConsole, col);
}

#endif // !_WIN32