#include "stdafx_common.h"
#include "Console.h"

#include <cstdarg>

#define BUFFER_SIZE 512

////////////////////////////////////////////////////////////////////
CConsole& CConsole::Instance()
{
  static CConsole s_instance;
  return s_instance;
}

////////////////////////////////////////////////////////////////////
void CConsole::Log(const char* format, ...)
{
  SetColor(Color::WHITE, Color::BLACK);
  va_list args;
  va_start(args, format);
  char buffer[BUFFER_SIZE];
  int pos = snprintf(buffer, BUFFER_SIZE, "%s", "[LOG] ");
  int length = vsnprintf(buffer + pos, BUFFER_SIZE, format, args);
  assert(length >= 0 && length < BUFFER_SIZE);
  va_end(args);
  _WriteLine(buffer);
}

////////////////////////////////////////////////////////////////////
void CConsole::Warn(const char* format, ...)
{
  SetColor(Color::YELLOW, Color::BLACK);
  va_list args;
  va_start(args, format);
  char buffer[BUFFER_SIZE];
  int pos = snprintf(buffer, BUFFER_SIZE, "%s", "[WARNING] ");
  int length = vsnprintf(buffer + pos, BUFFER_SIZE, format, args);
  assert(length >= 0 && length < BUFFER_SIZE);
  va_end(args);
  _WriteLine(buffer);
}

////////////////////////////////////////////////////////////////////
void CConsole::Error(const char* format, ...)
{
  SetColor(Color::RED, Color::BLACK);
  va_list args;
  va_start(args, format);
  char buffer[BUFFER_SIZE];
  int pos = snprintf(buffer, BUFFER_SIZE, "%s", "[ERROR] ");
  int length = vsnprintf(buffer + pos, BUFFER_SIZE, format, args);
  assert(length >= 0 && length < BUFFER_SIZE);
  va_end(args);
  _WriteLine(buffer);
}

////////////////////////////////////////////////////////////////////
void CConsole::RegisterCVar(int64* pVar, const string& name, int64 nVal, const string& tooltip)
{
  auto it = m_cvars.find(name);
  if (it != m_cvars.end())
  {
    LOG_WARN("CVar '%s' registered more than once.", name.c_str());
    return;
  }

  // Check if it exists in config file and assign if not
  *pVar = nVal;

  m_cvars[name] = CVar{ pVar, tooltip };
}

////////////////////////////////////////////////////////////////////
void CConsole::RegisterCVar(string* pVar, const string& name, const string& str, const string& tooltip)
{
  auto it = m_cvars.find(name);
  if (it != m_cvars.end())
  {
    LOG_WARN("CVar '%s' registered more than once.", name.c_str());
    return;
  }

  // Check if it exists in config file and assign if not
  *pVar = str;

  m_cvars[name] = CVar{ pVar, tooltip };
}

////////////////////////////////////////////////////////////////////
CVar* CConsole::GetCVar(const string& name)
{
  auto it = m_cvars.find(name);
  if (it != m_cvars.end())
    return &it->second;
  LOG_WARN("Can not find cvar '%s'", name.c_str());
  return nullptr;
}

////////////////////////////////////////////////////////////////////
void CConsole::_WriteLine(const char* pBuffer)
{
  std::cout << pBuffer << "\n";
}
