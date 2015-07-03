local M = {}
local ffi = require"ffi"

local dtl, sdl, km, soil = require("dtl"), require("sdl"), require("km"), require("soil")
local shader, render, GL = require("shader"), require("render"), require("gl")
local vec3, mat4, quat =  km.vec3, km.mat4, km.quat
ffi.cdef[[
int main(int argc, char *argv[]);
void initOGL();
]]
--dtl.main(0,nil)

--[[function glerror(s)
	local e = dtl.gl_GetError()
	if e > 0 then
		error("OpenGL returned error: "..dtl.gl_GetError, 2)
	end
end]]

local info = ffi.new("struct SDL_Info[1]")

dtl.initSDL(info)
--dtl.initOGL()
if dtl.ogl_LoadFunctions() == dtl.ogl_LOAD_FAILED then
	error("Could not load OpenGL functions")
end
shader.init()

print("Loading mesh...")
local cube = ffi.new("struct mesh[1]")
local quad = ffi.new("struct mesh[1]")
dtl.loadmesh("../cube.obj", cube)
dtl.loadmesh("../quad.obj", quad)
print("Loading texture...")
local tex_2d = soil.SOIL_load_OGL_texture(
	"../gravatar512.png",
	soil.SOIL_LOAD_AUTO,
	soil.SOIL_CREATE_NEW_ID,
	bit.bor(soil.SOIL_FLAG_MIPMAPS, soil.SOIL_FLAG_INVERT_Y)
)

print("Creating framebuffer")
-- The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
local fbo = ffi.new("GLuint[1]")
dtl.gl_GenFramebuffers(1, fbo)

dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, fbo[0])

print(dtl.gl_CheckFramebufferStatus(fbo[0]))

-- The texture we're going to render to
local texRenderColor = ffi.new("GLuint[1]")
dtl.gl_GenTextures(1, texRenderColor)
local texRenderSpecular = ffi.new("GLuint[1]")

-- "Bind" the newly created texture : all future texture functions will modify this texture
dtl.gl_BindTexture(GL.TEXTURE_2D, texRenderColor[0])
-- Give an empty image to OpenGL ( the last "0" )
dtl.gl_TexImage2D(GL.TEXTURE_2D, 0, GL.RGB, 1280, 720, 0, GL.RGB, GL.UNSIGNED_BYTE, nil)
--Poor filtering. Needed !
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MAG_FILTER, GL.NEAREST)
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MIN_FILTER, GL.NEAREST)

-- Set "renderedTexture" as our colour attachement #0
dtl.gl_FramebufferTexture2D(GL.FRAMEBUFFER, GL.COLOR_ATTACHMENT0, GL.TEXTURE_2D, texRenderColor[0], 0)

-- The depth buffer
local rboDepthStencil = ffi.new("GLuint[1]")
dtl.gl_GenRenderbuffers(1, rboDepthStencil)
dtl.gl_BindRenderbuffer(GL.RENDERBUFFER, rboDepthStencil[0])
dtl.gl_RenderbufferStorage(GL.RENDERBUFFER, GL.DEPTH24_STENCIL8, 1280, 720)
dtl.gl_FramebufferRenderbuffer(GL.FRAMEBUFFER, GL.DEPTH_STENCIL_ATTACHMENT, GL.RENDERBUFFER, rboDepthStencil[0])
 
-- Set the list of draw buffers.
local drawBuffers = ffi.new("GLenum[1]")
drawBuffers[0] = GL.COLOR_ATTACHMENT0
dtl.gl_DrawBuffers(1, drawBuffers) -- "1" is the size of DrawBuffers

-- Always check that our framebuffer is ok
if (dtl.gl_CheckFramebufferStatus(GL.FRAMEBUFFER) ~= GL.FRAMEBUFFER_COMPLETE) then
	error("Could not create framebuffer!")
end

print("Loading shaders")
local mainShader = shader("../shaders/main.vert", "../shaders/main.frag")
local quadShader = shader("../shaders/draw_quad.vert", "../shaders/draw_quad.frag")

local cam = {
	pos = vec3(),
	lookat = vec3(),
	up = vec3()
}
cam.pos[0] = { 0, 1, 3 }
cam.lookat[0] = { 0, 0, -1 }
cam.up[0] = { 0, 0, 1 }
print()
local model_trans = mat4.translation(mat4.iden(mat4()), 0, 0, 0)
local model_scale = mat4.scaling(mat4.iden(mat4()), 1, 1, 1)
local model_rotate = mat4.rotationYPR(mat4.iden(mat4()), 0, 0, 0)
local model = mat4.iden(mat4())
mat4.mul(model, model_rotate, model)
mat4.mul(model, model_scale, model)
mat4.mul(model, model_trans, model)
local projection = mat4.perspectiveProjection(mat4(), 70, 16/9, 0.001, 100)
local view = mat4.lookAt(mat4(), cam.pos, cam.lookat, cam.up)
local mvp = mat4.iden(mat4())
local mvpinv = mat4()
mat4.mul(mvp, model, mvp)
mat4.mul(mvp, view, mvp)
mat4.mul(mvp, projection, mvp)

