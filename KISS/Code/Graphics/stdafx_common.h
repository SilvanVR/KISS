#include <Common/ProjectDefines.h>
#include <CoreEngine/IConsole.h>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <SDKs/vulkan/vulkan.hpp>
#undef max
#undef min
