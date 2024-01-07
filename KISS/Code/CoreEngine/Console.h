#pragma once

#include "Common/Color.h"

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
class CConsole
{
public:
  void Log(const char* format, ...);
  void Warn(const char* format, ...);
  void Error(const char* format, ...);
  void SetColor(Color color, Color backgroundColor = Color::BLACK) const;

  void RegisterCVar(int64* pVar, const string& name, int64 nVal, const string& tooltip);
  void RegisterCVar(string* pVar, const string& name, const string& str, const string& tooltip);
  CVar* GetCVar(const string& name);

  static CConsole& Instance();

private:
  std::unordered_map<string, CVar> m_cvars;

  void _WriteLine(const char* pBuffer);
};

#define LOG(format, ...)       CConsole::Instance().Log(format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  CConsole::Instance().Warn(format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) CConsole::Instance().Error(format, ##__VA_ARGS__)

#define REGISTER_CVAR(var, name, value, tooltip) CConsole::Instance().RegisterCVar(var, name, value, tooltip);
