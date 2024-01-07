project "SDKs"
	kind "StaticLib"

	files 
	{ 
		"**.h",
		"**.cpp",
		"**.c",
		"premake5.lua"
	}

	includedirs	
	{ 
		"%{includeDir.Root}"
	}
