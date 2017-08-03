local utils = require"utils"

workspace "NetworkLib"
   configurations { "Debug", "Release" }

project "NetworkLib"
   kind "StaticLib"
   language "C++"
   targetdir "builds/%{cfg.buildcfg}"
   targetprefix ""
   targetname "Network"

   files { "src/**.hpp", "src/**.cpp" }
   includedirs { "src" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

if isModuleAvailable("lfs") then
	local lfs = require"lfs"
	function GetDirs(path)
		local dirs
		local i = 1
		for file in lfs.dir(path) do
			if file ~= "." and file ~= ".." then
				local fullpath = path .. "/" .. file
				local attributes = lfs.attributes(fullpath)
				if attributes.mode == "directory" then
					dirs[i] = fullpath
					i = i + 1
				end
			end
		end
		return dirs
	end
	function HasMain(path)
		local fullpath = path .. "/main.cpp"
		return lfs.attributes(fullpath) ~= nil
	end

	function CreateProject(path)
		local prettypath = string.replace(path, "/", "_")
		prettypath = string.replace(prettypath, "\\", "_")
		project{prettypath}
			kind "ConsoleApp"
			language "C++"
			targetdir {path .. "/%{cfg.buildcfg}"}
			files { "**.hpp", "**.cpp" }
			includedirs { "src" }
			links { "Network" }
			filter "configurations:Debug"
				defines { "DEBUG" }
				symbols "On"
			filter "configurations:Release"
				defines { "NDEBUG" }
				optimize "On"
	end

	function CreateSamples(basepath)
		for path in basepath do
			local fullpath = basepath .. "/" .. path
			if HasMain(fullpath) then
				CreateProject(fullpath)
			else
				CreateSamples(fullpath)
			end
		end
	end

	local SamplesDirectory = "Samples"
	local SamplesPaths = GetDirs(SamplesDirectory)
	CreateSamples(SamplesPaths)
else
	print("lfs not found. Unable to create UT projects.")
end