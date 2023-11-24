project "Tests"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
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
    filter "files:../Eva/src/Main.cpp"
    flags { "ExcludeFromBuild" } -- Exclude Main.cpp from build

    
