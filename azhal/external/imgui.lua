project "imgui"
	location "../../temp/build/imgui"
    kind "StaticLib"
    language "C++"

	outputdir = "%{cfg.buildcfg}_%{cfg.architecture}"	
    targetdir ("../../bin/%{prj.name}/" .. outputdir)
    objdir ("../../temp/int/%{prj.name}/" .. outputdir)
	
    --Common Files
    files
    {
        "imgui/imgui.cpp",
		"imgui/backends/imgui_impl_glfw.h",
		"imgui/backends/imgui_impl_vulkan.h",
		"imgui/backends/imgui_impl_glfw.cpp",
		"imgui/backends/imgui_impl_vulkan.cpp"
    }
	
	includedirs
	{ 
		"imgui",
		"glfw/include",
		"$(VULKAN_SDK)/Include"
	}
    
    filter "system:windows"
        systemversion "latest"
        staticruntime "on"

	filter "system:windows"
        defines { "_CRT_SECURE_NO_WARNINGS" }


	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		optimize "off"		
		
	filter "configurations:Release"
		runtime "Release"
		symbols "on"
		optimize "Debug"
	
	filter "configurations:Final"
		runtime "Release"
		symbols "off"
		optimize "Full"
