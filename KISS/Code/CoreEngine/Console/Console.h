#pragma once

#include <Common/Color.h>

#include "IConsole.h"

////////////////////////////////////////////////////////////////////
class CConsole : public IConsole
{
public:
  CConsole() = default;

  virtual void Log(const char* format, ...) override;
  virtual void LogAlways(const char* format, ...) override;
  virtual void Warn(const char* format, ...) override;
  virtual void Error(const char* format, ...) override;
  virtual void SetColor(Color color, Color backgroundColor = Color::BLACK) const override;

  virtual void RegisterCVar(const string& name, int64* pVar, int64 nDefaultValue, const string& tooltip) override;
  virtual void RegisterCVar(const string& name, string* pVar, const string& nDefaultStr, const string& tooltip) override;
  virtual CVar* GetCVar(const string& name) override;

  void InitConsole();

  static int64 CV_r_logVerbosity;

private:
  std::map<string, CVar> m_cvars;
  std::unordered_map<string, string> m_iniMap;  

  void _Log(const char* pBuffer);
  void _RegisterCVars();
  void _WriteLine(const char* pBuffer);
  void _PrintToConsole(const char* pBuffer);
};