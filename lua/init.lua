local M = {}

local ffi = require"ffi"
local dtl, sdl, km, soil = require("dtl"), require("sdl"), require("km"), require("soil")
local tex = require("texture")
local shader, render, GL = require("shader"), require("render"), require("gl")
local time = require("time")
local entity = require("entity")
local vec3, mat3, mat4, quat =  km.vec3, km.mat3, km.mat4, km.quat

local sleep = require("util.sleep")

local md

local function ultrastack()
	--local x =debug.getinfo(2)
	--for k,v in pairs(x) do print(k,v) end
	--print(x.short_src)
	print(debug.traceback(2))
end
--debug.sethook(ultrastack, "l")

DEBUG = true

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
--dtl.initOGL()
if dtl.ogl_LoadFunctions() == dtl.ogl_LOAD_FAILED then
	error("Could not load OpenGL functions")
end
if soil.ogl_LoadFunctions() == dtl.ogl_LOAD_FAILED then
	error("Could not load SOIL's OpenGL functions")
end

print(dtl.gl_GetIntegerv)


local majorv, minorv = ffi.new("int[1]"), ffi.new("int[1]")
dtl.gl_GetIntegerv(GL.MAJOR_VERSION, majorv)
dtl.gl_GetIntegerv(GL.MINOR_VERSION, minorv)
print("GL VERSION: "..majorv[0].."."..minorv[0])
print("GLSL VERSION: "..ffi.string(dtl.gl_GetString(GL.SHADING_LANGUAGE_VERSION)))

shader.init()
print("Loading mesh...")
local cube = ffi.new("struct mesh[1]")
local quad = ffi.new("struct mesh[1]")
dtl.loadmesh("../assets/plane.x", cube)
dtl.loadmesh("../assets/quad.obj", quad)
print("Loading texture...")
local tex_diffuse = tex.getTextureID("grey.png")
--local tex_diffuse = tex.getTextureID("brick1.png")
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_WRAP_S, GL.REPEAT)
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_WRAP_T, GL.REPEAT)
local tex_normal = tex.getTextureID("flat_normal.png")
--local tex_normal = tex.getTextureID("brick1n.png")
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_WRAP_S, GL.REPEAT)
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_WRAP_T, GL.REPEAT)
local tex_material = tex.getTextureID("grey.png")
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_WRAP_S, GL.REPEAT)
dtl.gl_TexParameteri(GL.TEXTURE_2D, GL.TEXTURE_WRAP_T, GL.REPEAT)

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
cam.lookat[0] = { 0, 0, 0 }
cam.up[0] = { 0, 1, 0 }

local zNear = 0.1
local zFar = 50
local vFOV = math.rad(35)
local aspectRatio = 16/9
local hFOV = math.atan(aspectRatio * math.tan(vFOV/2))

local model_trans = mat4.translation(mat4.iden(mat4()), 0, 0, 0)
local model_scale = mat4.scaling(mat4.iden(mat4()),1,1,1)
local model_rotate = mat4.rotationYPR(mat4.iden(mat4()), 0, 0, 0)
local model = mat4.iden(mat4())
mat4.mul(model, model_rotate, model)
mat4.mul(model, model_scale, model)
mat4.mul(model, model_trans, model)
local projection = mat4()
projection = mat4.perspectiveProjection(projection, math.deg(vFOV)*2, aspectRatio, zNear, zFar)
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

mainShader:newUniform("MVPInv", shader.UNIFORM_MAT4F)
mainShader:newUniform("time", shader.UNIFORM_1F)

mainShader:newUniform("zNear", shader.UNIFORM_1F, zNear)
mainShader:newUniform("zFar", shader.UNIFORM_1F, zFar)
mainShader:newUniform("vFOV", shader.UNIFORM_1F)
mainShader:newUniform("aspectRatio", shader.UNIFORM_1F)

mainShader:newUniform("diffuse_t", shader.UNIFORM_1I, 0)

mainShader:newUniform("normal_t", shader.UNIFORM_1I, 1)

mainShader:newUniform("material_t", shader.UNIFORM_1I, 2)

mainShader:newUniform("MVP", shader.UNIFORM_MAT4F)
mainShader:setUniform("MVP", mvp)

mainShader:newUniform("MVPInv", shader.UNIFORM_MAT4F)
mainShader:setUniform("MVPInv", mvp)

mainShader:newUniform("MV", shader.UNIFORM_MAT4F)
mainShader:setUniform("MV", mv)

