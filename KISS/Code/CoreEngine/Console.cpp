#include "stdafx_common.h"
#include "Console.h"
#include "FileSystem.h"

#include <cstdarg>

#define BUFFER_SIZE 512

#define ENGINE_CONFIG "engine.ini"

////////////////////////////////////////////////////////////////////
CConsole& CConsole::Instance()
{
  static CConsole s_instance;
  return s_instance;
}

////////////////////////////////////////////////////////////////////
CConsole::CConsole()
{
  if (CFile file = CFileSystem::Instance().ReadFile(ENGINE_CONFIG))
  {
    const string& contents = file.GetData();
    std::vector<string> arrLines = CFileSystem::SplitString(contents, '\n');
    for (const string& line : arrLines)
    {
      std::vector<string> arrLines = CFileSystem::SplitString(line, '=');
      if (arrLines.empty())
        continue;

      if (arrLines.size() != 2)
      {
        Error("%s: Failed to read line: %s", ENGINE_CONFIG, line.c_str());
        continue;
      }

      string name = CFileSystem::RemoveWhitespace(arrLines[0]);
      if (m_iniMap.find(name) != m_iniMap.end())
      {
        Warn("%s: Duplicated entry '%s' found. Please make sure every entry exists once.", ENGINE_CONFIG, name.c_str());
        continue;
      }

      string value = CFileSystem::RemoveWhitespace(arrLines[1]);
      value = CFileSystem::RemoveCharacter(value, '"'); // Remove " character in strings
      m_iniMap[name] = value;
    }

    Log("Successfully read %s (%llu entries read)", ENGINE_CONFIG, arrLines.size());
  }
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
void CConsole::RegisterCVar(const string& name, int64* pVar, int64 nDefaultValue, const string& tooltip)
{
  auto it = m_cvars.find(name);
  if (it != m_cvars.end())
  {
    LOG_WARN("CVar '%s' registered more than once.", name.c_str());
    return;
  }

  // If name exists in ini file use it otherwise apply default value
  auto it2 = m_iniMap.find(name);
  if (it2 == m_iniMap.end())
  {
    *pVar = nDefaultValue;
  }
  else
  {
    try {
      *pVar = std::stoll(it2->second);
    }
    catch (const std::exception& e) {
      LOG_WARN("CVar '%s' in '%s' is not a number (value: '%s') but was registered as one (error: %s). Please fix this.", name.c_str(), ENGINE_CONFIG, it2->second.c_str(), e.what());
    }
  }

  m_cvars[name] = CVar{ pVar, tooltip };
}

////////////////////////////////////////////////////////////////////
void CConsole::RegisterCVar(const string& name, string* pVar, const string& defaultString, const string& tooltip)
{
  auto it = m_cvars.find(name);
  if (it != m_cvars.end())
  {
    LOG_WARN("CVar '%s' registered more than once.", name.c_str());
    return;
  }

  // If name exists in ini file use it otherwise apply default value
  auto it2 = m_iniMap.find(name);
  *pVar = it2 == m_iniMap.end() ? defaultString : it2->second;

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
