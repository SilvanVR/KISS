#pragma once

#include "color.h"

namespace Logging
{
  class CLogger
  {
  public:
    void Log(const char* format, ...);
    void Warn(const char* format, ...);
    void Error(const char* format, ...);
    void SetColor(Color color, Color backgroundColor = Color::BLACK) const;

    static CLogger& Instance();

  private:
    void _WriteLine(const char* pBuffer);
  };
}

#define LOG(format, ...)       Logging::CLogger::Instance().Log(format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  Logging::CLogger::Instance().Warn(format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Logging::CLogger::Instance().Error(format, ##__VA_ARGS__)
