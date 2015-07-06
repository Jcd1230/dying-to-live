local M = {}

local ffi = require"ffi"
local dtl, sdl, km, soil = require("dtl"), require("sdl"), require("km"), require("soil")
local shader, render, GL = require("shader"), require("render"), require("gl")
local vec3, mat3, mat4, quat =  km.vec3, km.mat3, km.mat4, km.quat
ffi.cdef[[
int main(int argc, char *argv[]);
void initOGL();
]]
--dtl.main(0,nil)

local md
if DEBUG then
	md = require("mobdebug")
	md.start("127.0.0.1",8172)
end

local brkpt = DEBUG and md.pause or function()end


local function debugmat(m) 
	local x = {}
	for i = 1,16 do 
		x[i]=(i-1).." = "..m.mat[i-1] 
	end; 
	return x 
end

local function glerror(s)
	local e = dtl.gl_GetError()
	if e > 0 then
		error(string.format("%s OpenGL returned error: 0x%X",s, e), 2)
	end
end

local info = ffi.new("struct SDL_Info[1]")

dtl.initSDL(info)
--dtl.initOGL()
if dtl.ogl_LoadFunctions() == dtl.ogl_LOAD_FAILED then
	error("Could not load OpenGL functions")
end
shader.init()
brkpt()
print("Loading mesh...")
local cube = ffi.new("struct mesh[1]")
local quad = ffi.new("struct mesh[1]")
dtl.loadmesh("../assets/cube_s2.x", cube)
dtl.loadmesh("../assets/quad.obj", quad)
print("Loading texture...")
local tex_diffuse = soil.SOIL_load_OGL_texture(
	"../assets/brick1.png",
	soil.SOIL_LOAD_AUTO,
	soil.SOIL_CREATE_NEW_ID,
	bit.bor(soil.SOIL_FLAG_MIPMAPS, soil.SOIL_FLAG_INVERT_Y)
)
local tex_normal = soil.SOIL_load_OGL_texture(
	"../assets/brick1n.png",
	soil.SOIL_LOAD_AUTO,
	soil.SOIL_CREATE_NEW_ID,
	bit.bor(soil.SOIL_FLAG_MIPMAPS, soil.SOIL_FLAG_INVERT_Y)
)
local tex_material = soil.SOIL_load_OGL_texture(
	"../assets/brick1s.png",
	soil.SOIL_LOAD_AUTO,
	soil.SOIL_CREATE_NEW_ID,
	bit.bor(soil.SOIL_FLAG_MIPMAPS, soil.SOIL_FLAG_INVERT_Y)
)

print("Creating framebuffer")
-- The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
local fbo = ffi.new("GLuint[1]")
dtl.gl_GenFramebuffers(1, fbo)
dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, fbo[0])


-- The texture we're going to render to
local frameRes = { w = 1280, h = 720 }
local texRenders = ffi.new("GLuint[4]")
dtl.gl_GenTextures(4, texRenders)
local texRender = {
	Color = texRenders[0],
	Normal = texRenders[1],
	Material = texRenders[2],
	Depth = texRenders[3]
}

------------------ COLOR
dtl.gl_BindTexture(GL.TEXTURE_2D, texRender.Color)
dtl.gl_TexImage2D(GL.TEXTURE_2D, 0, GL.RGB, frameRes.w, frameRes.h, 0, GL.RGB, GL.UNSIGNED_BYTE, nil)
-- Nearest Filtering
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MAG_FILTER, GL.NEAREST)
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MIN_FILTER, GL.NEAREST)

------------------ NORMAL
dtl.gl_BindTexture(GL.TEXTURE_2D, texRender.Normal)
dtl.gl_TexImage2D(GL.TEXTURE_2D, 0, GL.RGB, frameRes.w, frameRes.h, 0, GL.RGB, GL.UNSIGNED_BYTE, nil)
-- Nearest Filtering
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MAG_FILTER, GL.NEAREST)
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MIN_FILTER, GL.NEAREST)

------------------ Material / Specular
dtl.gl_BindTexture(GL.TEXTURE_2D, texRender.Material)
dtl.gl_TexImage2D(GL.TEXTURE_2D, 0, GL.RGB, frameRes.w, frameRes.h, 0, GL.RGB, GL.UNSIGNED_BYTE, nil)
-- Nearest Filtering
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MAG_FILTER, GL.NEAREST)
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MIN_FILTER, GL.NEAREST)

------------------ DEPTH
dtl.gl_BindTexture(GL.TEXTURE_2D, texRender.Depth)
--dtl.gl_TexImage2D(GL.TEXTURE_2D, 0, GL.RGB, frameRes.w, frameRes.h, 0, GL.RGB, GL.UNSIGNED_BYTE, nil)
dtl.gl_TexImage2D(GL.TEXTURE_2D, 0, GL.R32F, frameRes.w, frameRes.h, 0, GL.RED, GL.FLOAT, nil)
-- Nearest Filtering -- Might want to make linear for depth
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MAG_FILTER, GL.NEAREST)
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_MIN_FILTER, GL.NEAREST)



