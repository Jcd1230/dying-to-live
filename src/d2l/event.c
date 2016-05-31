#include "event.h"

void handle_events(SDL_Event* event)
{
	while (SDL_PollEvent(event)) {
		int isdown = (event->type==SDL_KEYDOWN);
		switch (event->type)
		{
		case SDL_QUIT:
			exit(0);
			break;
		case SDL_MOUSEMOTION:
			if (!window_locked) break;
			cam.yaw = fmodf((cam.yaw - (float)event->motion.xrel*yaw_sensitivity),2.0f*PI);
			cam.pitch = fminf(
					fmaxf(
						(cam.pitch - (float)event->motion.yrel*pitch_sensitivity),
						0.01 //0.01 safe zone to prevent camera flipping
					),
					PI - 0.01
				);
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (event->key.repeat == 1) {
				break;
			}
			if (event->key.keysym.sym == SDLK_w) {
				keystate.w = isdown;
			}
			if (event->key.keysym.sym == SDLK_a) {
				keystate.a = isdown;
			}
			if (event->key.keysym.sym == SDLK_s) {
				keystate.s = isdown;
			}
			if (event->key.keysym.sym == SDLK_d) {
				keystate.d = isdown;
			}
			if (event->key.keysym.sym == SDLK_SPACE) {
				keystate.sp = isdown;
			}
			if (event->key.keysym.sym == SDLK_LCTRL && isdown) {
				window_locked = !window_locked;
				//SDL_ShowCursor(!window_locked);
				SDL_SetRelativeMouseMode(window_locked);
			}
			if (event->key.keysym.sym == SDLK_LSHIFT) {
				keystate.lsh = isdown;
			}
			break;
		}
	}
}