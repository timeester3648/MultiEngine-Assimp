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
		"%{IncludeDir.assimp}",
		"%{IncludeDir.intrinsics}"
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
		"./contrib/zlib/**.cc",
		"./contrib/zlib/**.cpp"
	}

	defines {
		"OPENDDL_STATIC_LIBARY",
		"ASSIMP_BUILD_NO_EXPORT",
		"ASSIMP_BUILD_NO_OWN_ZLIB",
		"ASSIMP_BUILD_NO_C4D_IMPORTER",
		"ASSIMP_BUILD_NO_IFC_IMPORTER",
		"ASSIMP_BUILD_NO_M3D_IMPORTER"
	}

	filter "action:vs*"
		buildoptions { "/bigobj" }
		disablewarnings { "4244" }
		defines { "_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING" }

	filter "system:windows"
		defines { "_CRT_SECURE_NO_WARNINGS" }