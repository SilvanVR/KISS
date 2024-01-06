#include "stdafx_common.h"
#include "logger.h"

#include <cstdarg>

#define BUFFER_SIZE 512

namespace Logging
{
  ////////////////////////////////////////////////////////////////////
  CLogger& CLogger::Instance()
  {
    static CLogger s_instance;
    return s_instance;
  }

  ////////////////////////////////////////////////////////////////////
  void CLogger::Log(const char* format, ...)
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
  void CLogger::Warn(const char* format, ...)
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
  void CLogger::Error(const char* format, ...)
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
  void CLogger::_WriteLine(const char* pBuffer)
  {
    std::cout << pBuffer << "\n";
  }

} // end namespaces
