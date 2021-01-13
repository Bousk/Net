require "../NetworkLib"

function CreateNetworkLib_Test(baseLibFolder)
	baseTestFolder = baseLibFolder .. "Tests/";
	project( "NetworkLib_Test" )
		kind "ConsoleApp"
		language "C++"
		-- architecture "x64"
		targetdir ( baseTestFolder .. "builds/%{cfg.buildcfg}" )
		targetprefix ""
		targetname "NetworkLib_Test"

		files {
			baseTestFolder .. "src/**.hpp",
			baseTestFolder .. "src/**.cpp",
			baseTestFolder .. "src/**.inl"
		}
		includedirs {
			baseTestFolder .. "src",
			baseLibFolder .. "src"
		}

		filter "configurations:Debug"
			defines { "DEBUG" }
			symbols "On"

		filter "configurations:Release"
			defines { "NDEBUG" }
			optimize "On"
		filter {}

		links { "Network" }
end

workspace "NetworkLib_Test"
	configurations { "Debug", "Release" }
	architecture "x64"
	cppdialect "c++17"
	location("./Projects/" .. _ACTION)
CreateNetworkLib_Test("../")
CreateNetworkLib("../", "../tmp/builds/files/" .. _ACTION .. "/%{cfg.buildcfg}")
