local ffi = require("ffi")
local dtl = require("dtl")

local Shader = {}

local _M = {
	__index = Shader
}

_M.VERTEX_EXT = '.vs'
_M.FRAGMENT_EXT = '.fs'

_M.init = function()
	_M.UNIFORM_1F = dtl.gl_Uniform1f
	--_M.UNIFORM_2F = dtl.gl_Uniform2f
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
	self.uniforms[name] = {
		dtl.gl_GetUniformLocation(self.programID, name), --location
		_type, --uniform function
		( -- What function signature does this use? (1 = matrix, 2 = single by value, nil = count + reference)
			_type == _M.UNIFORM_MAT2F or
			_type == _M.UNIFORM_MAT3F or
			_type == _M.UNIFORM_MAT4F
		) and 1 or
		(
			_type == _M.UNIFORM_1F or
			_type == _M.UNIFORM_1I or
			_type == _M.UNIFORM_1UI
		) and 2,
		count or 1,
		transpose or 0
	}
end

Shader.setUniform = function(self, name, value)
	local uni = self.uniforms[name]
	if not uni then error("Tried to set uniform '"..name.."' that hasnt been initialized.", 2) end
	if uni[3] == 1 then -- uni[3] == 1 -> Matrix Uniform
		uni[2](uni[1], uni[4], uni[5], ffi.cast("const float *", value))
	elseif uni[3] == 2 then
		uni[2](uni[1], value)
	else
		uni[2](uni[1], uni[4], ffi.cast("const float *", value))
	end
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