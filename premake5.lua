include "../../premake/common_premake_defines.lua"
		
project "Assimp"
	kind "StaticLib"
	language "C++"
	cppdialect "C++latest"
	cdialect "C17"
	targetname "%{prj.name}"
	inlining "Auto"

	files {
		"./**.c",
		"./**.h",
		"./**.cc",
		"./**.inl",
		"./**.hpp",
		"./**.cpp"
	}

	includedirs {
		".",
		"./code",
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