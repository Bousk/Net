require "../NetworkLib"

function CreateNetworkLib_Test(baseLibFolder)
	baseTestFolder = baseLibFolder .. "Tests/";
	project( "NetworkLib_Test" )
		kind "ConsoleApp"
		language "C++"
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

		configuration {"*"}
			links { "Network" }
end

workspace "NetworkLib_Test"
	configurations { "Debug", "Release" }
CreateNetworkLib_Test("../")
CreateNetworkLib("../")
