local class = require("30log")
local ffi = require("ffi")
local soil = require("soil")
local dtl = require("dtl")
local GL = require("gl")
local function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

local _M = {}

local assetsDir = "../assets/"

local textureBank = {}

function _M.getTextureID(assetPath)
	if not file_exists(assetsDir..assetPath) then
		error("COULD NOT FIND TEXTURE '"..assetPath.."'!!!")
	end
	local tID = soil.SOIL_load_OGL_texture(
		assetsDir..assetPath,
		soil.SOIL_LOAD_AUTO,
		soil.SOIL_CREATE_NEW_ID,
		bit.bor(soil.SOIL_FLAG_MIPMAPS, soil.SOIL_FLAG_INVERT_Y)
	)
	dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_WRAP_S, GL.REPEAT)
	dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_WRAP_T, GL.REPEAT)
	
	textureBank[assetPath] = tID;
	return tID;
end

return _M