render.clear()
render.setShader(mainShader)
mainShader:newUniform("MVP", shader.UNIFORM_MAT4F)
mainShader:newUniform("MVPInv", shader.UNIFORM_MAT4F)
mainShader:newUniform("time", shader.UNIFORM_1F)
mainShader:newUniform("diffuse_color", shader.UNIFORM_1I)

mainShader:setUniform("diffuse_color", 0)
mainShader:setUniform("MVP", mvp)

render.setShader(quadShader)
quadShader:newUniform("time", shader.UNIFORM_1F)
quadShader:newUniform("renderColor", shader.UNIFORM_1I)
quadShader:setUniform("renderColor", 0)

local cam_angle = quat.rotationPYR(quat(), 0, 0, 0)
local cam_forward = vec3()
local cam_right = vec3()
local cam_motion = vec3.zero(vec3())
dtl.keystate.w = 0
dtl.keystate.a = 0
dtl.keystate.s = 0
dtl.keystate.d = 0
dtl.keystate.sp = 0
dtl.cam.yaw = 0
dtl.cam.pitch = 0
dtl.gl_Enable(GL.DEPTH_TEST)
dtl.gl_DepthFunc(GL.LESS)
print(dtl.gl_CheckFramebufferStatus(fbo[0]))
while dtl.keystate.sp == 0 do
	local event = ffi.new("SDL_Event[1]")
	dtl.handle_events(event)
	
	quat.rotationPYR(cam_angle, dtl.cam.pitch, dtl.cam.yaw, 0)
	vec3.quatForwardVec3RH(cam_forward, cam_angle)
	vec3.quatUpVec3(cam.up, cam_angle)
	vec3.quatRightVec3(cam_right, cam_angle)
	vec3.zero(cam_motion)
	if dtl.keystate.w == 1 then
		vec3.add(cam_motion, cam_motion, cam_forward)
	end
	if dtl.keystate.a == 1 then
		vec3.sub(cam_motion, cam_motion, cam_right)
	end
	if dtl.keystate.s == 1 then
		vec3.sub(cam_motion, cam_motion, cam_forward)
	end
	if dtl.keystate.d == 1 then
		vec3.add(cam_motion, cam_motion, cam_right)
	end
	vec3.scale(cam_motion, cam_motion, 0.1)
	vec3.add(cam.pos, cam.pos, cam_motion)
	vec3.scale(cam.lookat, cam_forward, 10)
	vec3.add(cam.lookat, cam.lookat, cam.pos)
	mat4.lookAt(mat4.iden(view), cam.pos, cam.lookat, cam.up)
	mat4.iden(mvp)
	mat4.mul(mvp, model, mvp)
	mat4.mul(mvp, view, mvp)
	mat4.mul(mvp, projection, mvp)
	mat4.inverse(mvpinv, mvp)
	
	render.setShader(mainShader)
	mainShader:setUniform("MVP", mvp)
	mainShader:setUniform("MVPInv", mvpinv)
	dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, fbo[0])
	render.clear()
	
	dtl.gl_BindTexture(GL.TEXTURE_2D, tex_2d)
	dtl.gl_BindVertexArray(cube[0].vao)
	--dtl.gl_BindBuffer(GL.ELEMENT_ARRAY_BUFFER, cube[0].element_buffer)	
	dtl.gl_DrawElements(GL.TRIANGLES,
		cube[0].n_indices,
		GL.UNSIGNED_INT,
		nil
	)
	
	dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, 0)
	render.clear()
	render.setShader(quadShader)
	quadShader:setUniform("time", os.clock()*50)
	dtl.gl_BindTexture(GL.TEXTURE_2D, texRenderColor[1])

	dtl.gl_BindVertexArray(quad[0].vao)
	--dtl.gl_BindBuffer(GL.ELEMENT_ARRAY_BUFFER, quad[0].element_buffer)	
	dtl.gl_DrawElements(GL.TRIANGLES,
		quad[0].n_indices,
		GL.UNSIGNED_INT,
		nil
	)
  
	sdl.SDL_GL_SwapWindow(info[0].window)
end

dtl.gl_DisableVertexAttribArray(0)
dtl.cleanupSDL(info[0])
--ffi.C.free(cam.pos)
