local class = require("30log")
local ffi = require("ffi")
local model = require("model")
local km = require("km")
local texture = require("texture")

local _M = class("Entity")

local entities = {}

function _M.getAll()
	return entities
end

function _M:init()
	self:setModel("error.x")
	self.tex_diffuse = texture.getTextureID("error.png")
	self.pos = { x = 0, y = 0, z = 0}
	self.rot = { p = 0, y = 0, r = 0}
	self.matrix = km.mat4.iden(km.mat4())
	self._matrix_trans = km.mat4()
	self._matrix_scale = km.mat4()
	self._matrix_rotate = km.mat4()
	
	table.insert(entities, self)
end

function _M:calculateMatrix()
	km.mat4.iden(self.matrix)
	km.mat4.translation(self._matrix_trans, self.pos.x, self.pos.y, self.pos.z)
	km.mat4.scaling(self._matrix_scale, 1, 1, 1)
	km.mat4.rotationYPR(self._matrix_rotate, self.rot.p, self.rot.y, self.rot.r)
	km.mat4.mul(self.matrix, self._matrix_rotate, self.matrix)
	km.mat4.mul(self.matrix, self._matrix_scale, self.matrix)
	km.mat4.mul(self.matrix, self._matrix_trans, self.matrix)
	self.matrixInvalidated = false
end

function _M:getMatrix()
	if self.matrixInvalidated then
		self:calculateMatrix()
	end
	return self.matrix
end

function _M:setPos(x, y, z)
	self.matrixInvalidated = true
	self.pos.x = x
	self.pos.y = y
	self.pos.z = z
end

function _M:setRotationPYR(pitch, yaw, roll)
	self.matrixInvalidated = true
	self.rot.p = pitch
	self.rot.y = roll -- YAW and ROLL switched because kazmath assumes yaw rotates
	self.rot.r = yaw -- around Y axis not Z
end

function _M:setModel(modelpath)
	self.mesh = model.getModel(modelpath)
	self.modelpath = modelpath
	return self
end

function _M:setDiffuse(texturepath)
	self.tex_diffuse = texture.getTextureID(texturepath)
	self.tex_diffuse_path = texturepath
	return self
end

return _M