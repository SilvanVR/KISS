#include "stdafx_common.h"

#include "DebugMarker.h"

////////////////////////////////////////////////////////////////////
DebugMarker::DebugMarker(vk::CommandBuffer cmd, const char* pName, Color color)
	: m_cmd(cmd)
{
	vk::DebugMarkerMarkerInfoEXT markerInfo(pName);
	markerInfo.color = color.Normalized();
	m_cmd.debugMarkerBeginEXT(markerInfo);
}

////////////////////////////////////////////////////////////////////
DebugMarker::~DebugMarker()
{
	m_cmd.debugMarkerEndEXT();
}