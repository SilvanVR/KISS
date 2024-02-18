#pragma once

#include <Common/Color.h>

////////////////////////////////////////////////////////////////////
struct DebugMarker
{
	DebugMarker(vk::CommandBuffer cmd, const char* pName, Color color = Color::GREEN);
	~DebugMarker();

private:
	vk::CommandBuffer m_cmd = VK_NULL_HANDLE;
};

////////////////////////////////////////////////////////////////////
#define GPU_DEBUG_MARKER(cmd, name, color) DebugMarker __debugMarker(cmd, name, color)
