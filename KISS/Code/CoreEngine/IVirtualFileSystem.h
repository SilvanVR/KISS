#pragma once

#include <Common/ProjectDefines.h>

namespace VFS
{
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
  class IVirtualFileSystem
  {
  public:
    IVirtualFileSystem() = default;
    virtual ~IVirtualFileSystem() = default;

    virtual CFile ReadFile(const string& fileName) = 0;
    virtual bool WriteFile(const string& fileName, const char* pBuffer, EFileWriteFlags eFlags) = 0;
  };

  void CreateVirtualFileSystem();
};

extern VFS::IVirtualFileSystem* g_pVFS;