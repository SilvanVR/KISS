--projectKind = "WindowedApp"
projectKind = "ConsoleApp"

project "Sandbox"
	kind (projectKind)
		
	pchheader "stdafx.h"
	pchsource "stdafx.cpp"

	if (projectKind) == "ConsoleApp" then
		defines { "CONSOLE_APP" }
	end

	files 
	{ 
		"**.h",
		"**.cpp",
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
		"Common",
		"Graphics",
		"CoreEngine"
	}
	
	filter "configurations:Debug"	  
		targetname "SandboxDev"

	filter "configurations:Release"
		targetname "Sandbox"
