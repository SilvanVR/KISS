#pragma once

////////////////////////////////////////////////////////////////////
enum class EFileWriteFlags
{
  Clear,
  Append
};

////////////////////////////////////////////////////////////////////
class CFile
{
public:
  CFile() = default;
  CFile(string&& data) : m_data(std::move(data)), m_bValid(true) {}

  const string& GetData() const { return m_data; }

  operator bool() const { return m_bValid; }

private:
  string m_data;
  bool m_bValid = false;
};

////////////////////////////////////////////////////////////////////
class CFileSystem
{
public:
  CFileSystem();
  ~CFileSystem() = default;

  CFile ReadFile(const string& fileName);
  void WriteFile(const string& fileName, const char* pBuffer, EFileWriteFlags eFlags);

  static CFileSystem& Instance();

private:
  string m_dataPath;
};