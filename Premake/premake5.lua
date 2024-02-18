projectName = "KISS"

projectRoot = "../" .. projectName .. "/"

solution = projectRoot .. "Solution/"
code     = projectRoot .. "Code/"
libs     = projectRoot .. "Libs/"
bin      = projectRoot .. "Bin64/" -- %{cfg.buildcfg}
binTmp   = projectRoot .. "Bin64/BinTmp"

-- Relative to the included file (Include these in each sub premake build file if needed)
includeDir = {}
includeDir["Root"]         = ""
includeDir["RootProjects"] = ".."
includeDir["Common"]       = "../Common/"
includeDir["SDKs"]         = "../SDKs/"

-- Workspace
workspace (projectName)
	location (solution)
	architecture "x64"
	language "C++"
	targetdir (bin)
	objdir (binTmp)
	libdirs (libs)
	configurations { "Debug", "Release" }
	startproject "Sandbox"

	-- Execute buildscript each time before we build (MAY BECOME TO EXPENSIVE IN THE FUTURE)
	prebuildcommands { 'cd "$(ProjectDir)../.."', 'call "GenerateProject.bat"' }
	
	buildoptions { "/wd4267" }
	linkoptions { "/ignore:4099" } 

	-- filters
	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"
		defines { "WIN32" }
		
	filter "configurations:Debug"	  
		defines { "DEBUG" }
		symbols "On"
		buildoptions "/MDd"

	filter "configurations:Release"
		defines { "RELEASE" }
		optimize "On"
		buildoptions "/MD"

	--- Projects
	group "KISSEngine"
		include (code .. "Common")
		include (code .. "SDKs")
		include (code .. "Graphics")
		include (code .. "CoreEngine")
	group ""

	include (code .. "Sandbox")