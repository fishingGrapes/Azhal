workspace "azhal"
	architecture "x64"
	startproject "sandbox"

	--Declare Configs
	configurations 
	{
		--unoptimized, with logging
		"Debug",
		--optimized, with logging
		"Release",
		--fully optmized, without logging
		"Final"
	}

--Congig-Platform-Architecture : Debug_x64
outputdir = "%{cfg.buildcfg}_%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludePaths =  {}
IncludePaths["cxxopts"] = "external/cxxopts/include"
IncludePaths["glfw"] = "external/glfw/include"
IncludePaths["glm"] = "external/glm"
IncludePaths["imgui"] = "external/imgui/"
IncludePaths["spdlog"] = "external/spdlog/include"

group "Dependencies"
	include ("external/glfw")
	include ("external/imgui")
group ""

project "common"
	location "temp/build/common"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/%{prj.name}/" .. outputdir)
	objdir ("temp/int/%{prj.name}/" .. outputdir)

	disablewarnings { "26812" }
	
	--Set the Precompiled Header
	--IMPORTANT: Project name must be Hardcoded
	 pchheader ("cmnpch.h")
	 pchsource ("common/src/cmnpch.cpp")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.c",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludePaths.cxxopts}",
		"%{IncludePaths.glm}",
		"%{IncludePaths.spdlog}"
	}
	
	defines
	{
		"GLM_FORCE_RADIANS"
	}	
	
	filter "system:windows"
			systemversion "latest"
			buildoptions { "/Zc:__cplusplus" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		optimize "off"
		defines
		{ 
			"AZHAL_DEBUG", 
			"AZHAL_ENABLE_LOGGING",
			"AZHAL_ENABLE_ASSERTS"
		}			
		
	filter "configurations:Release"
		runtime "Release"
		symbols "on"
		optimize "Debug"
		defines
		{ 
			"AZHAL_RELEASE",
			"AZHAL_ENABLE_LOGGING",
			"AZHAL_ENABLE_ASSERTS"
		}
	
	filter "configurations:Final"
		runtime "Release"
		symbols "off"
		optimize "Full"
		defines{ "AZHAL_FINAL" }


project "azhal"
	location "temp/build/azhal"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/%{prj.name}/" .. outputdir)
	objdir ("temp/int/%{prj.name}/" .. outputdir)

	--Set the Precompiled Header
	--IMPORTANT: Project name must be Hardcoded
	 pchheader ("azpch.h")
	 pchsource ("azhal/src/azpch.cpp")

	disablewarnings { "26812" }

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.c",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"common/src",
		"%{prj.name}/src",
		"%{IncludePaths.glfw}",
		"%{IncludePaths.glm}",
		"%{IncludePaths.imgui}",
		"%{IncludePaths.spdlog}",
		"$(VULKAN_SDK)/Include"
	}
	
	libdirs 
	{ 	
		"$(VULKAN_SDK)/Lib",
	}

	links
	{
		"common",
		"vulkan-1.lib",
		"glfw",
		"imgui"
	}
	
	defines
	{
		--"VULKAN_HPP_NO_EXCEPTIONS",
		"VULKAN_HPP_NO_CONSTRUCTORS",
		"GLFW_INCLUDE_VULKAN",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_LEFT_HANDED"
	}
	
	
	filter "system:windows"
			systemversion "latest"
			buildoptions { "/Zc:__cplusplus" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		optimize "off"
		defines
		{ 
			"AZHAL_DEBUG"
		}			
		
	filter "configurations:Release"
		runtime "Release"
		symbols "on"
		optimize "Debug"
		defines
		{ 
			"AZHAL_RELEASE"
		}
	
	filter "configurations:Final"
		runtime "Release"
		symbols "off"
		optimize "Full"
		defines{ "AZHAL_FINAL" }


project "sandbox"
	location "temp/build/sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/%{prj.name}/" .. outputdir )
	objdir ("temp/int/%{prj.name}/" .. outputdir )

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"common/src",
		"azhal/src",
		"%{IncludePaths.cxxopts}",
		"%{IncludePaths.glm}",
		"%{IncludePaths.spdlog}"
	}

	links 
	{ 
		"azhal"
	}
	
	defines
	{
		"GLM_FORCE_RADIANS"
	}
	
	filter "system:windows"
			systemversion "latest"
			buildoptions { "/Zc:__cplusplus" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		optimize "off"
		defines
		{ 
			"AZHAL_DEBUG"
		}			
		
	filter "configurations:Release"
		runtime "Release"
		symbols "on"
		optimize "Debug"
		defines
		{ 
			"AZHAL_RELEASE"
		}
	
	filter "configurations:Final"
		runtime "Release"
		symbols "off"
		optimize "Full"
		defines{ "AZHAL_FINAL" }
