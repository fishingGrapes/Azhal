project "tracy"
	location "../temp/build/tracy"
    kind "StaticLib"
    language "C++"

	outputdir = "%{cfg.buildcfg}_%{cfg.architecture}"
    targetdir ("../bin/%{prj.name}/" .. outputdir)
    objdir ("../temp/int/%{prj.name}/" .. outputdir)


	includedirs { "tracy/public/" }
	
    --Common Files
    files
    {
        "tracy/public/TracyClient.cpp"
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
