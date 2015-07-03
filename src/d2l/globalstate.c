
struct {
		int w,a,s,d, sp, lsh;
} keystate;

struct {
	float yaw, pitch;
	float x,y,z;
} cam;

extern struct mesh_library meshes_global;

const float yaw_sensitivity = 0.005;
const float pitch_sensitivity = 0.005;
int window_locked = 0;