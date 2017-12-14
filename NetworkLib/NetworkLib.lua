function CreateNetworkLib(baseLibFolder)
	project "NetworkLib"
	   kind "StaticLib"
	   language "C++"
	   targetdir ( baseLibFolder .. "builds/%{cfg.buildcfg}" )
	   targetprefix ""
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
end