-- Attach buffers
dtl.gl_FramebufferTexture2D(GL.FRAMEBUFFER, GL.COLOR_ATTACHMENT0, GL.TEXTURE_2D, texRender.Color, 0)
dtl.gl_FramebufferTexture2D(GL.FRAMEBUFFER, GL.COLOR_ATTACHMENT1, GL.TEXTURE_2D, texRender.Normal, 0)
dtl.gl_FramebufferTexture2D(GL.FRAMEBUFFER, GL.COLOR_ATTACHMENT2, GL.TEXTURE_2D, texRender.Material, 0)
dtl.gl_FramebufferTexture2D(GL.FRAMEBUFFER, GL.COLOR_ATTACHMENT3, GL.TEXTURE_2D, texRender.Depth, 0)

--The depth/stencil buffer (for depth/stencil tests)
--
local rboDepthStencil = ffi.new("GLuint[1]")
dtl.gl_GenRenderbuffers(1, rboDepthStencil)
dtl.gl_BindRenderbuffer(GL.RENDERBUFFER, rboDepthStencil[0])
dtl.gl_RenderbufferStorage(GL.RENDERBUFFER, GL.DEPTH24_STENCIL8, frameRes.w, frameRes.h)
dtl.gl_FramebufferRenderbuffer(GL.FRAMEBUFFER, GL.DEPTH_STENCIL_ATTACHMENT, GL.RENDERBUFFER, rboDepthStencil[0])
--]]

-- Set the list of draw buffers.
local drawBuffers =  ffi.new("GLenum[4]")

drawBuffers[0] = GL.COLOR_ATTACHMENT0
drawBuffers[1] = GL.COLOR_ATTACHMENT1
drawBuffers[2] = GL.COLOR_ATTACHMENT2
drawBuffers[3] = GL.COLOR_ATTACHMENT3

dtl.gl_DrawBuffers(4, drawBuffers) -- N = number of buffers

-- Always check that our framebuffer is ok
if (dtl.gl_CheckFramebufferStatus(GL.FRAMEBUFFER) ~= GL.FRAMEBUFFER_COMPLETE) then
	error("Could not create framebuffer!".. string.format("%x",dtl.gl_CheckFramebufferStatus(GL.FRAMEBUFFER)))
end
print("Loading shaders")
local mainShader = shader("../shaders/main.vs", "../shaders/main.fs")
local quadShader = shader("../shaders/draw_quad.vs", "../shaders/draw_quad.fs")

local cam = {
	pos = vec3(),
	lookat = vec3(),
	up = vec3()
}
cam.pos[0] = { 1, 1, 3 }
cam.lookat[0] = { 0, 0, 5 }
cam.up[0] = { 0, 0, 1 }
local model_trans = mat4.translation(mat4.iden(mat4()), 0, 0, 0)
local model_scale = mat4.scaling(mat4.iden(mat4()), 1, 1, 1)
local model_rotate = mat4.rotationYPR(mat4.iden(mat4()), 0, 0, 0)
local model = mat4.iden(mat4())
mat4.mul(model, model_rotate, model)
mat4.mul(model, model_scale, model)
mat4.mul(model, model_trans, model)
local projection = mat4.perspectiveProjection(mat4(), 70, 16/9, 0.01, 100)
local view = mat4.lookAt(mat4(), cam.pos, cam.lookat, cam.up)
local mvp = mat4.iden(mat4())
local mvpinv = mat4()
local mvpinvT = mat3()
local mv = mat4.iden(mat4())
local vp = mat4()
local vpinv = mat4()
mat4.mul(mv, model, mv)
mat4.mul(mv, view, mv)
mat4.mul(mvp, projection, mv)
mat4.mul(vp, view, projection)
mat4.inverse(vpinv, vp)

render.clear()
render.setShader(mainShader)

mainShader:newUniform("MVPInv", shader.UNIFORM_MAT4F)
mainShader:newUniform("time", shader.UNIFORM_1F)

mainShader:newUniform("diffuse_t", shader.UNIFORM_1I)
mainShader:setUniform("diffuse_t", 0)

mainShader:newUniform("normal_t", shader.UNIFORM_1I)
mainShader:setUniform("normal_t", 1)

mainShader:newUniform("material_t", shader.UNIFORM_1I)
mainShader:setUniform("material_t", 2)

mainShader:newUniform("MVP", shader.UNIFORM_MAT4F)
mainShader:setUniform("MVP", mvp)

mainShader:newUniform("MVPInv", shader.UNIFORM_MAT4F)
mainShader:setUniform("MVPInv", mvp)

