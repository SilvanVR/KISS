#pragma once

#include "Graphics/IRenderer.h"

////////////////////////////////////////////////////////////////////
class CCoreEngine
{
public:
	void InitEngine();
	void Run();
	void Heartbeat(f64 fTimeStep);

private:
	static string CV_e_engineName;
	static string CV_e_appName;

	void _RegisterCVars();
};