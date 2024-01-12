#include "stdafx_common.h"

#include "FileSystem.h"

#include <fstream>

////////////////////////////////////////////////////////////////////
CFileSystem::CFileSystem()
{
  m_dataPath = "../Data/";
}

////////////////////////////////////////////////////////////////////
CFileSystem& CFileSystem::Instance()
{
  static CFileSystem s_fileSystem;
  return s_fileSystem;
}

////////////////////////////////////////////////////////////////////
CFile CFileSystem::ReadFile(const string& fileName)
{
  string strRealPath = m_dataPath + fileName;
  std::ifstream file(strRealPath);

  if (!file.is_open())
  {
    LOG_ERROR("Error opening file: %s", strRealPath.c_str());
    return CFile{};
  }

  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  file.close();

  return CFile{ std::move(content) };
}