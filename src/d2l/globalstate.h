#ifndef __GLOBALSTATE_H__
#define __GLOBALSTATE_H__


extern struct {
		int w,a,s,d, sp, lsh;
} keystate;

extern struct {
	float yaw, pitch;
	float x,y,z;
} cam;

extern struct mesh_library meshes_global;

extern const float yaw_sensitivity;
extern const float pitch_sensitivity;
extern int window_locked;

#endif