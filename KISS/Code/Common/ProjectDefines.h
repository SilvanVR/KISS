#include <stdint.h>
#include <iostream>
#include <vector>
#include <assert.h>
#include <string>
#include <unordered_map>
#include <array>

using byte = uint8_t;

using int32 = int32_t;
using int64 = int64_t;

using uint32 = uint32_t;
using uint64 = uint64_t;

using f32 = float;
using f64 = double;

using string = std::string;

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
