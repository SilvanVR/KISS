#pragma once

#include "../IVirtualFileSystem.h"

namespace VFS
{
  ////////////////////////////////////////////////////////////////////
  class CVirtualFileSystem : public IVirtualFileSystem
  {
  public:
    CVirtualFileSystem() = default;
    ~CVirtualFileSystem() = default;

    // IVirtualFileSystem Interface
    virtual CFile ReadFile(const string& fileName) override;
    virtual bool WriteFile(const string& fileName, const char* pBuffer, EFileWriteFlags eFlags) override;

    ///////////////////////////////
    void InitVFS();

  private:
    string m_dataPath;
  };
};