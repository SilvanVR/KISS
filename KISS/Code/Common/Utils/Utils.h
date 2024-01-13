#pragma once

#include "stdafx_common.h"

namespace Utils
{

  ////////////////////////////////////////////////////////////////////
  template<typename T, typename T2>
  T Clamp(const T& value, const T2& min, const T2& max) { return value < T(min) ? T(min) : (value > T(max) ? T(max) : value); }

}