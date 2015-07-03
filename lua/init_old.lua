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

local info = ffi.new("struct SDL_Info[1]")

dtl.initSDL(info)
--dtl.initOGL()
if dtl.ogl_LoadFunctions() == dtl.ogl_LOAD_FAILED then
	error("Could not load OpenGL functions")
end
shader.init()

local meshes = ffi.new("struct mesh[1]")
print("Loading mesh...")
dtl.loadmesh("../test.obj", meshes)
print("Loading texture...")
local tex_2d = soil.SOIL_load_OGL_texture(
	"../distance.png",
	soil.SOIL_LOAD_AUTO,
	soil.SOIL_CREATE_NEW_ID,
	bit.bor(soil.SOIL_FLAG_MIPMAPS, soil.SOIL_FLAG_INVERT_Y)
)

print("Creating framebuffers")
-- The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
local FramebufferName = ffi.new("GLuint[1]")
FramebufferName[0] = 0
dtl.gl_GenFramebuffers(1, FramebufferName)
dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, FramebufferName[0])


-- The texture we're going to render to
local renderedTexture = ffi.new("GLuint[1]")
dtl.gl_GenTextures(1, renderedTexture)
 
-- "Bind" the newly created texture : all future texture functions will modify this texture
dtl.gl_BindTexture(GL.TEXTURE_2D, renderedTexture[0])
 
-- Give an empty image to OpenGL ( the last "0" )
dtl.gl_TexImage2D(GL.TEXTURE_2D, 0, GL.RGB, 1280, 720, 0, GL.RGB, GL.UNSIGNED_BYTE, nil)
 
--Poor filtering. Needed !
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MAG_FILTER, GL.NEAREST)
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MIN_FILTER, GL.NEAREST)

-- The depth buffer
local depthrenderbuffer = ffi.new("GLuint[1]")
dtl.gl_GenRenderbuffers(1, depthrenderbuffer)
dtl.gl_BindRenderbuffer(GL.RENDERBUFFER, depthrenderbuffer[0])
dtl.gl_RenderbufferStorage(GL.RENDERBUFFER, GL.DEPTH_COMPONENT, 1024, 768)
dtl.gl_FramebufferRenderbuffer(GL.FRAMEBUFFER, GL.DEPTH_ATTACHMENT, GL.RENDERBUFFER, depthrenderbuffer[0])

-- Set "renderedTexture" as our colour attachement #0
dtl.gl_FramebufferTexture(GL.FRAMEBUFFER, GL.COLOR_ATTACHMENT0, renderedTexture[0], 0)
 
-- Set the list of draw buffers.
local drawBuffers = ffi.new("GLenum[1]")
drawBuffers[0] = GL.COLOR_ATTACHMENT0
--GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
dtl.gl_DrawBuffers(1, drawBuffers) -- "1" is the size of DrawBuffers

-- Always check that our framebuffer is ok
if (dtl.gl_CheckFramebufferStatus(GL.FRAMEBUFFER) ~= GL.FRAMEBUFFER_COMPLETE) then
	error("Could not create framebuffer!")
end

print("Loading shaders")
local mainShader = shader("../shaders/main.vert", "../shaders/main.frag")

local cam = {
	pos = vec3(),
	lookat = vec3(),
	up = vec3()
}
cam.pos[0] = { 3, 3, 3 }
cam.lookat[0] = { 0, 0, 0 }
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
--dtl.gl_UniformMatrix4fv(matID, 1, GL.FALSE, ffi.cast("const float *", mvp))
local sampler = ffi.new("GLuint[1]")
dtl.gl_GenSamplers(1, sampler)
dtl.gl_ActiveTexture(GL.TEXTURE0)
dtl.gl_BindTexture(GL.TEXTURE_2D, tex_2d)
dtl.gl_BindSampler(0, sampler[0])
dtl.gl_Enable(GL.DEPTH_TEST)
dtl.gl_DepthFunc(GL.LESS)

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

while dtl.keystate.sp == 0 do
	local event = ffi.new("SDL_Event[1]")
	dtl.handle_events(event)
	mainShader:setUniform("time", os.clock()*60)
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
	--print(cam_right[0].x, cam_right[0].y, cam_right[0].z)
	vec3.scale(cam_motion, cam_motion, 0.1)
	vec3.add(cam.pos, cam.pos, cam_motion)
	--quat.rotationPYR(cam_angle. dtl.cam.pitch, dtl.cam.yaw, 0)
	--cam_forward:quatForwardVec3RH(cam_angle)
	--cam.up:quatUpVec3(cam_angle)
	dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, FramebufferName)
	render.clear()
	
	dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, nil)
	render.clear()
	
	vec3.scale(cam.lookat, cam_forward, 10)
	vec3.add(cam.lookat, cam.lookat, cam.pos)
	mat4.lookAt(mat4.iden(view), cam.pos, cam.lookat, cam.up)
	mat4.iden(mvp)
	mat4.mul(mvp, model, mvp)
	mat4.mul(mvp, view, mvp)
	mat4.mul(mvp, projection, mvp)
	mat4.inverse(mvpinv, mvp)
	mainShader:setUniform("MVP", mvp)
	--dtl.gl_UniformMatrix4fv(matID, 1, 0, ffi.cast("const float *", mvp))
	mainShader:setUniform("MVPInv", mvpinv)
	--dtl.gl_UniformMatrix4fv(matInvID, 1, 0, ffi.cast("const float *", mvpinv))
	dtl.gl_BindVertexArray(meshes[0].vao)
	dtl.gl_BindBuffer(GL.ELEMENT_ARRAY_BUFFER, meshes[0].element_buffer)	
	dtl.gl_DrawElements(GL.TRIANGLES,
		meshes[0].n_indices,
		GL.UNSIGNED_INT,
		nil
	)
	
	sdl.SDL_GL_SwapWindow(info[0].window)
end

dtl.gl_DisableVertexAttribArray(0)
dtl.cleanupSDL(info[0])
--ffi.C.free(cam.pos)