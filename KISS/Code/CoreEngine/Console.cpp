#include "stdafx_common.h"
#include "Console.h"
#include "FileSystem.h"

#include <cstdarg>
#include <iomanip>
#include <ctime>
#include <sstream>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
#endif

#define BUFFER_SIZE   512

#define ENGINE_CONFIG "engine.ini"
#define ENGINE_LOG    "engine.log"

#define BUFFER_TRUNCATED() \
  { SetColor(Color::YELLOW, Color::BLACK); \
  _WriteLine(__FUNCTION__ "(): Buffer too big. Output was truncated. Increase BUFFER_SIZE\n"); }\

////////////////////////////////////////////////////////////////////
string GetCurrentDateTime() 
{
  // Get the current time
  std::time_t currentTime;
  time(&currentTime);

  // Localtime_s for safer localtime
  std::tm localTime;
  localtime_s(&localTime, &currentTime);

  // Create a stringstream to format the date and time
  std::stringstream ss;
  ss << std::put_time(&localTime, "<%d.%m.%Y-%H:%M:%S>");

  return ss.str();
}

////////////////////////////////////////////////////////////////////
int64 CConsole::CV_r_logVerbosity;

////////////////////////////////////////////////////////////////////
CConsole& CConsole::Instance()
{
  static CConsole s_instance;
  return s_instance;
}

////////////////////////////////////////////////////////////////////
CConsole::CConsole()
{
  CFileSystem::Instance().WriteFile(ENGINE_LOG, "", EFileWriteFlags::Clear);

  size_t numLinesRead = 0;
  if (CFile file = CFileSystem::Instance().ReadFile(ENGINE_CONFIG))
  {
    const string& contents = file.GetData();
    std::vector<string> arrLines = StringUtils::SplitString(contents, '\n');
    for (const string& line : arrLines)
    {
      std::vector<string> arrLine = StringUtils::SplitString(line, '=');
      if (arrLine.empty())
        continue;

      // Skip non valid lines
      char firstChar = arrLine.front()[0];
      if ((firstChar < 'a' || firstChar > 'z') && (firstChar < 'A' || firstChar > 'Z'))
        continue;

      if (arrLine.size() != 2)
      {
        Error("%s: Failed to read line: %s", ENGINE_CONFIG, line.c_str());
        continue;
      }

      string name = StringUtils::RemoveWhitespace(arrLine[0]);
      name = StringUtils::ToLower(name);
      if (m_iniMap.find(name) != m_iniMap.end())
      {
        Warn("%s: Duplicated entry '%s' found. Please make sure every entry exists once.", ENGINE_CONFIG, name.c_str());
        continue;
      }

      string value = StringUtils::RemoveWhitespace(arrLine[1]);
      value = StringUtils::RemoveCharacter(value, '"'); // Remove " character in strings
      m_iniMap[name] = value;
    }
    numLinesRead = arrLines.size();
  }
  _RegisterCVars();

  Log("Successfully read %s (%llu entries read - %llu added)", ENGINE_CONFIG, numLinesRead, m_iniMap.size());
}

////////////////////////////////////////////////////////////////////
void CConsole::Log(const char* format, ...)
{
  if (CV_r_logVerbosity == 0)
    return;
  SetColor(Color::WHITE, Color::BLACK);
  va_list args;
  va_start(args, format);
  char buffer[BUFFER_SIZE];
  int length = vsnprintf(buffer, BUFFER_SIZE, format, args);
  va_end(args);
  _Log(buffer);
  if (length > BUFFER_SIZE)
    BUFFER_TRUNCATED();
}

////////////////////////////////////////////////////////////////////
void CConsole::LogAlways(const char* format, ...)
{
  SetColor(Color::TURQUOISE, Color::BLACK);
  va_list args;
  va_start(args, format);
  char buffer[BUFFER_SIZE];
  int length = vsnprintf(buffer, BUFFER_SIZE, format, args);
  va_end(args);
  _Log(buffer);
  if (length > BUFFER_SIZE)
    BUFFER_TRUNCATED();
  SetColor(Color::WHITE, Color::BLACK);
}

