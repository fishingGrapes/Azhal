workspace "azhal"
	architecture "x64"
	startproject "sandbox"

	--Declare Configs
	configurations 
	{
		--unoptimized, with logging, with profiling
		"Debug",
		--optimized, with logging, with profiling
		"Release",
		--fully optmized, without logging, with profiling
		"Profile",
		--fully optmized, without logging, without profiling
		"Final"
	}

--Congig-Platform-Architecture : Debug_x64
outputdir = "%{cfg.buildcfg}_%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludePaths =  {}
IncludePaths["vulkan"] = "$(VULKAN_SDK)/Include"
IncludePaths["cxxopts"] = "external/cxxopts/include"
IncludePaths["glfw"] = "external/glfw/include"
IncludePaths["glm"] = "external/glm"
IncludePaths["imgui"] = "external/imgui"
IncludePaths["spdlog"] = "external/spdlog/include"
IncludePaths["tracy"] = "external/tracy/public/tracy"
IncludePaths["common"] = "common/include"
IncludePaths["azhal"] = "azhal/include"

group "Dependencies"
	include ("external/glfw")
	include ("external/imgui")
	include ("external/tracy")
group ""

project "common"
	location "temp/build/common"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/%{prj.name}/" .. outputdir)
	objdir ("temp/int/%{prj.name}/" .. outputdir)

	includedirs
	{
		"%{IncludePaths.cxxopts}",
		"%{IncludePaths.glm}",
		"%{IncludePaths.tracy}",
		"%{IncludePaths.spdlog}",
		"%{IncludePaths.imgui}"
	}
	
	files
	{
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
	
	links
	{
		"tracy"
	}
	
	defines
	{
		"GLM_FORCE_RADIANS"
	}	
	
	filter "system:windows"
			systemversion "latest"
			buildoptions { "/Zc:__cplusplus" }
			defines { "AZHAL_PLATFORM_WINDOWS" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		optimize "off"
		defines
		{ 
			"AZHAL_DEBUG", 
			"AZHAL_ENABLE_LOGGING",
			"AZHAL_ENABLE_ASSERTS",
			"TRACY_ENABLE"
		}			
		
	filter "configurations:Release"
		runtime "Release"
		symbols "on"
		optimize "Debug"
		defines
		{ 
			"AZHAL_RELEASE",
			"AZHAL_ENABLE_LOGGING",
			"AZHAL_ENABLE_ASSERTS",
			"TRACY_ENABLE"
		}
		
	filter "configurations:Profile"
		runtime "Release"
		symbols "off"
		optimize "Full"
		defines
		{ 
			"AZHAL_FINAL",
			"TRACY_ENABLE"
		}
	
	filter "configurations:Final"
		runtime "Release"
		symbols "off"
		optimize "Full"
		defines
		{ 
			"AZHAL_FINAL" 
		}


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

	includedirs
	{
		"%{IncludePaths.vulkan}",
		"%{IncludePaths.cxxopts}",
		"%{IncludePaths.glfw}",
		"%{IncludePaths.glm}",
		"%{IncludePaths.tracy}",
		"%{IncludePaths.spdlog}",
		"%{IncludePaths.imgui}",
		"%{IncludePaths.common}",
	}
	
	files
	{
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
	
	libdirs 
	{ 	
		"$(VULKAN_SDK)/Lib"
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
		"VULKAN_HPP_NO_EXCEPTIONS",
		"VULKAN_HPP_NO_CONSTRUCTORS",
		"GLFW_INCLUDE_VULKAN",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_LEFT_HANDED"
	}
	
	
	filter "system:windows"
			systemversion "latest"
			buildoptions { "/Zc:__cplusplus" }
			defines { "AZHAL_PLATFORM_WINDOWS" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		optimize "off"
		defines
		{ 
			"AZHAL_DEBUG",
			"AZHAL_ENABLE_LOGGING",
			"TRACY_ENABLE"
		}			
		
	filter "configurations:Release"
		runtime "Release"
		symbols "on"
		optimize "Debug"
		defines
		{ 
			"AZHAL_RELEASE",
			"AZHAL_ENABLE_LOGGING",
			"TRACY_ENABLE"
		}
		
	filter "configurations:Profile"
		runtime "Release"
		symbols "off"
		optimize "Full"
		defines
		{ 
			"AZHAL_FINAL",
			"TRACY_ENABLE"
		}
	
	filter "configurations:Final"
		runtime "Release"
		symbols "off"
		optimize "Full"
		defines
		{ 
			"AZHAL_FINAL",
			--"VULKAN_HPP_NO_EXCEPTIONS",
			--"VK_DEADLY_VALIDATION"
		}


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
		"%{IncludePaths.vulkan}",
		"%{IncludePaths.cxxopts}",
		"%{IncludePaths.glm}",
		"%{IncludePaths.spdlog}",
		"%{IncludePaths.tracy}",
		"%{IncludePaths.imgui}",
		"%{IncludePaths.common}",
		"%{IncludePaths.azhal}"
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
			defines { "AZHAL_PLATFORM_WINDOWS" }
			
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		optimize "off"
		defines
		{ 
			"AZHAL_DEBUG",
			"AZHAL_ENABLE_LOGGING",
			"TRACY_ENABLE"
		}			
		
	filter "configurations:Release"
		runtime "Release"
		symbols "on"
		optimize "Debug"
		defines
		{ 
			"AZHAL_RELEASE",
			"AZHAL_ENABLE_LOGGING",
			"TRACY_ENABLE"
		}
		
	filter "configurations:Profile"
		runtime "Release"
		symbols "off"
		optimize "Full"
		defines
		{ 
			"AZHAL_FINAL",
			"TRACY_ENABLE"
		}
	
	filter "configurations:Final"
		runtime "Release"
		symbols "off"
		optimize "Full"
		defines
		{ 
			"AZHAL_FINAL"
		}
