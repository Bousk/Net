require "utils"
require "NetworkLib"

workspace "NetworkLib"
	configurations { "Debug", "Release" }
	architecture "x64"
	location("./Projects/" .. _ACTION)
	filter "system:windows"
		systemversion "10.0.15063.0"
	filter {}
		
CreateNetworkLib("./")

function CreateProject(path)
	local prettypath = buildPathName(path)
	print("Creating project " .. prettypath .. " from " .. path)
	project(prettypath)
		kind "ConsoleApp"
		language "C++"
		-- architecture "x64"
		targetdir (path .. "/%{cfg.buildcfg}")
		files { path.."/*.hpp", path.."/*.cpp", path.."/*.inl" }
		links { "Network" }
		includedirs { path, "./src" }
		filter "configurations:Debug"
			defines { "DEBUG" }
			symbols "On"
		filter "configurations:Release"
			defines { "NDEBUG" }
			optimize "On"
		filter {}
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