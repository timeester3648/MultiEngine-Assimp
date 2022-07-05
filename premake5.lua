include "../../premake/common_premake_defines.lua"
		
project "Assimp"
	kind "StaticLib"
	language "C++"
	cppdialect "C++latest"
	cdialect "C17"
	targetname "%{prj.name}"
	inlining "Auto"

	files {
		"./code/**.c",
		"./code/**.h",
		"./code/**.cc",
		"./code/**.inl",
		"./code/**.hpp",
		"./code/**.cpp",

		"./contrib/**.c",
		"./contrib/**.h",
		"./contrib/**.cc",
		"./contrib/**.inl",
		"./contrib/**.hpp",
		"./contrib/**.cpp",

		"./include/**.h"
	}

	includedirs {
		".",
		"./code",
		"./include",
		"./contrib",
		"./contrib/unzip",
		"./contrib/irrXML",
		"./contrib/pugixml/src",
		"./contrib/rapidjson/include",
		"./contrib/openddlparser/include",

		"%{IncludeDir.mle}",
		"%{IncludeDir.zlib}",
		"%{IncludeDir.assimp}"
	}

	excludes {
		"./contrib/zlib/*.c",
		"./contrib/draco/**.cc",
		"./contrib/gtest/**.cc",

		"./code/AssetLib/C4D/**.cpp",
		"./code/AssetLib/IFC/**.cpp",
		"./code/AssetLib/M3D/**.cpp",

		"./contrib/zip/**.c",

		"./contrib/zlib/**.c",
		"./contrib/zlib/**.cc"
	}

	defines {
		"OPENDDL_STATIC_LIBARY",
		"ASSIMP_BUILD_NO_EXPORT",
		"ASSIMP_BUILD_NO_OWN_ZLIB"
	}

 	filter "system:windows"
		buildoptions { "/bigobj" }
		defines { "_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING", "_CRT_SECURE_NO_WARNINGS" }

 	filter "configurations:Debug"
		defines { "MLE_DEBUG_BUILD", "DEBUG" }
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines { "MLE_RELEASE_BUILD", "NDEBUG" }
		flags { "LinkTimeOptimization" }
		runtime "Release"
		optimize "speed"
		intrinsics "on"

	filter "configurations:Distribution"
		defines {  "MLE_DISTRIBUTION_BUILD", "NDEBUG" }
		flags { "LinkTimeOptimization" }
		runtime "Release"
		optimize "speed"
		intrinsics "on"