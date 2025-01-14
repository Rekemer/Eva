project "Tests"
    kind "ConsoleApp"
    language "C++"
    targetdir ("%{wks.location}/bin/%{prj.name}/"..outputdir)
    objdir ("%{wks.location}/bin-int/%{prj.name}/"..outputdir)
    
    includedirs
    {   
        BASE_HEADER,
        CEREAL_HEADER
    }

    files
    {
        "%{prj.location}/src/**.cpp"
    }
    
    removefiles { "%{prj.location}/src/Tests.cpp"}
    
    links {"Eva-Base"}