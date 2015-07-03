-- Render
local ffi = require("ffi")
local dtl = require("dtl")
local GL = require("gl")
local _M = {}

_M.setShader = function(shader)
	dtl.gl_UseProgram(shader.programID)
end

_M.clear = function(flags)
	dtl.gl_Clear(flags or bit.bor(GL.COLOR_BUFFER_BIT, GL.DEPTH_BUFFER_BIT))
end

return _M