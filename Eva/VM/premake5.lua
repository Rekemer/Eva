project "Eva-VM"
    kind "ConsoleApp"
    language "C++"
    targetdir ("%{wks.location}/bin/%{prj.name}/"..outputdir)
    objdir ("%{wks.location}/bin-int/%{prj.name}/"..outputdir)

    debugargs {"-test $(SolutionDir)Intermediates/test.json"}

    includedirs
    {
        BASE_HEADER,
        CEREAL_HEADER,
        SPDLOG_HEADER
    }

    files
    {
        "%{prj.location}/src/**.cpp",
        "%{prj.location}/src/**.h",
    }
    buildoptions { "/utf-8" }
    links {"Eva-Base"}

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    
