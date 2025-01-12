project "Eva-Base"
    kind "StaticLib"
    language "C++"
    targetdir ("%{wks.location}/bin/%{prj.name}/"..outputdir)
    objdir ("%{wks.location}/bin-int/%{prj.name}/"..outputdir)
    includedirs
    {
       CEREAL_HEADER
    }
    files
    {
        "%{prj.location}/src/**.cpp",
        "%{prj.location}/src/**.h",
    }

    removefiles { "%{prj.location}/src/String.cpp"}
    removefiles { "%{prj.location}/../VM/src/VirtualMachine.cpp"}
    removefiles { "%{prj.location}/../Compiler/src/Compiler.cpp"}
    
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    
