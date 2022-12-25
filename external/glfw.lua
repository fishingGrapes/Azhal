project "glfw"
	location "../temp/build/glfw"
    kind "StaticLib"
    language "C"

	outputdir = "%{cfg.buildcfg}_%{cfg.architecture}"
    targetdir ("../bin/%{prj.name}/" .. outputdir)
    objdir ("../temp/int/%{prj.name}/" .. outputdir)


	includedirs { "glfw/include/" }
	
    --Common Files
    files
    {
        "glfw/src/glfw_config.h",
        "glfw/src/context.c",
        "glfw/src/init.c",
        "glfw/src/input.c",
        "glfw/src/monitor.c",
        "glfw/src/vulkan.c",
        "glfw/src/window.c",
		"glfw/src/platform.c",
		"glfw/src/null_platform.h",
		"glfw/src/null_joystick.h",
		"glfw/src/null_init.c",
		"glfw/src/null_monitor.c",
		"glfw/src/null_window.c",
		"glfw/src/null_joystick.c"
    }
    
    filter "system:windows"
        systemversion "latest"
        staticruntime "on"
        
        --Windows Specific files in GLFW's Src Folder
        files
        {
			"glfw/src/win32_init.c",
			"glfw/src/win32_module.c",
			"glfw/src/win32_joystick.c",
			"glfw/src/win32_monitor.c",
			"glfw/src/win32_time.h",
			"glfw/src/win32_time.c",
			"glfw/src/win32_thread.h",
			"glfw/src/win32_thread.c",
			"glfw/src/win32_window.c",
			"glfw/src/wgl_context.c",
			"glfw/src/egl_context.c",
			"glfw/src/osmesa_context.c"
        }

	filter "system:windows"
        defines { "_GLFW_WIN32", "_CRT_SECURE_NO_WARNINGS" }


	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		optimize "off"		
		
	filter "configurations:Release"
		runtime "Release"
		symbols "on"
		optimize "Debug"
		
	filter "configurations:Profile"
		runtime "Release"
		symbols "off"
		optimize "Full"
	
	filter "configurations:Final"
		runtime "Release"
		symbols "off"
		optimize "Full"
