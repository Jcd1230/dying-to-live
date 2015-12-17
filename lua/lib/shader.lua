local ffi = require("ffi")
local dtl = require("dtl")
local render = require("render")
local Shader = {}

local _M = {
	__index = Shader
}

_M.VERTEX_EXT = '.vs'
_M.FRAGMENT_EXT = '.fs'

local init = false

_M.init = function()
	if init then return end
	init = true
	_M.UNIFORM_1F = dtl.gl_Uniform1f
	--_M.UNIFORM_2F = dtl.gl_Uniform2f TODO: Implement all uniform types
	--_M.UNIFORM_3F = dtl.gl_Uniform3f
	--_M.UNIFORM_4F = dtl.gl_Uniform4f

	_M.UNIFORM_1I = dtl.gl_Uniform1i
	--_M.UNIFORM_2I = dtl.gl_Uniform2i
	--_M.UNIFORM_3I = dtl.gl_Uniform3i
	--_M.UNIFORM_4I = dtl.gl_Uniform4i

	_M.UNIFORM_1UI = dtl.gl_Uniform1ui
	--_M.UNIFORM_2UI = dtl.gl_Uniform2ui
	--_M.UNIFORM_3UI = dtl.gl_Uniform3ui
	--_M.UNIFORM_4UI = dtl.gl_Uniform4ui

	_M.UNIFORM_1FV = dtl.gl_Uniform1fv
	_M.UNIFORM_2FV = dtl.gl_Uniform2fv
	_M.UNIFORM_3FV = dtl.gl_Uniform3fv
	_M.UNIFORM_4FV = dtl.gl_Uniform4fv

	_M.UNIFORM_MAT2F = dtl.gl_UniformMatrix2fv
	_M.UNIFORM_MAT3F = dtl.gl_UniformMatrix3fv 
	_M.UNIFORM_MAT4F = dtl.gl_UniformMatrix4fv
end

Shader.newUniform = function(self, name, _type, count, transpose)
	
	if not _type then error("Invalid uniform type.", 2) end
	
	local uniform_type = ( -- What function signature does this use? 
			--(matrix (array + inverse bit), single by value, count + reference (float array))
			_type == _M.UNIFORM_MAT2F or
			_type == _M.UNIFORM_MAT3F or
			_type == _M.UNIFORM_MAT4F
		) and render.sig_matrix or
		(
			_type == _M.UNIFORM_1F or
			_type == _M.UNIFORM_1I or
			_type == _M.UNIFORM_1UI
		) and render.sig_single
		or render.sig_array
	
	-- Allow first value in newUniform for single values
	local value
	if count ~= nil and uniform_type == render.sig_single then
		value = count
		count = 1
	end
	
	self.uniforms[name] = {
		dtl.gl_GetUniformLocation(self.programID, name), --location
		_type, --uniform function
		uniform_type,
		count or 1,
		transpose or 0
	}
	
	-- Set first value in newUniform for single values
	if value then
		self:setUniform(name, value)
	end
end

Shader.setUniform = function(self, name, value)
	render.setUniform(self, name, value) --Pass to renderer since uniforms are global information
	--The renderer performs additional error checking
end

Shader.new = function(vertfile, fragfile)
	if type(vertfile) == "table" then
		error("Tried creating new Shader from existing one, use Shader.new()", 2)
	end
	local s = {}
	s.uniforms = {}
	if not fragfile then
		vertfile = vertfile .. _M.VERTEX_EXT
		fragfile = vertfile .. _M.FRAGMENT_EXT
	end
	s.programID = dtl.load_shaders(vertfile, fragfile)
	setmetatable(s, _M)
	return s
end

setmetatable(_M, { 
	__call = function(self, v, f) return Shader.new(v, f) end
})

return _M
