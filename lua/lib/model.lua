local class = require("30log")
local ffi = require("ffi")
local d2l = require("dtl")

local _M = {}

local assetsDir = "../assets/"

local modelCache = {}

function _M.getModel(filename)
	local assetPath = assetsDir..filename
	if modelCache[assetPath] then
		return modelCache[assetPath]
	end
	local f = io.open(assetPath, "r")
	if not f then error("Could not load model '"..assetPath.."', failed to open file.") end
	f:close()
	local mesh = ffi.new("struct mesh[1]")
	dtl.loadmesh("../assets/sphere.x", mesh)
	modelCache[assetPath] = mesh
	return mesh
end

return _M