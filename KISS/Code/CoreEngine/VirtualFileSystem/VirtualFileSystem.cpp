#include "stdafx_common.h"

#include "VirtualFileSystem.h"

#include <fstream>

VFS::IVirtualFileSystem* g_pVFS = nullptr;

namespace VFS
{
	////////////////////////////////////////////////////////////////////
  void CreateVirtualFileSystem()
	{
    assert(!g_pVFS);
		CVirtualFileSystem* pVFS = new CVirtualFileSystem;
    g_pVFS = pVFS;
		pVFS->InitVFS();
	}

	////////////////////////////////////////////////////////////////////
	void CVirtualFileSystem::InitVFS()
	{
		m_dataPath = "../Data/";
	}

  ////////////////////////////////////////////////////////////////////
  CFile CVirtualFileSystem::ReadFile(const string& fileName)
  {
    string realFilePath = m_dataPath + fileName;
    std::ifstream file(realFilePath);

    if (!file.is_open())
    {
      KISS_LOG_ERROR("Error opening file: %s", realFilePath.c_str());
      return CFile{};
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    return CFile{ std::move(content) };
  }

  ////////////////////////////////////////////////////////////////////
  bool CVirtualFileSystem::WriteFile(const string& fileName, const char* pBuffer, EFileWriteFlags eFlags)
  {
    string realFilePath = m_dataPath + fileName;
    std::ofstream file;

    switch (eFlags)
    {
    case EFileWriteFlags::Clear:  file.open(realFilePath, std::ios::out); break;
    case EFileWriteFlags::Append: file.open(realFilePath, std::ios::out | std::ios::app); break;
    }

    if (!file.is_open()) {
      KISS_LOG_ERROR("Error opening file: %s", realFilePath.c_str());
      return false;
    }

    file << pBuffer;

    file.close();

    return true;
  }


}