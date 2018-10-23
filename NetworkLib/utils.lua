function isModuleAvailable(name)
  if package.loaded[name] then
    return true
  else
    for _, searcher in ipairs(package.searchers or package.loaders) do
      local loader = searcher(name)
      if type(loader) == 'function' then
        package.preload[name] = loader
        return true
      end
    end
    return false
  end
end

function buildPathName(path)
	local chars = {"/", "\\", " "}
	for _,c in pairs(chars) do
		path = string.gsub(path, c, "_")
	end
	return path
end