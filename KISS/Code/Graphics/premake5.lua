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

	filter "configurations:Debug"
		links 
		{
			"glslang-default-resource-limitsd",
			"glslangd",
			"GenericCodeGend.lib",
			"MachineIndependentd.lib"
		}

	filter "configurations:Release"	
		links 
		{
			"glslang-default-resource-limits",
			"glslang",
			"GenericCodeGen.lib",
			"MachineIndependent.lib"
		}