project "OpenGL-Legacy"
    kind "SharedLib"
    language "C++"
    targetdir ("%{wks.location}/bin/Modules/%{prj.name}/"..outputdir)
    objdir ("%{wks.location}/bin-int/Modules/%{prj.name}/"..outputdir)

    includedirs
    {
       BASE_HEADER,
       CEREAL_HEADER,
    }

    files
    {
        "%{prj.location}/src/**.cpp",
    }

    links { "opengl32.lib"}

    buildoptions { "/utf-8" }
    
    links {"Eva-Base"}
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    
       