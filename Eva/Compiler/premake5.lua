project "Eva-Compiler"
    kind "ConsoleApp"
    language "C++"
    targetdir ("%{wks.location}/bin/%{prj.name}/"..outputdir)
    objdir ("%{wks.location}/bin-int/%{prj.name}/"..outputdir)
    
    includedirs
    {
       BASE_HEADER,
       CEREAL_HEADER,
       SPDLOG_HEADER
    }

    files
    {
        "%{prj.location}/src/**.cpp",
    }
    
    links {"Eva-Base"}
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    
