#include "stdafx_common.h"

#include "FileSystem.h"

#include <fstream>
#include <sstream>

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

////////////////////////////////////////////////////////////////////
std::vector<string> CFileSystem::SplitString(const string& input, char delim)
{
  std::vector<std::string> result;
  std::istringstream iss(input);

  std::string token;
  while (std::getline(iss, token, delim))
    result.push_back(token);

  return result;
}

////////////////////////////////////////////////////////////////////
string CFileSystem::RemoveWhitespace(string result)
{
  result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
  return result;
}

////////////////////////////////////////////////////////////////////
string CFileSystem::RemoveCharacter(string input, char c)
{
  input.erase(std::remove(input.begin(), input.end(), c), input.end());
  return input;
}