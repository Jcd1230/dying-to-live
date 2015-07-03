#include <stdlib.h>
#include "gl_core_3_3.h"
#include "shader.h"
#include <stdio.h>

GLuint load_shaders(const char* vertFile, const char* fragFile)
{
	printf("Created shaders\n");
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	
	printf("Created shaders\n");
	char* vertShaderSource;
	char* fragShaderSource;
	
	FILE* fp;
	long lSize;

	fp = fopen(vertFile, "rb");
	fseek(fp, 0, SEEK_END);
	lSize = ftell(fp);
	rewind(fp);
	
	
	vertShaderSource = calloc(1, lSize+1);
	if (fread(vertShaderSource, lSize, 1, fp) != 1) {
		printf("Failed to read vertex shader file\n");
		exit(1);
	}
	fclose(fp);

	fp = fopen(fragFile, "rb");
	fseek(fp, 0, SEEK_END);
	lSize = ftell(fp);
	rewind(fp);

	fragShaderSource = calloc(1, lSize+1);
	if (fread(fragShaderSource, lSize, 1, fp) != 1) {
		printf("Failed to read fragment shader file\n");
		exit(1);
	}
	fclose(fp);

	//Compile shaders
	glShaderSource(vertexShaderID, 1, (const GLchar * const*)&vertShaderSource, NULL);
	glCompileShader(vertexShaderID);

	glShaderSource(fragmentShaderID, 1, (const GLchar * const*)&fragShaderSource, NULL);
	glCompileShader(fragmentShaderID);

	//Check shaders
	GLint result = GL_FALSE;
	int infoLogLen;

	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLen);
	char *vertShaderLog = calloc(1, infoLogLen);
	glGetShaderInfoLog(vertexShaderID, infoLogLen, NULL, vertShaderLog);
	if (strlen(vertShaderLog)) {
		printf("ERROR: (vertex shader) %s: %s\n", vertFile, vertShaderLog);
		exit(1);
	}

	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLen);
	char *fragShaderLog = calloc(1, infoLogLen);
	glGetShaderInfoLog(fragmentShaderID, infoLogLen, NULL, fragShaderLog);
	if (strlen(fragShaderLog)) {
		printf("ERROR: (fragment shader) %s: %s\n", fragFile, fragShaderLog);
		exit(1);
	}
	// Link the program
	printf("Linking\n");
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	//Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLen);
	char *programErrorMessage = calloc(1, infoLogLen);
	glGetProgramInfoLog(programID, infoLogLen, NULL, programErrorMessage);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	free(vertShaderLog);
	free(fragShaderLog);
	free(programErrorMessage);

	return programID;
}
