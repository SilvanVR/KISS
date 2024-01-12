#pragma once

#include "stdafx_common.h"

namespace StringUtils
{

  ////////////////////////////////////////////////////////////////////
  std::vector<string> SplitString(const string& input, char delim);
  string RemoveWhitespace(const string& input);
  string RemoveCharacter(const string& input, char c);
  string ToLower(const string& input);

}