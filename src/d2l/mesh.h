#ifndef __MESH_H__
#define __MESH_H__

#include <stdlib.h>
#include <stdio.h>
#include "gl_core_3_3.h"
#include "../kazmath/kazmath.h"
#include "assimp.h"

struct mesh {
	int n_verts;
	int n_indices;
	unsigned int *indices;

	GLfloat *v_pos;
	GLfloat *v_normal;
	GLfloat *v_tangent;
	GLfloat *v_uv;
	GLuint vao;
	GLuint pos_buffer;
	GLuint normal_buffer;
	GLuint tangent_buffer;
	GLuint uv_buffer;
	GLuint element_buffer;
};

struct mesh_library {
	int size;
	int max_size;
	struct mesh* meshes;
};

int init_mesh_library(struct mesh_library *lib, int initial_size);
int loadmeshes(const char* filename, struct mesh **meshes_in, int n_meshes_out);
int loadallmeshes(const char* filename, struct mesh **meshes_in);
int loadmesh(const char* filename, struct mesh *mesh_in);

#endif
