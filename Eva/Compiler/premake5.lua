project "Eva-Compiler"
    kind "ConsoleApp"
    language "C++"
    targetdir ("%{wks.location}/bin/%{prj.name}/"..outputdir)
    objdir ("%{wks.location}/bin-int/%{prj.name}/"..outputdir)
    -- turns out it works only if you generate the vcxproj from scratch (it doesn't exist)
    debugargs{"-spath=./test.eva -bpath=$(SolutionDir)Intermediates/test.json"}

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
    

    buildoptions { "/utf-8" }
    
    links {"Eva-Base"}
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    
       