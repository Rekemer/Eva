

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

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	include "Eva"
	include "Tests"
	

	