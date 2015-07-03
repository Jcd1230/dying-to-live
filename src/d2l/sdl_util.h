#ifndef __SDL_UTIL_H__
#define __SDL_UTIL_H__
#include <SDL2/SDL.h>

#define PI 3.14159265

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

struct SDL_Info {
	SDL_Window *window;
	SDL_GLContext *context;
};

void sdldie(const char *msg);
void checkSDLError(int line);
void initSDL(struct SDL_Info *info);
void cleanupSDL(struct SDL_Info *info);

#endif


/*
--print(ffi.string(ffi.C.getcwd(nil)))
--[[
local SDL_INIT_VIDEO = 0x20
dtl.testjcd(5)
local SDL_GL_CONTEXT_MAJOR_VERSION, SDL_GL_CONTENT_MINOR_VERSION = 0x11, 0x12
local SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_CENTERED = 0x1fff0000, 0x2fff0000
dtl.SDL_GL_SetAttribute(dtl.SDL_GL_CONTEXT_MAJOR_VERSION, 3)
dtl.SDL_GL_SetAttribute(dtl.SDL_GL_CONTEXT_MINOR_VERSION, 3)

if dtl.SDL_Init(SDL_INIT_VIDEO) < 0 then
	error("Couldn't initialize SDL")
end
local window = {
	width = 1280,
	height = 720
}
dtl.SDL_GL_SetAttribute(dtl.SDL_GL_DOUBLEBUFFER, 1)
dtl.SDL_GL_SetAttribute(dtl.SDL_GL_DEPTH_SIZE, 24)
M.window = dtl.SDL_CreateWindow(
	"D2L",
	SDL_WINDOWPOS_CENTERED,
	SDL_WINDOWPOS_CENTERED,
	window.width, window.height,
	bit.bor(dtl.SDL_WINDOW_OPENGL,dtl.SDL_WINDOW_SHOWN)
)
if not window then
	error("Couldn't initialize window")
end]]
*/