local M = {}

local ffi = require"ffi"
local dtl, sdl, km, soil = require("dtl"), require("sdl"), require("km"), require("soil")
local tex = require("texture")
local shader, render, GL = require("shader"), require("render"), require("gl")
local time = require("time")
local entity = require("entity")
local vec3, mat3, mat4, quat =  km.vec3, km.mat3, km.mat4, km.quat
local vr = require("vr")

print(vr.isHmdPresent())

local sleep = require("util.sleep")

local md

local function ultrastack()
	--local x =debug.getinfo(2)
	--for k,v in pairs(x) do print(k,v) end
	--print(x.short_src)
	print(debug.traceback(2))
end
--debug.sethook(ultrastack, "l")

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

local function debugmat2(m)
	local form = [[
%.2f	%.2f	%.2f	%.2f
%.2f	%.2f	%.2f	%.2f
%.2f	%.2f	%.2f	%.2f
%.2f	%.2f	%.2f	%.2f
]]
	return string.format(form,
		m.mat[0], m.mat[4], m.mat[8], m.mat[12],
		m.mat[1], m.mat[5], m.mat[9], m.mat[13],
		m.mat[2], m.mat[6], m.mat[10], m.mat[14],
		m.mat[3], m.mat[7], m.mat[11], m.mat[15]
	)
end

local function glerror(s)
	local e = dtl.gl_GetError()
	if e > 0 then
		error(string.format("%s OpenGL returned error: 0x%X",s, e), 2)
	end
end

local info = ffi.new("struct SDL_Info[1]")

dtl.initSDL(info)

if dtl.ogl_LoadFunctions() == dtl.ogl_LOAD_FAILED then
	error("Could not load OpenGL functions")
end
if soil.ogl_LoadFunctions() == dtl.ogl_LOAD_FAILED then
	error("Could not load SOIL's OpenGL functions")
end

local majorv, minorv = ffi.new("int[1]"), ffi.new("int[1]")
dtl.gl_GetIntegerv(GL.MAJOR_VERSION, majorv)
dtl.gl_GetIntegerv(GL.MINOR_VERSION, minorv)
print("GL VERSION: "..majorv[0].."."..minorv[0])
print("GLSL VERSION: "..ffi.string(dtl.gl_GetString(GL.SHADING_LANGUAGE_VERSION)))

shader.init()

print("Loading mesh...")
local quad = ffi.new("struct mesh[1]")
dtl.loadmesh("../assets/quad.obj", quad)

print("Loading texture...")
local tex_diffuse = tex.getTextureID("grey.png")
--local tex_diffuse = tex.getTextureID("brick1.png")
local tex_normal = tex.getTextureID("flat_normal.png")
--local tex_normal = tex.getTextureID("brick1n.png")
local tex_material = tex.getTextureID("grey.png")

print("Creating framebuffer")

local fbo = render.init()

print("Loading shaders")
local mainShader = shader("../shaders/main.vs", "../shaders/main.fs")
local quadShader = shader("../shaders/draw_quad.vs", "../shaders/draw_quad.fs")

local cam = {
	pos = vec3(),
	lookat = vec3(),
	up = vec3()
}
cam.pos[0] = { 3, 3, 0 }
cam.lookat[0] = { 1, 0, 0 }
cam.up[0] = { 0, 1, 0 }

local zNear = 0.1
local zFar = 50
local vFOV = math.rad(70)
local frameW, frameH = render.getFrameRes()
local frameAspect = frameW/frameH
local screenW, screenH = render.getScreenSize()

local hFOV = math.atan(frameAspect * math.tan(vFOV/2))

local model_trans = mat4.translation(mat4.iden(mat4()), 0, 0, 0)
local model_scale = mat4.scaling(mat4.iden(mat4()),1,1,1)
local model_rotate = mat4.rotationYPR(mat4.iden(mat4()), 0, 0, 0)
local model = mat4.iden(mat4())
mat4.mul(model, model_rotate, model)
mat4.mul(model, model_scale, model)
mat4.mul(model, model_trans, model)
local projection = mat4()
projection = mat4.perspectiveProjection(projection, math.deg(vFOV), frameAspect, zNear, zFar)
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

local cam_angle = quat.rotationPYR(quat(), 0, 0, 0)
local cam_forward = vec3()
local cam_right = vec3()
local cam_motion = vec3.zero(vec3())


render.clear()
render.setShader(mainShader)

