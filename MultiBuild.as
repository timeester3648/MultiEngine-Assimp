void main(MultiBuild::Workspace& workspace) {	
	auto project = workspace.create_project(".");
	auto properties = project.properties();

	properties.name("Assimp");
	properties.binary_object_kind(MultiBuild::BinaryObjectKind::eStaticLib);
	properties.project_includes("zlib");
	project.license("./LICENSE");

	project.include_own_required_includes(true);
	project.add_required_project_include({
		"./include"
	});

	properties.defines({
		"OPENDDL_STATIC_LIBARY",
		"ASSIMP_BUILD_NO_EXPORT",
		"ASSIMP_BUILD_NO_OWN_ZLIB",
		"ASSIMP_BUILD_NO_C4D_IMPORTER",
		"ASSIMP_BUILD_NO_IFC_IMPORTER",
		"ASSIMP_BUILD_NO_M3D_IMPORTER"
	});

	properties.files({
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
	});

	properties.include_directories({
		".",
		"./code",
		"./include",
		"./contrib",
		"./contrib/unzip",
		"./contrib/irrXML",
		"./contrib/pugixml/src",
		"./contrib/utf8cpp/source",
		"./contrib/rapidjson/include",
		"./contrib/openddlparser/include"
	});

	properties.excluded_files({
		"./contrib/zlib/*.c",
		"./contrib/draco/**.cc",
		"./contrib/googletest/**.cc",

		"./code/AssetLib/C4D/**.cpp",
		"./code/AssetLib/IFC/**.cpp",
		"./code/AssetLib/M3D/**.cpp",

		"./contrib/zip/**.c",

		"./contrib/zlib/**.c",
		"./contrib/zlib/**.cc",
		"./contrib/zlib/**.cpp"
	});

	{
		MultiBuild::ScopedFilter _(workspace, "project.compiler:VisualCpp");
		properties.build_options({ "/bigobj" });
		properties.disable_warnings({ "4244" });
		properties.defines("_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING");
	}

	{
		MultiBuild::ScopedFilter _(workspace, "config.platform:Windows");
		properties.defines({
			"_CRT_SECURE_NO_WARNINGS"
		});
	}
}