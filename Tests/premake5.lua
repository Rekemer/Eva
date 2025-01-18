project "Tests"
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
        "%{prj.location}/src/**.cpp"
    }
    buildoptions { "/utf-8" }
    removefiles { "%{prj.location}/src/Tests.cpp"}
    
    filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"


    links {"Eva-Base"}