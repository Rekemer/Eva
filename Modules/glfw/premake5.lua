project "GLFW"
    kind "SharedLib"
    language "C++"
    targetdir ("%{wks.location}/bin/Modules/%{prj.name}/"..outputdir)
    objdir ("%{wks.location}/bin-int/Modules/%{prj.name}/"..outputdir)

    includedirs
    {
       BASE_HEADER,
       CEREAL_HEADER,
       SPDLOG_HEADER,
       "%{prj.location}//Dependencies/GLFW/include"
    }

    files
    {
        "%{prj.location}/src/**.cpp",
    }

    libdirs { "%{prj.location}/Dependencies/GLFW/lib"}
    links { "glfw3.lib"}

    buildoptions { "/utf-8" }
    
    links {"Eva-Base"}
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    
       