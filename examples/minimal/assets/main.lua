local function assets_searcher(libraryname)
	local paths_sep = package.config:sub(3, 3)
	local name_rep = package.config:sub(5, 5)
	local paths_sep_pattern = string.format("([^%s]+)", paths_sep)
	libraryname = libraryname:gsub("%.", '/')

	local err = {}
	for path in string.gmatch(package.apath, paths_sep_pattern) do
		local path = path:gsub(name_rep, libraryname)

		local file_data = FS.readAsset(path)

		if file_data then
			local module_func, errmsg = load(file_data, "@"..path)
			if module_func then
				return module_func, path
			else
				return nil, errmsg
			end
		else
			table.insert(err, ("no file 'asset:%s'"):format(path))
		end
	end

	return nil, table.concat(err, "\n\t")
end
table.insert(package.searchers, 2, assets_searcher)
package.apath = ":?.lua;:?/init.lua" -- for assets_searcher (with `:` and without `./`)

local HelloButton = require "hello_button"

local win = Window("Test1", 854, 500)

win:inflateView(Centered (
    Vertical {
        Label "Hello world!",
        HelloButton(),
    }
))

print("Win dpi scale", win:getDpiRatio())
win:show()