mainShader:newUniform("MV", shader.UNIFORM_MAT4F)
mainShader:setUniform("MV", mv)


render.setShader(quadShader)
quadShader:newUniform("MVP", shader.UNIFORM_MAT4F)
quadShader:setUniform("MVP", mvp)
quadShader:newUniform("MVPInv", shader.UNIFORM_MAT4F)
quadShader:setUniform("MVPInv", mvp)
--quadShader:newUniform("MVPInvT", shader.UNIFORM_MAT3F)
--quadShader:setUniform("MVPInvT", mvp)
--quadShader:newUniform("MV", shader.UNIFORM_MAT4F)
--quadShader:setUniform("MV", mv)
quadShader:newUniform("VPInv", shader.UNIFORM_MAT4F)
quadShader:setUniform("VPInv", vpinv)
--quadShader:newUniform("VP", shader.UNIFORM_MAT4F)
--quadShader:setUniform("VP", vp)
--quadShader:newUniform("V", shader.UNIFORM_MAT4F)
--quadShader:setUniform("V", view)
--quadShader:newUniform("P", shader.UNIFORM_MAT4F)
--quadShader:setUniform("P", projection)

quadShader:newUniform("camera_world_pos", shader.UNIFORM_3FV)
quadShader:setUniform("camera_world_pos", cam.pos)

quadShader:newUniform("max_specular_intensity", shader.UNIFORM_1F)
quadShader:setUniform("max_specular_intensity", 50)

quadShader:newUniform("time", shader.UNIFORM_1F)
quadShader:newUniform("renderColor", shader.UNIFORM_1I)
quadShader:newUniform("renderNormal", shader.UNIFORM_1I)
quadShader:newUniform("renderMaterial", shader.UNIFORM_1I)
quadShader:newUniform("renderDepth", shader.UNIFORM_1I)
quadShader:setUniform("renderColor", 0)
quadShader:setUniform("renderNormal", 1)
quadShader:setUniform("renderMaterial", 2)
quadShader:setUniform("renderDepth", 3)

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
local t = 0
while dtl.keystate.sp == 0 do
	local event = ffi.new("SDL_Event[1]")
	dtl.handle_events(event)
	t = t + 1/60;
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
	if dtl.keystate.lsh == 1 then
		vec3.scale(cam_motion, cam_motion, 0.2)
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
	mat3.assignMat4(mvpinvT, mvp)
	mat3.inverse(mvpinvT, mvpinvT)
	mat3.transpose(mvpinvT, mvpinvT)
	mat4.mul(vp, view, projection)
	mat4.inverse(vpinv, vp)
	mat4.mul(mv, model, view)
	
	brkpt()
	
	render.setShader(mainShader)
	mainShader:setUniform("MVP", mvp)
	mainShader:setUniform("MVPInv", mvpinv)

	dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, fbo[0])
	dtl.gl_Viewport(0,0,1280,720)
	render.clear()
	
	dtl.gl_ActiveTexture(GL.TEXTURE0)
	dtl.gl_BindTexture(GL.TEXTURE_2D, tex_diffuse)
	dtl.gl_ActiveTexture(GL.TEXTURE1)
	dtl.gl_BindTexture(GL.TEXTURE_2D, tex_normal)
	dtl.gl_ActiveTexture(GL.TEXTURE2)
	dtl.gl_BindTexture(GL.TEXTURE_2D, tex_material)
	
	dtl.gl_BindVertexArray(cube[0].vao)
	dtl.gl_DrawElements(GL.TRIANGLES,
		cube[0].n_indices,
		GL.UNSIGNED_INT,
		nil
	)
	
	dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, 0)
	render.clear()
	render.setShader(quadShader)
	quadShader:setUniform("camera_world_pos", cam.pos)
	quadShader:setUniform("MVP", mvp)
--	quadShader:setUniform("MV", mv)
--	quadShader:setUniform("VP", vp)
	quadShader:setUniform("VPInv", vpinv)
	quadShader:setUniform("MVPInv", mvpinv)
--	quadShader:setUniform("MVPInvT", mvpinvT)
	quadShader:setUniform("time", t)
--	quadShader:setUniform("V", view)
--	 quadShader:setUniform("P", projection)
	
	dtl.gl_ActiveTexture(GL.TEXTURE0)
	dtl.gl_BindTexture(GL.TEXTURE_2D, texRender.Color)
	
	dtl.gl_ActiveTexture(GL.TEXTURE1)
	dtl.gl_BindTexture(GL.TEXTURE_2D, texRender.Normal)
	
	dtl.gl_ActiveTexture(GL.TEXTURE2)
	dtl.gl_BindTexture(GL.TEXTURE_2D, texRender.Material)
	
	dtl.gl_ActiveTexture(GL.TEXTURE3)
	dtl.gl_BindTexture(GL.TEXTURE_2D, texRender.Depth)
	
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
