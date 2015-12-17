#include "mesh.h"


int loadmeshes(const char* filename, struct mesh **meshes_in, int n_meshes_out)
{
	const struct aiScene* scene = aiImportFile(filename,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);
	
	int n_meshes = scene->mNumMeshes;
	
	if (n_meshes_out > 0) {
		n_meshes = n_meshes_out;
	} else {
		*meshes_in = malloc(sizeof(struct mesh) * n_meshes);
	}
	
	struct mesh* meshes = *meshes_in;
	int i;
	
	for (i = 0; i < n_meshes; i++) {
		meshes[i].n_verts = scene->mMeshes[i]->mNumVertices;
		meshes[i].v_pos = malloc(sizeof(GLfloat) * meshes[i].n_verts * 3);
		meshes[i].v_normal = malloc(sizeof(GLfloat) * meshes[i].n_verts * 3);
		meshes[i].v_tangent = malloc(sizeof(GLfloat) * meshes[i].n_verts * 3);
		meshes[i].v_uv = malloc(sizeof(GLfloat) * meshes[i].n_verts * 3);
		
		int n_faces = scene->mMeshes[i]->mNumFaces;
		meshes[i].n_indices = n_faces * 3;
		meshes[i].indices = malloc(sizeof(int) * meshes[i].n_indices);
		int j;
		//printf("Setting up %d indices\n", meshes[i].n_indices);
		for (j = 0; j < n_faces; j++) {
			//printf("Setting up indice %d\n", j*3);
			meshes[i].indices[j*3] = scene->mMeshes[i]->mFaces[j].mIndices[0];
			meshes[i].indices[j*3+1] = scene->mMeshes[i]->mFaces[j].mIndices[1];
			meshes[i].indices[j*3+2] = scene->mMeshes[i]->mFaces[j].mIndices[2];
		}
		//printf("Setting up pos/norm/tan/uv for %d verts\n", meshes[i].n_verts);
		for (j = 0; j < meshes[i].n_verts; j++) {
			//printf("Setting up #%d\n", j);
			struct aiVector3D pos, normal, uv, tangent;
			pos = scene->mMeshes[i]->mVertices[j];
			//printf("Setting up pos\n");
			meshes[i].v_pos[j*3+0] = pos.x;
			meshes[i].v_pos[j*3+1] = pos.y;
			meshes[i].v_pos[j*3+2] = pos.z;
			//printf("Setting up nor\n");
			if (scene->mMeshes[i]->mNormals != NULL) {
				normal = scene->mMeshes[i]->mNormals[j];

				meshes[i].v_normal[j*3+0] = normal.x;
				meshes[i].v_normal[j*3+1] = normal.y;
				meshes[i].v_normal[j*3+2] = normal.z;
				//printf("Setting up tan\n");
			} else {
				printf("Mesh is missing vertex normals (mesh.c)\n");
				exit(1);
				meshes[i].v_normal[j*3+0] = 0.0;
				meshes[i].v_normal[j*3+1] = 0.0;
				meshes[i].v_normal[j*3+2] = 0.0;
			}
			if (scene->mMeshes[i]->mTextureCoords != NULL) {
				uv = scene->mMeshes[i]->mTextureCoords[0][j];
				meshes[i].v_uv[j*3+0] = uv.x;
				meshes[i].v_uv[j*3+1] = uv.y;
				meshes[i].v_uv[j*3+2] = uv.z;
			} else {
				printf("Mesh is missing UV textures coordinates (mesh.c: %d)\n", __LINE__);
				exit(1);
				meshes[i].v_uv[j*3+0] = 0.0;
				meshes[i].v_uv[j*3+1] = 0.0;
				meshes[i].v_uv[j*3+2] = 0.0;
			}
			if (scene->mMeshes[i]->mTangents != NULL) {
				//printf("Adding tangent %d\n", j);
				tangent = scene->mMeshes[i]->mTangents[j];
				meshes[i].v_tangent[j*3+0] = tangent.x;
				meshes[i].v_tangent[j*3+1] = tangent.y;
				meshes[i].v_tangent[j*3+2] = tangent.z;
			} else {
				printf("Mesh is missing vertex tangents (mesh.c)\n");
				exit(1);
			}
		}

		glGenVertexArrays(1, &meshes[i].vao);
		glBindVertexArray(meshes[i].vao);

		glGenBuffers(1, &meshes[i].pos_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, meshes[i].pos_buffer);
		glBufferData(GL_ARRAY_BUFFER, 
				sizeof(GLfloat) * meshes[i].n_verts * 3,
				meshes[i].v_pos, GL_STATIC_DRAW);

		glGenBuffers(1, &meshes[i].normal_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, meshes[i].normal_buffer);
		glBufferData(GL_ARRAY_BUFFER, 
				sizeof(GLfloat) * meshes[i].n_verts * 3,
				meshes[i].v_normal, GL_STATIC_DRAW);
				
		glGenBuffers(1, &meshes[i].tangent_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, meshes[i].tangent_buffer);
		glBufferData(GL_ARRAY_BUFFER, 
				sizeof(GLfloat) * meshes[i].n_verts * 3,
				meshes[i].v_tangent, GL_STATIC_DRAW);
		
		glGenBuffers(1, &meshes[i].uv_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, meshes[i].uv_buffer);
		glBufferData(GL_ARRAY_BUFFER, 
				sizeof(GLfloat) * meshes[i].n_verts * 3,
				meshes[i].v_uv, GL_STATIC_DRAW);
		
		glBindBuffer(GL_ARRAY_BUFFER, meshes[i].pos_buffer);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void *)0
		);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, meshes[i].normal_buffer);
		glVertexAttribPointer(
			1,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void *)0
		);
		glEnableVertexAttribArray(1);
		
		glBindBuffer(GL_ARRAY_BUFFER, meshes[i].tangent_buffer);
		glVertexAttribPointer(
			2,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void *)0
		);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, meshes[i].uv_buffer);
		glVertexAttribPointer(
			3,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void *)0
		);
		glEnableVertexAttribArray(3);
		
		glGenBuffers(1, &meshes[i].element_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[i].element_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
				sizeof(unsigned int) * meshes[i].n_indices,
				meshes[i].indices, GL_STATIC_DRAW);
		

		//printf("Loaded mesh #%d, with %d vertices/%d indices/%d faces\n", i, j, 
		//		meshes[i].n_indices, scene->mMeshes[i]->mNumFaces);
	}
	return n_meshes;
}

int loadallmeshes(const char* filename, struct mesh **meshes_in)
{
	return loadmeshes(filename, meshes_in, -1);
}

int loadmesh(const char* filename, struct mesh *mesh_in)
{
	return loadmeshes(filename, &mesh_in, 1);
}