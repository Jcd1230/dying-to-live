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

local rboDepthStencil, drawBuffers, texRenders, frameRes, fbo


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

_M.init = function()
	-- The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	fbo = ffi.new("GLuint[1]")
	dtl.gl_GenFramebuffers(1, fbo)
	dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, fbo[0])


	-- The texture we're going to render to
	frameRes = { w = 1280, h = 720 }
	texRenders = ffi.new("GLuint[4]")
	dtl.gl_GenTextures(4, texRenders)
	_M.GBuffer = {
		Color = texRenders[0],
		Normal = texRenders[1],
		Material = texRenders[2],
		Depth = texRenders[3]
	}

	------------------ COLOR
	dtl.gl_BindTexture(GL.TEXTURE_2D, _M.GBuffer.Color)
	dtl.gl_TexImage2D(GL.TEXTURE_2D, 0, GL.RGB, frameRes.w, frameRes.h, 0, GL.RGB, GL.UNSIGNED_BYTE, nil)
	-- Nearest Filtering
	dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MAG_FILTER, GL.NEAREST)
	dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MIN_FILTER, GL.NEAREST)

	------------------ NORMAL
	dtl.gl_BindTexture(GL.TEXTURE_2D, _M.GBuffer.Normal)
	dtl.gl_TexImage2D(GL.TEXTURE_2D, 0, GL.RGB, frameRes.w, frameRes.h, 0, GL.RGB, GL.UNSIGNED_BYTE, nil)
	-- Nearest Filtering
	dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MAG_FILTER, GL.NEAREST)
	dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MIN_FILTER, GL.NEAREST)

	------------------ Material / Specular
	dtl.gl_BindTexture(GL.TEXTURE_2D, _M.GBuffer.Material)
	dtl.gl_TexImage2D(GL.TEXTURE_2D, 0, GL.RGB, frameRes.w, frameRes.h, 0, GL.RGB, GL.UNSIGNED_BYTE, nil)
	-- Nearest Filtering
	dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MAG_FILTER, GL.NEAREST)
	dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MIN_FILTER, GL.NEAREST)

	------------------ DEPTH
	dtl.gl_BindTexture(GL.TEXTURE_2D, _M.GBuffer.Depth)
	--dtl.gl_TexImage2D(GL.TEXTURE_2D, 0, GL.RGB, frameRes.w, frameRes.h, 0, GL.RGB, GL.UNSIGNED_BYTE, nil)
	dtl.gl_TexImage2D(GL.TEXTURE_2D, 0, GL.R32F, frameRes.w, frameRes.h, 0, GL.RED, GL.FLOAT, nil)
	-- Nearest Filtering -- Might want to make linear for depth
	dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MAG_FILTER, GL.NEAREST)
	dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MIN_FILTER, GL.NEAREST)



	-- Attach buffers
	dtl.gl_FramebufferTexture2D(GL.FRAMEBUFFER, GL.COLOR_ATTACHMENT0, GL.TEXTURE_2D, _M.GBuffer.Color, 0)
	dtl.gl_FramebufferTexture2D(GL.FRAMEBUFFER, GL.COLOR_ATTACHMENT1, GL.TEXTURE_2D, _M.GBuffer.Normal, 0)
	dtl.gl_FramebufferTexture2D(GL.FRAMEBUFFER, GL.COLOR_ATTACHMENT2, GL.TEXTURE_2D, _M.GBuffer.Material, 0)
	dtl.gl_FramebufferTexture2D(GL.FRAMEBUFFER, GL.COLOR_ATTACHMENT3, GL.TEXTURE_2D, _M.GBuffer.Depth, 0)

	--The depth/stencil buffer (for depth/stencil tests)
	--
	rboDepthStencil = ffi.new("GLuint[1]")
	dtl.gl_GenRenderbuffers(1, rboDepthStencil)
	dtl.gl_BindRenderbuffer(GL.RENDERBUFFER, rboDepthStencil[0])
	dtl.gl_RenderbufferStorage(GL.RENDERBUFFER, GL.DEPTH24_STENCIL8, frameRes.w, frameRes.h)
	dtl.gl_FramebufferRenderbuffer(GL.FRAMEBUFFER, GL.DEPTH_STENCIL_ATTACHMENT, GL.RENDERBUFFER, rboDepthStencil[0])
	--]]

	-- Set the list of draw buffers.
	drawBuffers =  ffi.new("GLenum[4]")

	drawBuffers[0] = GL.COLOR_ATTACHMENT0
	drawBuffers[1] = GL.COLOR_ATTACHMENT1
	drawBuffers[2] = GL.COLOR_ATTACHMENT2
	drawBuffers[3] = GL.COLOR_ATTACHMENT3

	dtl.gl_DrawBuffers(4, drawBuffers) -- N = number of buffers

	-- Always check that our framebuffer is ok
	if (dtl.gl_CheckFramebufferStatus(GL.FRAMEBUFFER) ~= GL.FRAMEBUFFER_COMPLETE) then
		error("Could not create framebuffer!".. string.format("%x",dtl.gl_CheckFramebufferStatus(GL.FRAMEBUFFER)))
	end
	return fbo
end

return _M