mainShader:newUniform("time", shader.UNIFORM_1F)

mainShader:newUniform("vFOV", shader.UNIFORM_1F)
mainShader:newUniform("aspectRatio", shader.UNIFORM_1F)

mainShader:newUniform("diffuse_t", shader.UNIFORM_1I, 0)

mainShader:newUniform("normal_t", shader.UNIFORM_1I, 1)

mainShader:newUniform("material_t", shader.UNIFORM_1I, 2)

mainShader:newUniform("MVP", shader.UNIFORM_MAT4F)
mainShader:setUniform("MVP", mvp)

mainShader:newUniform("MV", shader.UNIFORM_MAT4F)
mainShader:setUniform("MV", mv)

render.setShader(quadShader)
quadShader:newUniform("MVP", shader.UNIFORM_MAT4F)
quadShader:setUniform("MVP", mvp)
quadShader:newUniform("V", shader.UNIFORM_MAT4F)
quadShader:setUniform("V", view)

quadShader:newUniform("max_specular_intensity", shader.UNIFORM_1F, 50)

quadShader:newUniform("vFOV", shader.UNIFORM_1F, vFOV)
quadShader:newUniform("aspectRatio", shader.UNIFORM_1F, frameAspect)

quadShader:newUniform("max_specular_intensity", shader.UNIFORM_1F, 50)

quadShader:newUniform("time", shader.UNIFORM_1F)
quadShader:newUniform("renderColor", shader.UNIFORM_1I, 0)
quadShader:newUniform("renderNormal", shader.UNIFORM_1I, 1)
quadShader:newUniform("renderMaterial", shader.UNIFORM_1I, 2)
quadShader:newUniform("renderDepth", shader.UNIFORM_1I, 3)

dtl.keystate.w = 0
dtl.keystate.a = 0
dtl.keystate.s = 0
dtl.keystate.d = 0
dtl.keystate.sp = 0
dtl.cam.yaw = 1.6
dtl.cam.pitch = math.pi/2
dtl.gl_Enable(GL.DEPTH_TEST)
dtl.gl_DepthFunc(GL.LESS)
dtl.gl_Enable(GL.CULL_FACE)
dtl.gl_CullFace(GL.BACK)
local t = 0

brkpt()

local frames_counter = 0
local frames_time = 0
local max_frame_time = 0
local min_frame_time = 99999
local last_fps_display = time.now()

if true then
	local s = 2
	for i=-s,s do
		for j=-s,s do
			local e = entity()
			e:setPos(i*5.1,math.random()*2-1, j*5.1)
			e:setModel("bolts.obj")
			e:setDiffuse("steel.png")
		end
	end
end
entity():setPos(25,10,5)

local event = ffi.new("SDL_Event[1]")

local interpupillaryDistance = 0.5
local eye_translate = vec3()
local eye_pos = vec3()

local begin_time = time.now()
local facesDrawn = 0


local function MainRenderToFBO()
	render.setShader(mainShader)

	dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, fbo[0])
	dtl.gl_Viewport(0,0,frameW,frameH)
	render.clear()
	
	dtl.gl_ActiveTexture(GL.TEXTURE1)
	dtl.gl_BindTexture(GL.TEXTURE_2D, tex_normal)
	dtl.gl_ActiveTexture(GL.TEXTURE2)
	dtl.gl_BindTexture(GL.TEXTURE_2D, tex_material)

	facesDrawn = 0
	local drawDistanceSqr = 500
	local lastModel = ""
	local lastTexture = ""
	for i, ent in ipairs(entity.getAll()) do
		facesDrawn = facesDrawn + ent.mesh[0].n_indices / 3

		mat4.mul(mvp, vp, ent:getMatrix())
		mat4.mul(mv, view, ent:getMatrix())
		mainShader:setUniform("MV", mv)
		mainShader:setUniform("MVP", mvp)


		if lastModel ~= ent.modelpath then
			dtl.gl_BindVertexArray(ent.mesh[0].vao)
			lastModel = ent.modelpath
		end
		
		if lastTexture ~= ent.tex_diffuse_path then
			dtl.gl_ActiveTexture(GL.TEXTURE0)
			dtl.gl_BindTexture(GL.TEXTURE_2D, ent.tex_diffuse)
			lastTexture = ent.tex_diffuse_path
		end
				
		dtl.gl_DrawElements(GL.TRIANGLES,
			ent.mesh[0].n_indices,
			GL.UNSIGNED_INT,
			nil
		)
	end