////////////////////////////////////////////////////////////////////
void CConsole::_Log(const char* pBuffer)
{
  string output = GetCurrentDateTime();
  output += "[INFO] ";
  output += pBuffer;
  output += "\n";
  _WriteLine(output.c_str());
}

////////////////////////////////////////////////////////////////////
void CConsole::Warn(const char* format, ...)
{
  SetColor(Color::YELLOW, Color::BLACK);
  va_list args;
  va_start(args, format);
  char buffer[BUFFER_SIZE];
  int length = vsnprintf(buffer, BUFFER_SIZE, format, args);
  va_end(args);

  string output = GetCurrentDateTime();
  output += "[WARN] ";
  output += buffer;
  output += "\n";

  _WriteLine(output.c_str());
  if (length > BUFFER_SIZE)
    BUFFER_TRUNCATED();
  SetColor(Color::WHITE, Color::BLACK);
}

////////////////////////////////////////////////////////////////////
void CConsole::Error(const char* format, ...)
{
  SetColor(Color::RED, Color::BLACK);
  va_list args;
  va_start(args, format);
  char buffer[BUFFER_SIZE];
  int length = vsnprintf(buffer, BUFFER_SIZE, format, args);
  va_end(args);

  string output = GetCurrentDateTime();
  output += "[ERROR] ";
  output += buffer;
  output += "\n";

  _WriteLine(output.c_str());
  if (length > BUFFER_SIZE)
    BUFFER_TRUNCATED();

#ifdef _WIN32
  MessageBoxA(NULL, buffer, "Error", MB_OK);
#endif

  SetColor(Color::WHITE, Color::BLACK);
}

////////////////////////////////////////////////////////////////////
void CConsole::RegisterCVar(const string& input, int64* pVar, int64 nDefaultValue, const string& tooltip)
{
  string name = StringUtils::ToLower(input);
  auto it = m_cvars.find(name);
  if (it != m_cvars.end())
  {
    KISS_LOG_WARN("CVar '%s' registered more than once.", name.c_str());
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
      KISS_LOG_WARN("CVar '%s' in '%s' is not a number (value: '%s') but was registered as one (error: %s). Please fix this.", name.c_str(), ENGINE_CONFIG, it2->second.c_str(), e.what());
    }
  }

  m_cvars[name] = CVar{ pVar, tooltip };
}

////////////////////////////////////////////////////////////////////
void CConsole::RegisterCVar(const string& input, string* pVar, const string& defaultString, const string& tooltip)
{
  string name = StringUtils::ToLower(input);
  auto it = m_cvars.find(name);
  if (it != m_cvars.end())
  {
    KISS_LOG_WARN("CVar '%s' registered more than once.", name.c_str());
    return;
  }

  // If name exists in ini file use it otherwise apply default value
  auto it2 = m_iniMap.find(name);
  *pVar = it2 == m_iniMap.end() ? defaultString : it2->second;

  m_cvars[name] = CVar{ pVar, tooltip };
}

////////////////////////////////////////////////////////////////////
CVar* CConsole::GetCVar(const string& input)
{
  string name = StringUtils::ToLower(input);
  auto it = m_cvars.find(name);
  if (it != m_cvars.end())
    return &it->second;
  KISS_LOG_WARN("Can not find cvar '%s'", name.c_str());
  return nullptr;
}

////////////////////////////////////////////////////////////////////
void CConsole::_RegisterCVars()
{
  RegisterCVar("log.verbosity", &CV_r_logVerbosity, 1, 
    "Controls how much of the output is visible.\n"
    "0: Only warnings and errors\n"
    "1: All output\n"
  );
}

////////////////////////////////////////////////////////////////////
void CConsole::_WriteLine(const char* pBuffer)
{
  _PrintToConsole(pBuffer);
  CFileSystem::Instance().WriteFile(ENGINE_LOG, pBuffer, EFileWriteFlags::Append);
}

////////////////////////////////////////////////////////////////////
void CConsole::_PrintToConsole(const char* pBuffer)
{
  std::cout << pBuffer;
}