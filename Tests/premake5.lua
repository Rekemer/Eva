project "Tests"
    kind "ConsoleApp"
    language "C++"
    targetdir ("%{wks.location}/bin/%{prj.name}/"..outputdir)
    objdir ("%{wks.location}/bin-int/%{prj.name}/"..outputdir)
    

    files
    {
        "../Eva/src/**.cpp",
        "%{prj.location}/src/**.cpp"
    }
    includedirs
    {   
        "../Eva/src",
    }
    removefiles { "../Eva/src/String.cpp", "../Eva/src/Main.cpp" }

    
