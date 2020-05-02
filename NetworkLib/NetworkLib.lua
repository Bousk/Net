function CreateNetworkLib(baseLibFolder)
	print("network lib : " .. baseLibFolder)
	project "Network"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"
		-- architecture "x64"
		targetdir ( baseLibFolder .. "builds/%{cfg.buildcfg}" )
		filter "toolset:codeblocks"
			targetprefix "lib"
		filter {}
		targetname "Network"
		
		files {
			baseLibFolder .. "src/**.hpp",
			baseLibFolder .. "src/**.cpp",
			baseLibFolder .. "src/**.inl"
		}
		includedirs { baseLibFolder .. "src" }
		
		filter "configurations:Debug"
			defines { "DEBUG" }
			symbols "On"
		
		filter "configurations:Release"
			defines { "NDEBUG" }
			optimize "On"
		filter {}
end
