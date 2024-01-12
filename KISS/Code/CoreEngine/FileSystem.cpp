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
    KISS_LOG_ERROR("Error opening file: %s", strRealPath.c_str());
    return CFile{};
  }

  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  file.close();

  return CFile{ std::move(content) };
}

////////////////////////////////////////////////////////////////////
void CFileSystem::WriteFile(const string& fileName, const char* pBuffer, EFileWriteFlags eFlags)
{
  std::ofstream file;

  switch (eFlags)
  {
  case EFileWriteFlags::Clear:  file.open(fileName, std::ios::out); break;
  case EFileWriteFlags::Append: file.open(fileName, std::ios::out | std::ios::app); break;
  }

  if (!file.is_open()) {
    KISS_LOG_ERROR("Error opening file: %s", fileName.c_str());
    return;
  }

  file << pBuffer;

  file.close();
}
