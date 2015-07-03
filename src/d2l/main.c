#include <stdio.h>
#include <stdlib.h>
#define GL3_PROTOTYPES 1
#include "gl_core_3_3.h"

#include "sdl_util.h"
#include <time.h>
#include "shader.h"
#include "mesh.h"
#include "../soil/SOIL.h"

#include "assimp.h"

#include "../kazmath/kazmath.h"

#include "globalstate.h"
#include "event.h"

#define PROGRAM_NAME "Tutorial1"


/* Our program's entry point */
int main(int argc, char *argv[])
{

	int run = 1;

	struct SDL_Info sdl_info;
	initSDL(&sdl_info);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1");
	


	printf("Loading shaders\n");
	// Load shaders
	GLuint programID = load_shaders("../shaders/main.vert",
			"../shaders/main.frag");

	kmVec3 cam_pos = { 3.0, 3.0, 3.0 };
	kmVec3 cam_lookat = { 0.0, 0.0, 0.0 };

	kmVec3 cam_up = { 0.0, 0.0, 1.0 };
	
	struct mesh meshes[1];
	int n_meshes = 1;
	printf("Loading mesh..\n");
	loadmesh("../test.obj", &meshes[0]);
	printf("Loaded mesh..\n");
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programID);

	kmMat4 iden;
	kmMat4Identity(&iden);

	kmMat4 model_translate = iden, model_scale = iden, model_rotate = iden;
	kmMat4 model = iden;
	kmMat4 view;
	kmMat4 projection = iden;
	
	kmMat4Translation(&model_translate, 0,0,0);
	kmMat4Scaling(&model_scale, 1.0,1.0,1.0);
	kmMat4RotationYawPitchRoll(&model_rotate, 0, 0, 0);

	kmMat4LookAt(&view, &cam_pos, &cam_lookat, &cam_up);
	kmMat4PerspectiveProjection(&projection, 70.0f, 16.0f/9.0f, 0.001, 100);

	kmMat4Multiply(&model, &model_rotate, &model);
	kmMat4Multiply(&model, &model_scale, &model);
	kmMat4Multiply(&model, &model_translate, &model);
	
	kmMat4 mvp = iden;

	kmMat4Multiply(&mvp, &model, &mvp);
	kmMat4Multiply(&mvp, &view, &mvp);
	kmMat4Multiply(&mvp, &projection, &mvp);
	

	GLuint tex_2d = SOIL_load_OGL_texture
		(
			"../distance.png",
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y );


	GLuint matID = glGetUniformLocation(programID, "MVP");
	GLuint matInvID = glGetUniformLocation(programID, "MVPInv");
	glUniformMatrix4fv(matID, 1, GL_FALSE, (const GLfloat *)&mvp.mat[0]);

	GLuint timeID = glGetUniformLocation(programID, "time");
	
	GLuint diffuse_loc = glGetUniformLocation(programID, "diffuse_color");
	glUniform1i(diffuse_loc, 0);
	
	GLuint sampler;
	glGenSamplers(1, &sampler);
/*	glSamplerParameteri(sampler,
	glSamplerParameteri(sampler, GL_TEXTURE_, 
	glSamplerParameteri(sampler, GL_TEXTURE_, 
	glSamplerParameteri(sampler, GL_TEXTURE_, 
	glSamplerParameteri(sampler, GL_TEXTURE_, 
*/
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_2d);
	glBindSampler(0, sampler);

	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	kmQuaternion cam_angle;
	kmQuaternionRotationPitchYawRoll(&cam_angle, 0,0,0);
	kmVec3 cam_forward;
	kmVec3 cam_right;
	kmVec3 cam_motion = {0, 0, 0};
	
	keystate.w = 0;
	keystate.a = 0;
	keystate.s = 0;
	keystate.d = 0;

	cam.yaw = 0.0f;
	cam.pitch = 0.0f;


	long long ticks_per_sec = SDL_GetPerformanceFrequency();
	long long ticks_last, ticks_cur;
	int frames_last = 0;
	ticks_last = SDL_GetPerformanceCounter();
	long long ticks_start = ticks_last;
	GLfloat curtime;
	while (run) {
		SDL_Event event;
		handle_events(&event);
		//Timing & fps
		ticks_cur = SDL_GetPerformanceCounter();
		curtime = (ticks_cur - ticks_start)/(float)ticks_per_sec;
		//printf("%lld\t%lld\t%.4f\n", ticks_cur, ticks_per_sec, curtime);
		
		frames_last++;
		if (ticks_cur > ticks_last + ticks_per_sec) {
			printf("FPS: %.2f\n", (float)frames_last*(float)(ticks_cur - ticks_last)/(float)ticks_per_sec);
			ticks_last = ticks_cur;
			frames_last = 0;
		}
		glUniform1f(timeID, curtime);
		
		kmQuaternionRotationPitchYawRoll(&cam_angle, cam.pitch, cam.yaw, 0.0f);
		kmQuaternionGetForwardVec3RH(&cam_forward, &cam_angle);
		kmQuaternionGetUpVec3(&cam_up, &cam_angle);
		kmQuaternionGetRightVec3(&cam_right, &cam_angle);
		kmVec3Zero(&cam_motion);
		if (keystate.w)
			kmVec3Add(&cam_motion, &cam_motion, &cam_forward);
		if (keystate.a)
			kmVec3Subtract(&cam_motion, &cam_motion, &cam_right);
		if (keystate.s)
			kmVec3Subtract(&cam_motion, &cam_motion, &cam_forward);
		if (keystate.d)
			kmVec3Add(&cam_motion, &cam_motion, &cam_right);
		kmVec3Scale(&cam_motion, &cam_motion, 0.1);
		kmVec3Add(&cam_pos, &cam_pos, &cam_motion);
		kmQuaternionRotationPitchYawRoll(&cam_angle, cam.pitch, cam.yaw, 0.0f);
		kmQuaternionGetForwardVec3RH(&cam_forward, &cam_angle);
		kmQuaternionGetUpVec3(&cam_up, &cam_angle);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		kmVec3Scale(&cam_lookat, &cam_forward, 10);
		kmVec3Add(&cam_lookat, &cam_lookat, &cam_pos);
		view = iden;
		kmMat4LookAt(&view, &cam_pos, &cam_lookat, &cam_up);
		mvp = iden;
		kmMat4Multiply(&mvp, &model, &mvp);
		kmMat4Multiply(&mvp, &view, &mvp);
		kmMat4Multiply(&mvp, &projection, &mvp);
		
		kmMat4 mvpinv = mvp;
		kmMat4Inverse(&mvpinv, &mvp);
		
		glUniformMatrix4fv(matInvID, 1, GL_FALSE, (const GLfloat *)&mvpinv.mat[0]);
		glUniformMatrix4fv(matID, 1, GL_FALSE, (const GLfloat *)&mvp.mat[0]);
		int i;
		struct mesh *curmesh;
		for (i = 0; i < n_meshes; i++) {
			curmesh = &meshes[i];
			glBindVertexArray(curmesh->vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curmesh->element_buffer);
			glDrawElements(GL_TRIANGLES, 
					curmesh->n_indices, 
					GL_UNSIGNED_INT, 
					(void*)0);

		}

    /* Swap our back buffer to the front */
		SDL_GL_SwapWindow(sdl_info.window);
	}
	glDisableVertexAttribArray(0);
    /* Delete our opengl context, destroy our window, and shutdown SDL */

	cleanupSDL(&sdl_info);
    return 0;
}
