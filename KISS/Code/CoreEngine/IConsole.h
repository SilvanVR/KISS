#pragma once

#include <Common/Color.h>

class IConsole;
extern IConsole* g_pConsole;

////////////////////////////////////////////////////////////////////
class CVar
{
  enum class EType
  {
    String,
    Number
  };
public:
  CVar() = default;
  ~CVar() = default;
  CVar(int64* pVar, const string& tooltip) : m_pVar(pVar), m_type(EType::Number), m_tooltip(tooltip) {}
  CVar(string* pVar, const string& tooltip) : m_pStr(pVar), m_type(EType::String), m_tooltip(tooltip) {}

  int64         GetIVal()    const { assert(m_type == EType::Number); return *m_pVar; }
  const string& GetString()  const { assert(m_type == EType::String); return *m_pStr; }
  const string& GetTooltip() const { return m_tooltip; }

  void Set(int64 nVal)        { assert(m_type == EType::Number); *m_pVar = nVal; }
  void Set(const string& val) { assert(m_type == EType::String); *m_pStr = val; }

private:
  EType m_type;
  union
  {
    int64*  m_pVar = nullptr;
    string* m_pStr;
  };
  string m_tooltip;
};

////////////////////////////////////////////////////////////////////
class IConsole
{
public:
  IConsole() = default;
  virtual ~IConsole() {}

  virtual void Log(const char* format, ...) = 0;
  virtual void LogAlways(const char* format, ...) = 0;
  virtual void Warn(const char* format, ...) = 0;
  virtual void Error(const char* format, ...) = 0;
  virtual void SetColor(Color color, Color backgroundColor = Color::BLACK) const = 0;

  virtual void RegisterCVar(const string& name, int64* pVar, int64 nDefaultValue, const string& tooltip) = 0;
  virtual void RegisterCVar(const string& name, string* pVar, const string& nDefaultStr, const string& tooltip) = 0;
  virtual CVar* GetCVar(const string& name) = 0;
};

void CreateConsole();

#define KISS_LOG(format, ...)        g_pConsole->Log(format, ##__VA_ARGS__)
#define KISS_LOG_ALWAYS(format, ...) g_pConsole->LogAlways(format, ##__VA_ARGS__)
#define KISS_LOG_WARN(format, ...)   g_pConsole->Warn(format, ##__VA_ARGS__)
#define KISS_LOG_ERROR(format, ...)  g_pConsole->Error(format, ##__VA_ARGS__)

#define KISS_FATAL(format, ...) \
g_pConsole->Error(format, ##__VA_ARGS__);\
exit(1);

#define KISS_FATAL_COND(cond, format, ...) \
if (!(cond)){\
g_pConsole->Error(format, ##__VA_ARGS__); \
exit(1);}

#define REGISTER_CVAR(var, name, value, tooltip) g_pConsole->RegisterCVar(var, name, value, tooltip);