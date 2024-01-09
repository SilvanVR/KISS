project "Graphics"
	kind "StaticLib"

	pchheader "stdafx_common.h"
	pchsource "stdafx_common.cpp"

	files 
	{ 
		"**.h",
		"**.cpp" ,
		"premake5.lua"
	}

	includedirs	
	{ 
		"%{includeDir.Root}",
		"%{includeDir.RootProjects}",
		"%{includeDir.SDKs}"
	}

	links 
	{
		"CoreEngine",
		"glfw3dll"
	}
