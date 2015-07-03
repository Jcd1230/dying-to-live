#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "gl_core_3_3.h"

struct texture {
	GLuint texID;
}

struct object {
	struct mesh* mesh;
	int n_textures;
	struct texture* textures;
	
};


#endif