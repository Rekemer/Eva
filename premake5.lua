

workspace "Eva"
	architecture "x86_64"
	startproject "Eva"
    language "C++"
    cppdialect "C++20"
	configurations
	{
		"Debug",
		"Release",
		"Dist",
	}

	flags
	{
		"MultiProcessorCompile"
	}

	BASE_HEADER = "%{wks.location}/Eva/Base/src"

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	group "Dependencies"
	include "Dependencies/Cereal"
	group ""

	include "Eva/Base"
	include "Eva/Compiler"
	include "Eva/VM"
	include "Tests"
	

	