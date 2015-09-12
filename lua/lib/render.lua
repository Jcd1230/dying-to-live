-- Render
local ffi = require("ffi")
local dtl = require("dtl")
local GL = require("gl")
local _M = {}

_M.sig_matrix = 1
_M.sig_single = 2
_M.sig_array = 3
local sig_matrix = _M.sig_matrix
local sig_single = _M.sig_single
local sig_array = _M.sig_array

local activeShader = nil

_M.setUniform = function(shader, name, value)
	local uni = shader.uniforms[name]
	
	--Error checking TODO: Move uniform name check to shader?
	if not uni then error("Tried to set uniform '"..name.."' that hasnt been initialized.", 2) end
	if activeShader ~= shader then error("Tried to set value on inactive shader.", 2) end
	
	if uni[3] == sig_matrix then -- uni[3] == 1 -> Matrix Uniform
		uni[2](uni[1], uni[4], uni[5], ffi.cast("const float *", value))
	elseif uni[3] == sig_single then
		uni[2](uni[1], value)
	elseif uni[3] == sig_array then
		uni[2](uni[1], uni[4], ffi.cast("const float *", value))
	end
end

_M.setShader = function(shader)
	activeShader = shader
	dtl.gl_UseProgram(shader.programID)
end

_M.clear = function(flags)
	dtl.gl_Clear(flags or bit.bor(GL.COLOR_BUFFER_BIT, GL.DEPTH_BUFFER_BIT))
end

return _M