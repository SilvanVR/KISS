#include "stdafx_common.h"
#include "StringUtils.h"

#include <sstream>
#include <algorithm>

namespace StringUtils
{

  ////////////////////////////////////////////////////////////////////
  std::vector<string> SplitString(const string& input, char delim)
  {
    std::vector<std::string> result;
    std::istringstream iss(input);

    std::string token;
    while (std::getline(iss, token, delim))
      result.push_back(token);

    return result;
  }

  ////////////////////////////////////////////////////////////////////
  string RemoveWhitespace(const string& input)
  {
    string result = input;
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
    return result;
  }

  ////////////////////////////////////////////////////////////////////
  string RemoveCharacter(const string& input, char c)
  {
    string result = input;
    result.erase(std::remove(result.begin(), result.end(), c), result.end());
    return result;
  }

  ////////////////////////////////////////////////////////////////////
  string ToLower(const string& input)
  {
    string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
  }

}