end

local function PrepDeferred()
	
	render.setShader(quadShader)
	quadShader:setUniform("MVP", mvp)
	quadShader:setUniform("V", view)

	quadShader:setUniform("time", t)
	dtl.gl_ActiveTexture(GL.TEXTURE0)
	dtl.gl_BindTexture(GL.TEXTURE_2D, render.GBuffer.Color)

	dtl.gl_ActiveTexture(GL.TEXTURE1)
	dtl.gl_BindTexture(GL.TEXTURE_2D, render.GBuffer.Normal)

	dtl.gl_ActiveTexture(GL.TEXTURE2)
	dtl.gl_BindTexture(GL.TEXTURE_2D, render.GBuffer.Material)

	dtl.gl_ActiveTexture(GL.TEXTURE3)
	dtl.gl_BindTexture(GL.TEXTURE_2D, render.GBuffer.Depth)

	dtl.gl_BindVertexArray(quad[0].vao)
	dtl.gl_BindBuffer(GL.ELEMENT_ARRAY_BUFFER, quad[0].element_buffer)
	
	dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, 0)
	
end

while dtl.keystate.sp == 0 do

	local start = time.now()
	--[[
	print("YAW: "..dtl.cam.yaw)
	print("PITCH:"..dtl.cam.pitch)
	print(hFOV, vFOV, math.deg(hFOV), math.deg(vFOV)) --]]
	--print(("POS: %.2f | %.2f | %.2f"):format(cam.pos[0].x, cam.pos[0].y, cam.pos[0].z))
	dtl.handle_events(event)
	t = t + 1/60;
	quat.rotationPYR(cam_angle, dtl.cam.pitch, dtl.cam.yaw, 0) -- Yaw and roll swapped for kazmath y-up
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
	
	vec3.scale(eye_translate, cam_right, interpupillaryDistance/2)
	
	
	vec3.sub(eye_pos, cam.pos, eye_translate)
	vec3.add(cam.lookat, cam.lookat, eye_pos)
	
	mat4.lookAt(mat4.iden(view), eye_pos, cam.lookat, cam.up)
	mat4.mul(vp, projection, view)

	-- LEFT EYE

	MainRenderToFBO()
	
	PrepDeferred()
	render.clear()
	dtl.gl_Viewport(0,0,frameW,frameH) --VR
	
	dtl.gl_DrawElements(GL.TRIANGLES,
		quad[0].n_indices,
		GL.UNSIGNED_INT,
		nil
	)	
	
	
	-- RIGHT EYE
	
	vec3.scale(cam.lookat, cam_forward, 10)
	
	vec3.add(eye_pos, cam.pos, eye_translate)
	vec3.add(cam.lookat, cam.lookat, eye_pos)
	
	mat4.lookAt(mat4.iden(view), eye_pos, cam.lookat, cam.up)
	mat4.mul(vp, projection, view)

	
	MainRenderToFBO()

	PrepDeferred()
	dtl.gl_Viewport(frameW,0,frameW,frameH) --VR
	
	dtl.gl_DrawElements(GL.TRIANGLES,
		quad[0].n_indices,
		GL.UNSIGNED_INT,
		nil
	)	

	sdl.SDL_GL_SwapWindow(info[0].window)

	local frame_end = time.now()
	local frametime = time.diff(frame_end, start)
	frames_counter = frames_counter + 1
	frames_time = frames_time + frametime
	if frametime > max_frame_time then max_frame_time = frametime end
	if frametime < min_frame_time then min_frame_time = frametime end
	if time.diff(frame_end, last_fps_display) > 1000.0 then
		print(
			string.format("FPS: %3.1f  AVG MS: %3.3f MIN: %3.3f MAX: %4.3f FACES %d",
			frames_counter*1000/frames_time,
			frames_time/frames_counter,
			min_frame_time, max_frame_time,
			facesDrawn)
		)
		frames_counter = 0
		min_frame_time = 99999
		max_frame_time = 0
		frames_time = 0
		last_fps_display = time.now()
	end


	--Clamp to ~60fps TODO: waste less time here? or remove (and use VSYNC)
	--sleep(math.min(16,(16-time.diff(time.now(), start)))/2000)
end

dtl.gl_DisableVertexAttribArray(0)
dtl.cleanupSDL(info[0])
