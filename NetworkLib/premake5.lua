local utils = require"utils"
require "NetworkLib"

workspace "NetworkLib"
	configurations { "Debug", "Release" }
	architecture "x64"
	filter "system:windows"
		systemversion "10.0.15063.0"

CreateNetworkLib("./")

function CreateProject(path)
	local prettypath = string.gsub(path, "/", "_")
	prettypath = string.gsub(prettypath, "\\", "_")
	project(prettypath)
		kind "ConsoleApp"
		language "C++"
		location(path)
		targetdir (path .. "/%{cfg.buildcfg}")
		files { path.."/*.hpp", path.."/*.cpp", path.."/*.inl" }
		links { "NetworkLib" }
		includedirs { "src" }
		filter "configurations:Debug"
			defines { "DEBUG" }
			symbols "On"
		filter "configurations:Release"
			defines { "NDEBUG" }
			optimize "On"
end
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

	function CreateSamples(basepath)
		group("Samples")
		for path in basepath do
			local fullpath = basepath .. "/" .. path
			if HasMain(fullpath) then
				CreateProject(fullpath)
			else
				CreateSamples(fullpath)
			end
		end
		group("")
	end

	local SamplesDirectory = "Samples"
	local SamplesPaths = GetDirs(SamplesDirectory)
	CreateSamples(SamplesPaths)
else
	print("lfs not found. Unable to create all UT projects.")
	print("Creating default sample projects...")
	group("Samples")
	CreateProject("Samples/TCP/Client")
	CreateProject("Samples/TCP/Server")
	CreateProject("Samples/UDP/Hello World")
end