mainShader:newUniform("M", shader.UNIFORM_MAT4F)
mainShader:setUniform("M", model)



render.setShader(quadShader)
quadShader:newUniform("MVP", shader.UNIFORM_MAT4F)
quadShader:setUniform("MVP", mvp)
quadShader:newUniform("MVPInv", shader.UNIFORM_MAT4F)
quadShader:setUniform("MVPInv", mvp)

quadShader:newUniform("VPInv", shader.UNIFORM_MAT4F)
quadShader:setUniform("VPInv", vpinv)


quadShader:newUniform("camera_world_pos", shader.UNIFORM_3FV)
quadShader:setUniform("camera_world_pos", cam.pos)
quadShader:newUniform("camera_forward", shader.UNIFORM_3FV)
quadShader:setUniform("camera_forward", cam_forward)

quadShader:newUniform("max_specular_intensity", shader.UNIFORM_1F, 50)

quadShader:newUniform("vFOV", shader.UNIFORM_1F, vFOV)
quadShader:newUniform("hFOV", shader.UNIFORM_1F, hFOV)

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
dtl.cam.pitch = 0
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

if false then
	local s = 7
	for i=-s,s do
		for j=-s,s do
			local e = entity()
			e:setPos(i*2.1,math.random()*2-1,j*2.1)
		end
	end
end
local e = entity()
e:setPos(0,0,0)

local event = ffi.new("SDL_Event[1]")

local begin_time = time.now()

local serial, err = io.open("/dev/ttyACM0", "r")
if not serial then print("Could not connect to serial device: "..err) end
serial:write("a\n")
local initialrot, lastrot

while dtl.keystate.sp == 0 do

	local start = time.now()
	--[[
	print("YAW: "..dtl.cam.yaw)
	print("PITCH:"..dtl.cam.pitch)
	print(("POS: %.2f | %.2f | %.2f"):format(cam.pos[0].x, cam.pos[0].y, cam.pos[0].z))
	print(hFOV, vFOV, math.deg(hFOV), math.deg(vFOV))--]]
	dtl.handle_events(event)
	t = t + 1/60;
	quat.rotationPYR(cam_angle, dtl.cam.pitch, dtl.cam.yaw, 0)
	vec3.quatForwardVec3RH(cam_forward, cam_angle)
	vec3.quatUpVec3(cam.up, cam_angle)
	vec3.quatRightVec3(cam_right, cam_angle)
	vec3.zero(cam_motion)
	
	
	if serial then
		-- Read until you cant
		local l
		local nl = serial:read()
		while nl do
			l = nl
			nl = serial:read()
		end
		if l then
			local y,p,r = l:match("ypr%s+(%--%d*%.%d%d)%s+(%--%d*%.%d%d)%s+(%--%d*%.%d%d)")
			
			y,p,r = tonumber(y), tonumber(p), tonumber(r)
			if y and p and r then
				print(l)
				if not initialrot then
					initialrot = {p, y, r}
				end
				e:setRotationPYR(
					-math.rad(p - initialrot[1]), 
					-math.rad(y - initialrot[2]), 
					math.rad(r - initialrot[3])
					)
				print("GOT: ",y,p,r)
			end
		end
	end
		
	--e:setRotationPYR(0, 0, (t) % 360)
	
	
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
	mat4.mul(vp, projection, view )

	render.setShader(mainShader)
	mainShader:setUniform("MV", mv)

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
	local facesDrawn = 0
	local drawDistanceSqr = 500
	for i, ent in ipairs(entity.getAll()) do
		facesDrawn = facesDrawn + cube[0].n_indices / 3
		
		mat4.mul(mvp, vp, ent:getMatrix())
		mainShader:setUniform("M", ent:getMatrix())
		mainShader:setUniform("MVP", mvp)
		
		dtl.gl_DrawElements(GL.TRIANGLES,
			cube[0].n_indices,
			GL.UNSIGNED_INT,
			nil
		)
	end

	dtl.gl_BindFramebuffer(GL.FRAMEBUFFER, 0)
	render.clear()
	render.setShader(quadShader)
	quadShader:setUniform("camera_world_pos", cam.pos)
	quadShader:setUniform("camera_forward", cam_forward)
	quadShader:setUniform("MVP", mvp)

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
	--dtl.gl_BindBuffer(GL.ELEMENT_ARRAY_BUFFER, quad[0].element_buffer)	
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
--ffi.C.free(cam.pos)
