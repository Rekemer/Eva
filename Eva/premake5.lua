project "Eva"
    kind "ConsoleApp"
    language "C++"
    targetdir ("%{wks.location}/bin/%{prj.name}/"..outputdir)
    objdir ("%{wks.location}/bin-int/%{prj.name}/"..outputdir)
    files
    {
        "%{prj.location}/src/**.cpp",
        "%{prj.location}/src/**.h",
    }
    removefiles { "%{prj.location}/src/String.cpp"}
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    
