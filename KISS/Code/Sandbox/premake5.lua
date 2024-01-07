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
		"SDKs",
		"Graphics",
		"CoreEngine"
	}