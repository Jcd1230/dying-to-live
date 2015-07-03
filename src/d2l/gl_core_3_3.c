#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "gl_core_3_3.h"

#if defined(__APPLE__)
#include <mach-o/dyld.h>

static void* AppleGLGetProcAddress (const GLubyte *name)
{
  static const struct mach_header* image = NULL;
  NSSymbol symbol;
  char* symbolName;
  if (NULL == image)
  {
    image = NSAddImage("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", NSADDIMAGE_OPTION_RETURN_ON_ERROR);
  }
  /* prepend a '_' for the Unix C symbol mangling convention */
  symbolName = malloc(strlen((const char*)name) + 2);
  strcpy(symbolName+1, (const char*)name);
  symbolName[0] = '_';
  symbol = NULL;
  /* if (NSIsSymbolNameDefined(symbolName))
	 symbol = NSLookupAndBindSymbol(symbolName); */
  symbol = image ? NSLookupSymbolInImage(image, symbolName, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND | NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR) : NULL;
  free(symbolName);
  return symbol ? NSAddressOfSymbol(symbol) : NULL;
}
#endif /* __APPLE__ */

#if defined(__sgi) || defined (__sun)
#include <dlfcn.h>
#include <stdio.h>

static void* SunGetProcAddress (const GLubyte* name)
{
  static void* h = NULL;
  static void* gpa;

  if (h == NULL)
  {
    if ((h = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL)) == NULL) return NULL;
    gpa = dlsym(h, "glXGetProcAddress");
  }

  if (gpa != NULL)
    return ((void*(*)(const GLubyte*))gpa)(name);
  else
    return dlsym(h, (const char*)name);
}
#endif /* __sgi || __sun */

#if defined(_WIN32)

#ifdef _MSC_VER
#pragma warning(disable: 4055)
#pragma warning(disable: 4054)
#endif

static int TestPointer(const PROC pTest)
{
	ptrdiff_t iTest;
	if(!pTest) return 0;
	iTest = (ptrdiff_t)pTest;
	
	if(iTest == 1 || iTest == 2 || iTest == 3 || iTest == -1) return 0;
	
	return 1;
}

static PROC WinGetProcAddress(const char *name)
{
	HMODULE glMod = NULL;
	PROC pFunc = wglGetProcAddress((LPCSTR)name);
	if(TestPointer(pFunc))
	{
		return pFunc;
	}
	glMod = GetModuleHandleA("OpenGL32.dll");
	return (PROC)GetProcAddress(glMod, (LPCSTR)name);
}
	
#define IntGetProcAddress(name) WinGetProcAddress(name)
#else
	#if defined(__APPLE__)
		#define IntGetProcAddress(name) AppleGLGetProcAddress(name)
	#else
		#if defined(__sgi) || defined(__sun)
			#define IntGetProcAddress(name) SunGetProcAddress(name)
		#else /* GLX */
		    #include <GL/glx.h>

			#define IntGetProcAddress(name) (*glXGetProcAddressARB)((const GLubyte*)name)
		#endif
	#endif
#endif

int ogl_ext_EXT_texture_compression_s3tc = ogl_LOAD_FAILED;
int ogl_ext_EXT_texture_sRGB = ogl_LOAD_FAILED;
int ogl_ext_EXT_texture_filter_anisotropic = ogl_LOAD_FAILED;
int ogl_ext_ARB_compressed_texture_pixel_storage = ogl_LOAD_FAILED;
int ogl_ext_ARB_conservative_depth = ogl_LOAD_FAILED;
int ogl_ext_ARB_ES2_compatibility = ogl_LOAD_FAILED;
int ogl_ext_ARB_get_program_binary = ogl_LOAD_FAILED;
int ogl_ext_ARB_explicit_uniform_location = ogl_LOAD_FAILED;
int ogl_ext_ARB_internalformat_query = ogl_LOAD_FAILED;
int ogl_ext_ARB_internalformat_query2 = ogl_LOAD_FAILED;
int ogl_ext_ARB_map_buffer_alignment = ogl_LOAD_FAILED;
int ogl_ext_ARB_program_interface_query = ogl_LOAD_FAILED;
int ogl_ext_ARB_separate_shader_objects = ogl_LOAD_FAILED;
int ogl_ext_ARB_shading_language_420pack = ogl_LOAD_FAILED;
int ogl_ext_ARB_shading_language_packing = ogl_LOAD_FAILED;
int ogl_ext_ARB_texture_buffer_range = ogl_LOAD_FAILED;
int ogl_ext_ARB_texture_storage = ogl_LOAD_FAILED;
int ogl_ext_ARB_texture_view = ogl_LOAD_FAILED;
int ogl_ext_ARB_vertex_attrib_binding = ogl_LOAD_FAILED;
int ogl_ext_ARB_viewport_array = ogl_LOAD_FAILED;
int ogl_ext_ARB_arrays_of_arrays = ogl_LOAD_FAILED;
int ogl_ext_ARB_clear_buffer_object = ogl_LOAD_FAILED;
int ogl_ext_ARB_copy_image = ogl_LOAD_FAILED;
int ogl_ext_ARB_ES3_compatibility = ogl_LOAD_FAILED;
int ogl_ext_ARB_fragment_layer_viewport = ogl_LOAD_FAILED;
int ogl_ext_ARB_framebuffer_no_attachments = ogl_LOAD_FAILED;
int ogl_ext_ARB_invalidate_subdata = ogl_LOAD_FAILED;
int ogl_ext_ARB_robust_buffer_access_behavior = ogl_LOAD_FAILED;
int ogl_ext_ARB_stencil_texturing = ogl_LOAD_FAILED;
int ogl_ext_ARB_texture_query_levels = ogl_LOAD_FAILED;
int ogl_ext_ARB_texture_storage_multisample = ogl_LOAD_FAILED;
int ogl_ext_KHR_debug = ogl_LOAD_FAILED;

void (CODEGEN_FUNCPTR *gl_ClearDepthf)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_DepthRangef)(GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_GetShaderPrecisionFormat)(GLenum, GLenum, GLint *, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ReleaseShaderCompiler)() = NULL;
void (CODEGEN_FUNCPTR *gl_ShaderBinary)(GLsizei, const GLuint *, GLenum, const void *, GLsizei) = NULL;

static int Load_ARB_ES2_compatibility()
{
	int numFailed = 0;
	gl_ClearDepthf = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glClearDepthf");
	if(!gl_ClearDepthf) numFailed++;
	gl_DepthRangef = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat))IntGetProcAddress("glDepthRangef");
	if(!gl_DepthRangef) numFailed++;
	gl_GetShaderPrecisionFormat = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *, GLint *))IntGetProcAddress("glGetShaderPrecisionFormat");
	if(!gl_GetShaderPrecisionFormat) numFailed++;
	gl_ReleaseShaderCompiler = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glReleaseShaderCompiler");
	if(!gl_ReleaseShaderCompiler) numFailed++;
	gl_ShaderBinary = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *, GLenum, const void *, GLsizei))IntGetProcAddress("glShaderBinary");
	if(!gl_ShaderBinary) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_GetProgramBinary)(GLuint, GLsizei, GLsizei *, GLenum *, void *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramBinary)(GLuint, GLenum, const void *, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramParameteri)(GLuint, GLenum, GLint) = NULL;

static int Load_ARB_get_program_binary()
{
	int numFailed = 0;
	gl_GetProgramBinary = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLenum *, void *))IntGetProcAddress("glGetProgramBinary");
	if(!gl_GetProgramBinary) numFailed++;
	gl_ProgramBinary = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const void *, GLsizei))IntGetProcAddress("glProgramBinary");
	if(!gl_ProgramBinary) numFailed++;
	gl_ProgramParameteri = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint))IntGetProcAddress("glProgramParameteri");
	if(!gl_ProgramParameteri) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_GetInternalformativ)(GLenum, GLenum, GLenum, GLsizei, GLint *) = NULL;

static int Load_ARB_internalformat_query()
{
	int numFailed = 0;
	gl_GetInternalformativ = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLsizei, GLint *))IntGetProcAddress("glGetInternalformativ");
	if(!gl_GetInternalformativ) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_GetInternalformati64v)(GLenum, GLenum, GLenum, GLsizei, GLint64 *) = NULL;

static int Load_ARB_internalformat_query2()
{
	int numFailed = 0;
	gl_GetInternalformati64v = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLsizei, GLint64 *))IntGetProcAddress("glGetInternalformati64v");
	if(!gl_GetInternalformati64v) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_GetProgramInterfaceiv)(GLuint, GLenum, GLenum, GLint *) = NULL;
GLuint (CODEGEN_FUNCPTR *gl_GetProgramResourceIndex)(GLuint, GLenum, const GLchar *) = NULL;
GLint (CODEGEN_FUNCPTR *gl_GetProgramResourceLocation)(GLuint, GLenum, const GLchar *) = NULL;
GLint (CODEGEN_FUNCPTR *gl_GetProgramResourceLocationIndex)(GLuint, GLenum, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetProgramResourceName)(GLuint, GLenum, GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetProgramResourceiv)(GLuint, GLenum, GLuint, GLsizei, const GLenum *, GLsizei, GLsizei *, GLint *) = NULL;

static int Load_ARB_program_interface_query()
{
	int numFailed = 0;
	gl_GetProgramInterfaceiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLenum, GLint *))IntGetProcAddress("glGetProgramInterfaceiv");
	if(!gl_GetProgramInterfaceiv) numFailed++;
	gl_GetProgramResourceIndex = (GLuint (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLchar *))IntGetProcAddress("glGetProgramResourceIndex");
	if(!gl_GetProgramResourceIndex) numFailed++;
	gl_GetProgramResourceLocation = (GLint (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLchar *))IntGetProcAddress("glGetProgramResourceLocation");
	if(!gl_GetProgramResourceLocation) numFailed++;
	gl_GetProgramResourceLocationIndex = (GLint (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLchar *))IntGetProcAddress("glGetProgramResourceLocationIndex");
	if(!gl_GetProgramResourceLocationIndex) numFailed++;
	gl_GetProgramResourceName = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetProgramResourceName");
	if(!gl_GetProgramResourceName) numFailed++;
	gl_GetProgramResourceiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint, GLsizei, const GLenum *, GLsizei, GLsizei *, GLint *))IntGetProcAddress("glGetProgramResourceiv");
	if(!gl_GetProgramResourceiv) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_ActiveShaderProgram)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_BindProgramPipeline)(GLuint) = NULL;
GLuint (CODEGEN_FUNCPTR *gl_CreateShaderProgramv)(GLenum, GLsizei, const GLchar *const*) = NULL;
void (CODEGEN_FUNCPTR *gl_DeleteProgramPipelines)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GenProgramPipelines)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetProgramPipelineInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetProgramPipelineiv)(GLuint, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsProgramPipeline)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform1d)(GLuint, GLint, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform1dv)(GLuint, GLint, GLsizei, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform1f)(GLuint, GLint, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform1fv)(GLuint, GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform1i)(GLuint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform1iv)(GLuint, GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform1ui)(GLuint, GLint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform1uiv)(GLuint, GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform2d)(GLuint, GLint, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform2dv)(GLuint, GLint, GLsizei, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform2f)(GLuint, GLint, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform2fv)(GLuint, GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform2i)(GLuint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform2iv)(GLuint, GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform2ui)(GLuint, GLint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform2uiv)(GLuint, GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform3d)(GLuint, GLint, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform3dv)(GLuint, GLint, GLsizei, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform3f)(GLuint, GLint, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform3fv)(GLuint, GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform3i)(GLuint, GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform3iv)(GLuint, GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform3ui)(GLuint, GLint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform3uiv)(GLuint, GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform4d)(GLuint, GLint, GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform4dv)(GLuint, GLint, GLsizei, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform4f)(GLuint, GLint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform4fv)(GLuint, GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform4i)(GLuint, GLint, GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform4iv)(GLuint, GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform4ui)(GLuint, GLint, GLuint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniform4uiv)(GLuint, GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix2dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix2fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix2x3dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix2x3fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix2x4dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix2x4fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix3dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix3fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix3x2dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix3x2fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix3x4dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix3x4fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix4dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix4fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix4x2dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix4x2fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix4x3dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProgramUniformMatrix4x3fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_UseProgramStages)(GLuint, GLbitfield, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_ValidateProgramPipeline)(GLuint) = NULL;

static int Load_ARB_separate_shader_objects()
{
	int numFailed = 0;
	gl_ActiveShaderProgram = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glActiveShaderProgram");
	if(!gl_ActiveShaderProgram) numFailed++;
	gl_BindProgramPipeline = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glBindProgramPipeline");
	if(!gl_BindProgramPipeline) numFailed++;
	gl_CreateShaderProgramv = (GLuint (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLchar *const*))IntGetProcAddress("glCreateShaderProgramv");
	if(!gl_CreateShaderProgramv) numFailed++;
	gl_DeleteProgramPipelines = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteProgramPipelines");
	if(!gl_DeleteProgramPipelines) numFailed++;
	gl_GenProgramPipelines = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenProgramPipelines");
	if(!gl_GenProgramPipelines) numFailed++;
	gl_GetProgramPipelineInfoLog = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetProgramPipelineInfoLog");
	if(!gl_GetProgramPipelineInfoLog) numFailed++;
	gl_GetProgramPipelineiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetProgramPipelineiv");
	if(!gl_GetProgramPipelineiv) numFailed++;
	gl_IsProgramPipeline = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsProgramPipeline");
	if(!gl_IsProgramPipeline) numFailed++;
	gl_ProgramUniform1d = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLdouble))IntGetProcAddress("glProgramUniform1d");
	if(!gl_ProgramUniform1d) numFailed++;
	gl_ProgramUniform1dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLdouble *))IntGetProcAddress("glProgramUniform1dv");
	if(!gl_ProgramUniform1dv) numFailed++;
	gl_ProgramUniform1f = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLfloat))IntGetProcAddress("glProgramUniform1f");
	if(!gl_ProgramUniform1f) numFailed++;
	gl_ProgramUniform1fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLfloat *))IntGetProcAddress("glProgramUniform1fv");
	if(!gl_ProgramUniform1fv) numFailed++;
	gl_ProgramUniform1i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint))IntGetProcAddress("glProgramUniform1i");
	if(!gl_ProgramUniform1i) numFailed++;
	gl_ProgramUniform1iv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLint *))IntGetProcAddress("glProgramUniform1iv");
	if(!gl_ProgramUniform1iv) numFailed++;
	gl_ProgramUniform1ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLuint))IntGetProcAddress("glProgramUniform1ui");
	if(!gl_ProgramUniform1ui) numFailed++;
	gl_ProgramUniform1uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLuint *))IntGetProcAddress("glProgramUniform1uiv");
	if(!gl_ProgramUniform1uiv) numFailed++;
	gl_ProgramUniform2d = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLdouble, GLdouble))IntGetProcAddress("glProgramUniform2d");
	if(!gl_ProgramUniform2d) numFailed++;
	gl_ProgramUniform2dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLdouble *))IntGetProcAddress("glProgramUniform2dv");
	if(!gl_ProgramUniform2dv) numFailed++;
	gl_ProgramUniform2f = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLfloat, GLfloat))IntGetProcAddress("glProgramUniform2f");
	if(!gl_ProgramUniform2f) numFailed++;
	gl_ProgramUniform2fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLfloat *))IntGetProcAddress("glProgramUniform2fv");
	if(!gl_ProgramUniform2fv) numFailed++;
	gl_ProgramUniform2i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLint))IntGetProcAddress("glProgramUniform2i");
	if(!gl_ProgramUniform2i) numFailed++;
	gl_ProgramUniform2iv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLint *))IntGetProcAddress("glProgramUniform2iv");
	if(!gl_ProgramUniform2iv) numFailed++;
	gl_ProgramUniform2ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLuint, GLuint))IntGetProcAddress("glProgramUniform2ui");
	if(!gl_ProgramUniform2ui) numFailed++;
	gl_ProgramUniform2uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLuint *))IntGetProcAddress("glProgramUniform2uiv");
	if(!gl_ProgramUniform2uiv) numFailed++;
	gl_ProgramUniform3d = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glProgramUniform3d");
	if(!gl_ProgramUniform3d) numFailed++;
	gl_ProgramUniform3dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLdouble *))IntGetProcAddress("glProgramUniform3dv");
	if(!gl_ProgramUniform3dv) numFailed++;
	gl_ProgramUniform3f = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glProgramUniform3f");
	if(!gl_ProgramUniform3f) numFailed++;
	gl_ProgramUniform3fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLfloat *))IntGetProcAddress("glProgramUniform3fv");
	if(!gl_ProgramUniform3fv) numFailed++;
	gl_ProgramUniform3i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLint, GLint))IntGetProcAddress("glProgramUniform3i");
	if(!gl_ProgramUniform3i) numFailed++;
	gl_ProgramUniform3iv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLint *))IntGetProcAddress("glProgramUniform3iv");
	if(!gl_ProgramUniform3iv) numFailed++;
	gl_ProgramUniform3ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLuint, GLuint, GLuint))IntGetProcAddress("glProgramUniform3ui");
	if(!gl_ProgramUniform3ui) numFailed++;
	gl_ProgramUniform3uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLuint *))IntGetProcAddress("glProgramUniform3uiv");
	if(!gl_ProgramUniform3uiv) numFailed++;
	gl_ProgramUniform4d = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glProgramUniform4d");
	if(!gl_ProgramUniform4d) numFailed++;
	gl_ProgramUniform4dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLdouble *))IntGetProcAddress("glProgramUniform4dv");
	if(!gl_ProgramUniform4dv) numFailed++;
	gl_ProgramUniform4f = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glProgramUniform4f");
	if(!gl_ProgramUniform4f) numFailed++;
	gl_ProgramUniform4fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLfloat *))IntGetProcAddress("glProgramUniform4fv");
	if(!gl_ProgramUniform4fv) numFailed++;
	gl_ProgramUniform4i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLint, GLint, GLint))IntGetProcAddress("glProgramUniform4i");
	if(!gl_ProgramUniform4i) numFailed++;
	gl_ProgramUniform4iv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLint *))IntGetProcAddress("glProgramUniform4iv");
	if(!gl_ProgramUniform4iv) numFailed++;
	gl_ProgramUniform4ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLuint, GLuint, GLuint, GLuint))IntGetProcAddress("glProgramUniform4ui");
	if(!gl_ProgramUniform4ui) numFailed++;
	gl_ProgramUniform4uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLuint *))IntGetProcAddress("glProgramUniform4uiv");
	if(!gl_ProgramUniform4uiv) numFailed++;
	gl_ProgramUniformMatrix2dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix2dv");
	if(!gl_ProgramUniformMatrix2dv) numFailed++;
	gl_ProgramUniformMatrix2fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix2fv");
	if(!gl_ProgramUniformMatrix2fv) numFailed++;
	gl_ProgramUniformMatrix2x3dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix2x3dv");
	if(!gl_ProgramUniformMatrix2x3dv) numFailed++;
	gl_ProgramUniformMatrix2x3fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix2x3fv");
	if(!gl_ProgramUniformMatrix2x3fv) numFailed++;
	gl_ProgramUniformMatrix2x4dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix2x4dv");
	if(!gl_ProgramUniformMatrix2x4dv) numFailed++;
	gl_ProgramUniformMatrix2x4fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix2x4fv");
	if(!gl_ProgramUniformMatrix2x4fv) numFailed++;
	gl_ProgramUniformMatrix3dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix3dv");
	if(!gl_ProgramUniformMatrix3dv) numFailed++;
	gl_ProgramUniformMatrix3fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix3fv");
	if(!gl_ProgramUniformMatrix3fv) numFailed++;
	gl_ProgramUniformMatrix3x2dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix3x2dv");
	if(!gl_ProgramUniformMatrix3x2dv) numFailed++;
	gl_ProgramUniformMatrix3x2fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix3x2fv");
	if(!gl_ProgramUniformMatrix3x2fv) numFailed++;
	gl_ProgramUniformMatrix3x4dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix3x4dv");
	if(!gl_ProgramUniformMatrix3x4dv) numFailed++;
	gl_ProgramUniformMatrix3x4fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix3x4fv");
	if(!gl_ProgramUniformMatrix3x4fv) numFailed++;
	gl_ProgramUniformMatrix4dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix4dv");
	if(!gl_ProgramUniformMatrix4dv) numFailed++;
	gl_ProgramUniformMatrix4fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix4fv");
	if(!gl_ProgramUniformMatrix4fv) numFailed++;
	gl_ProgramUniformMatrix4x2dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix4x2dv");
	if(!gl_ProgramUniformMatrix4x2dv) numFailed++;
	gl_ProgramUniformMatrix4x2fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix4x2fv");
	if(!gl_ProgramUniformMatrix4x2fv) numFailed++;
	gl_ProgramUniformMatrix4x3dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix4x3dv");
	if(!gl_ProgramUniformMatrix4x3dv) numFailed++;
	gl_ProgramUniformMatrix4x3fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix4x3fv");
	if(!gl_ProgramUniformMatrix4x3fv) numFailed++;
	gl_UseProgramStages = (void (CODEGEN_FUNCPTR *)(GLuint, GLbitfield, GLuint))IntGetProcAddress("glUseProgramStages");
	if(!gl_UseProgramStages) numFailed++;
	gl_ValidateProgramPipeline = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glValidateProgramPipeline");
	if(!gl_ValidateProgramPipeline) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_TexBufferRange)(GLenum, GLenum, GLuint, GLintptr, GLsizeiptr) = NULL;

static int Load_ARB_texture_buffer_range()
{
	int numFailed = 0;
	gl_TexBufferRange = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint, GLintptr, GLsizeiptr))IntGetProcAddress("glTexBufferRange");
	if(!gl_TexBufferRange) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_TexStorage1D)(GLenum, GLsizei, GLenum, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_TexStorage2D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_TexStorage3D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei) = NULL;

static int Load_ARB_texture_storage()
{
	int numFailed = 0;
	gl_TexStorage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei))IntGetProcAddress("glTexStorage1D");
	if(!gl_TexStorage1D) numFailed++;
	gl_TexStorage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei))IntGetProcAddress("glTexStorage2D");
	if(!gl_TexStorage2D) numFailed++;
	gl_TexStorage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei))IntGetProcAddress("glTexStorage3D");
	if(!gl_TexStorage3D) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_TextureView)(GLuint, GLenum, GLuint, GLenum, GLuint, GLuint, GLuint, GLuint) = NULL;

static int Load_ARB_texture_view()
{
	int numFailed = 0;
	gl_TextureView = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint, GLenum, GLuint, GLuint, GLuint, GLuint))IntGetProcAddress("glTextureView");
	if(!gl_TextureView) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_BindVertexBuffer)(GLuint, GLuint, GLintptr, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribBinding)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribFormat)(GLuint, GLint, GLenum, GLboolean, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribIFormat)(GLuint, GLint, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribLFormat)(GLuint, GLint, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexBindingDivisor)(GLuint, GLuint) = NULL;

static int Load_ARB_vertex_attrib_binding()
{
	int numFailed = 0;
	gl_BindVertexBuffer = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLintptr, GLsizei))IntGetProcAddress("glBindVertexBuffer");
	if(!gl_BindVertexBuffer) numFailed++;
	gl_VertexAttribBinding = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glVertexAttribBinding");
	if(!gl_VertexAttribBinding) numFailed++;
	gl_VertexAttribFormat = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLenum, GLboolean, GLuint))IntGetProcAddress("glVertexAttribFormat");
	if(!gl_VertexAttribFormat) numFailed++;
	gl_VertexAttribIFormat = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLenum, GLuint))IntGetProcAddress("glVertexAttribIFormat");
	if(!gl_VertexAttribIFormat) numFailed++;
	gl_VertexAttribLFormat = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLenum, GLuint))IntGetProcAddress("glVertexAttribLFormat");
	if(!gl_VertexAttribLFormat) numFailed++;
	gl_VertexBindingDivisor = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glVertexBindingDivisor");
	if(!gl_VertexBindingDivisor) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_DepthRangeArrayv)(GLuint, GLsizei, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_DepthRangeIndexed)(GLuint, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *gl_GetDoublei_v)(GLenum, GLuint, GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetFloati_v)(GLenum, GLuint, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ScissorArrayv)(GLuint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ScissorIndexed)(GLuint, GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_ScissorIndexedv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ViewportArrayv)(GLuint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ViewportIndexedf)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_ViewportIndexedfv)(GLuint, const GLfloat *) = NULL;

static int Load_ARB_viewport_array()
{
	int numFailed = 0;
	gl_DepthRangeArrayv = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLdouble *))IntGetProcAddress("glDepthRangeArrayv");
	if(!gl_DepthRangeArrayv) numFailed++;
	gl_DepthRangeIndexed = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble, GLdouble))IntGetProcAddress("glDepthRangeIndexed");
	if(!gl_DepthRangeIndexed) numFailed++;
	gl_GetDoublei_v = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLdouble *))IntGetProcAddress("glGetDoublei_v");
	if(!gl_GetDoublei_v) numFailed++;
	gl_GetFloati_v = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLfloat *))IntGetProcAddress("glGetFloati_v");
	if(!gl_GetFloati_v) numFailed++;
	gl_ScissorArrayv = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLint *))IntGetProcAddress("glScissorArrayv");
	if(!gl_ScissorArrayv) numFailed++;
	gl_ScissorIndexed = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glScissorIndexed");
	if(!gl_ScissorIndexed) numFailed++;
	gl_ScissorIndexedv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glScissorIndexedv");
	if(!gl_ScissorIndexedv) numFailed++;
	gl_ViewportArrayv = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLfloat *))IntGetProcAddress("glViewportArrayv");
	if(!gl_ViewportArrayv) numFailed++;
	gl_ViewportIndexedf = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glViewportIndexedf");
	if(!gl_ViewportIndexedf) numFailed++;
	gl_ViewportIndexedfv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glViewportIndexedfv");
	if(!gl_ViewportIndexedfv) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_ClearBufferData)(GLenum, GLenum, GLenum, GLenum, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_ClearBufferSubData)(GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void *) = NULL;

static int Load_ARB_clear_buffer_object()
{
	int numFailed = 0;
	gl_ClearBufferData = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLenum, const void *))IntGetProcAddress("glClearBufferData");
	if(!gl_ClearBufferData) numFailed++;
	gl_ClearBufferSubData = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void *))IntGetProcAddress("glClearBufferSubData");
	if(!gl_ClearBufferSubData) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_CopyImageSubData)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei) = NULL;

static int Load_ARB_copy_image()
{
	int numFailed = 0;
	gl_CopyImageSubData = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei))IntGetProcAddress("glCopyImageSubData");
	if(!gl_CopyImageSubData) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_FramebufferParameteri)(GLenum, GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_GetFramebufferParameteriv)(GLenum, GLenum, GLint *) = NULL;

static int Load_ARB_framebuffer_no_attachments()
{
	int numFailed = 0;
	gl_FramebufferParameteri = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint))IntGetProcAddress("glFramebufferParameteri");
	if(!gl_FramebufferParameteri) numFailed++;
	gl_GetFramebufferParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetFramebufferParameteriv");
	if(!gl_GetFramebufferParameteriv) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_InvalidateBufferData)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_InvalidateBufferSubData)(GLuint, GLintptr, GLsizeiptr) = NULL;
void (CODEGEN_FUNCPTR *gl_InvalidateFramebuffer)(GLenum, GLsizei, const GLenum *) = NULL;
void (CODEGEN_FUNCPTR *gl_InvalidateSubFramebuffer)(GLenum, GLsizei, const GLenum *, GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_InvalidateTexImage)(GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_InvalidateTexSubImage)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei) = NULL;

static int Load_ARB_invalidate_subdata()
{
	int numFailed = 0;
	gl_InvalidateBufferData = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glInvalidateBufferData");
	if(!gl_InvalidateBufferData) numFailed++;
	gl_InvalidateBufferSubData = (void (CODEGEN_FUNCPTR *)(GLuint, GLintptr, GLsizeiptr))IntGetProcAddress("glInvalidateBufferSubData");
	if(!gl_InvalidateBufferSubData) numFailed++;
	gl_InvalidateFramebuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLenum *))IntGetProcAddress("glInvalidateFramebuffer");
	if(!gl_InvalidateFramebuffer) numFailed++;
	gl_InvalidateSubFramebuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLenum *, GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glInvalidateSubFramebuffer");
	if(!gl_InvalidateSubFramebuffer) numFailed++;
	gl_InvalidateTexImage = (void (CODEGEN_FUNCPTR *)(GLuint, GLint))IntGetProcAddress("glInvalidateTexImage");
	if(!gl_InvalidateTexImage) numFailed++;
	gl_InvalidateTexSubImage = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei))IntGetProcAddress("glInvalidateTexSubImage");
	if(!gl_InvalidateTexSubImage) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_TexStorage2DMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *gl_TexStorage3DMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean) = NULL;

static int Load_ARB_texture_storage_multisample()
{
	int numFailed = 0;
	gl_TexStorage2DMultisample = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean))IntGetProcAddress("glTexStorage2DMultisample");
	if(!gl_TexStorage2DMultisample) numFailed++;
	gl_TexStorage3DMultisample = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean))IntGetProcAddress("glTexStorage3DMultisample");
	if(!gl_TexStorage3DMultisample) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_DebugMessageCallback)(GLDEBUGPROC, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_DebugMessageControl)(GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *gl_DebugMessageInsert)(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *) = NULL;
GLuint (CODEGEN_FUNCPTR *gl_GetDebugMessageLog)(GLuint, GLsizei, GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetObjectLabel)(GLenum, GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetObjectPtrLabel)(const void *, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetPointerv)(GLenum, void **) = NULL;
void (CODEGEN_FUNCPTR *gl_ObjectLabel)(GLenum, GLuint, GLsizei, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_ObjectPtrLabel)(const void *, GLsizei, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_PopDebugGroup)() = NULL;
void (CODEGEN_FUNCPTR *gl_PushDebugGroup)(GLenum, GLuint, GLsizei, const GLchar *) = NULL;

static int Load_KHR_debug()
{
	int numFailed = 0;
	gl_DebugMessageCallback = (void (CODEGEN_FUNCPTR *)(GLDEBUGPROC, const void *))IntGetProcAddress("glDebugMessageCallback");
	if(!gl_DebugMessageCallback) numFailed++;
	gl_DebugMessageControl = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean))IntGetProcAddress("glDebugMessageControl");
	if(!gl_DebugMessageControl) numFailed++;
	gl_DebugMessageInsert = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *))IntGetProcAddress("glDebugMessageInsert");
	if(!gl_DebugMessageInsert) numFailed++;
	gl_GetDebugMessageLog = (GLuint (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *))IntGetProcAddress("glGetDebugMessageLog");
	if(!gl_GetDebugMessageLog) numFailed++;
	gl_GetObjectLabel = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetObjectLabel");
	if(!gl_GetObjectLabel) numFailed++;
	gl_GetObjectPtrLabel = (void (CODEGEN_FUNCPTR *)(const void *, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetObjectPtrLabel");
	if(!gl_GetObjectPtrLabel) numFailed++;
	gl_GetPointerv = (void (CODEGEN_FUNCPTR *)(GLenum, void **))IntGetProcAddress("glGetPointerv");
	if(!gl_GetPointerv) numFailed++;
	gl_ObjectLabel = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLsizei, const GLchar *))IntGetProcAddress("glObjectLabel");
	if(!gl_ObjectLabel) numFailed++;
	gl_ObjectPtrLabel = (void (CODEGEN_FUNCPTR *)(const void *, GLsizei, const GLchar *))IntGetProcAddress("glObjectPtrLabel");
	if(!gl_ObjectPtrLabel) numFailed++;
	gl_PopDebugGroup = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glPopDebugGroup");
	if(!gl_PopDebugGroup) numFailed++;
	gl_PushDebugGroup = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLsizei, const GLchar *))IntGetProcAddress("glPushDebugGroup");
	if(!gl_PushDebugGroup) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *gl_BlendFunc)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_Clear)(GLbitfield) = NULL;
void (CODEGEN_FUNCPTR *gl_ClearColor)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_ClearDepth)(GLdouble) = NULL;
void (CODEGEN_FUNCPTR *gl_ClearStencil)(GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_ColorMask)(GLboolean, GLboolean, GLboolean, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *gl_CullFace)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_DepthFunc)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_DepthMask)(GLboolean) = NULL;
void (CODEGEN_FUNCPTR *gl_DepthRange)(GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *gl_Disable)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_DrawBuffer)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_Enable)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_Finish)() = NULL;
void (CODEGEN_FUNCPTR *gl_Flush)() = NULL;
void (CODEGEN_FUNCPTR *gl_FrontFace)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_GetBooleanv)(GLenum, GLboolean *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetDoublev)(GLenum, GLdouble *) = NULL;
GLenum (CODEGEN_FUNCPTR *gl_GetError)() = NULL;
void (CODEGEN_FUNCPTR *gl_GetFloatv)(GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetIntegerv)(GLenum, GLint *) = NULL;
const GLubyte * (CODEGEN_FUNCPTR *gl_GetString)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_GetTexImage)(GLenum, GLint, GLenum, GLenum, void *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetTexLevelParameterfv)(GLenum, GLint, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetTexLevelParameteriv)(GLenum, GLint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetTexParameterfv)(GLenum, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetTexParameteriv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_Hint)(GLenum, GLenum) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsEnabled)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_LineWidth)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_LogicOp)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_PixelStoref)(GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_PixelStorei)(GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_PointSize)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_PolygonMode)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_ReadBuffer)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_ReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *) = NULL;
void (CODEGEN_FUNCPTR *gl_Scissor)(GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_StencilFunc)(GLenum, GLint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_StencilMask)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_StencilOp)(GLenum, GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_TexImage1D)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_TexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_TexParameterf)(GLenum, GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_TexParameterfv)(GLenum, GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_TexParameteri)(GLenum, GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_TexParameteriv)(GLenum, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_Viewport)(GLint, GLint, GLsizei, GLsizei) = NULL;

void (CODEGEN_FUNCPTR *gl_BindTexture)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_CopyTexImage1D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_CopyTexImage2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_CopyTexSubImage1D)(GLenum, GLint, GLint, GLint, GLint, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_CopyTexSubImage2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_DeleteTextures)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_DrawArrays)(GLenum, GLint, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_DrawElements)(GLenum, GLsizei, GLenum, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_GenTextures)(GLsizei, GLuint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsTexture)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_PolygonOffset)(GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_TexSubImage1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_TexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *) = NULL;

void (CODEGEN_FUNCPTR *gl_CopyTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_DrawRangeElements)(GLenum, GLuint, GLuint, GLsizei, GLenum, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_TexImage3D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_TexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *) = NULL;

void (CODEGEN_FUNCPTR *gl_ActiveTexture)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_CompressedTexImage1D)(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_CompressedTexImage2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_CompressedTexImage3D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_CompressedTexSubImage1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_CompressedTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_CompressedTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetCompressedTexImage)(GLenum, GLint, void *) = NULL;
void (CODEGEN_FUNCPTR *gl_SampleCoverage)(GLfloat, GLboolean) = NULL;

void (CODEGEN_FUNCPTR *gl_BlendFuncSeparate)(GLenum, GLenum, GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_MultiDrawArrays)(GLenum, const GLint *, const GLsizei *, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_MultiDrawElements)(GLenum, const GLsizei *, GLenum, const void *const*, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_PointParameterf)(GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_PointParameterfv)(GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_PointParameteri)(GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_PointParameteriv)(GLenum, const GLint *) = NULL;

void (CODEGEN_FUNCPTR *gl_BeginQuery)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_BindBuffer)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_BufferData)(GLenum, GLsizeiptr, const void *, GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_BufferSubData)(GLenum, GLintptr, GLsizeiptr, const void *) = NULL;
void (CODEGEN_FUNCPTR *gl_DeleteBuffers)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_DeleteQueries)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_EndQuery)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_GenBuffers)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GenQueries)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetBufferParameteriv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetBufferPointerv)(GLenum, GLenum, void **) = NULL;
void (CODEGEN_FUNCPTR *gl_GetBufferSubData)(GLenum, GLintptr, GLsizeiptr, void *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetQueryObjectiv)(GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetQueryObjectuiv)(GLuint, GLenum, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetQueryiv)(GLenum, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsBuffer)(GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsQuery)(GLuint) = NULL;
void * (CODEGEN_FUNCPTR *gl_MapBuffer)(GLenum, GLenum) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_UnmapBuffer)(GLenum) = NULL;

void (CODEGEN_FUNCPTR *gl_AttachShader)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_BindAttribLocation)(GLuint, GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_BlendEquationSeparate)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_CompileShader)(GLuint) = NULL;
GLuint (CODEGEN_FUNCPTR *gl_CreateProgram)() = NULL;
GLuint (CODEGEN_FUNCPTR *gl_CreateShader)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_DeleteProgram)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_DeleteShader)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_DetachShader)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_DisableVertexAttribArray)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_DrawBuffers)(GLsizei, const GLenum *) = NULL;
void (CODEGEN_FUNCPTR *gl_EnableVertexAttribArray)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_GetActiveAttrib)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetActiveUniform)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetAttachedShaders)(GLuint, GLsizei, GLsizei *, GLuint *) = NULL;
GLint (CODEGEN_FUNCPTR *gl_GetAttribLocation)(GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetProgramInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetProgramiv)(GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetShaderInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetShaderSource)(GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetShaderiv)(GLuint, GLenum, GLint *) = NULL;
GLint (CODEGEN_FUNCPTR *gl_GetUniformLocation)(GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetUniformfv)(GLuint, GLint, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetUniformiv)(GLuint, GLint, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetVertexAttribPointerv)(GLuint, GLenum, void **) = NULL;
void (CODEGEN_FUNCPTR *gl_GetVertexAttribdv)(GLuint, GLenum, GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetVertexAttribfv)(GLuint, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetVertexAttribiv)(GLuint, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsProgram)(GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsShader)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_LinkProgram)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_ShaderSource)(GLuint, GLsizei, const GLchar *const*, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_StencilFuncSeparate)(GLenum, GLenum, GLint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_StencilMaskSeparate)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_StencilOpSeparate)(GLenum, GLenum, GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform1f)(GLint, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform1fv)(GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform1i)(GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform1iv)(GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform2f)(GLint, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform2fv)(GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform2i)(GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform2iv)(GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform3f)(GLint, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform3fv)(GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform3i)(GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform3iv)(GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform4fv)(GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform4i)(GLint, GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform4iv)(GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_UniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_UniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_UniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_UseProgram)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_ValidateProgram)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib1d)(GLuint, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib1dv)(GLuint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib1f)(GLuint, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib1fv)(GLuint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib1s)(GLuint, GLshort) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib1sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib2d)(GLuint, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib2dv)(GLuint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib2f)(GLuint, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib2fv)(GLuint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib2s)(GLuint, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib2sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib3d)(GLuint, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib3dv)(GLuint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib3f)(GLuint, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib3fv)(GLuint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib3s)(GLuint, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib3sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4Nbv)(GLuint, const GLbyte *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4Niv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4Nsv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4Nub)(GLuint, GLubyte, GLubyte, GLubyte, GLubyte) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4Nubv)(GLuint, const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4Nuiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4Nusv)(GLuint, const GLushort *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4bv)(GLuint, const GLbyte *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4d)(GLuint, GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4dv)(GLuint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4f)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4fv)(GLuint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4iv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4s)(GLuint, GLshort, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4ubv)(GLuint, const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4uiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttrib4usv)(GLuint, const GLushort *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *) = NULL;

void (CODEGEN_FUNCPTR *gl_UniformMatrix2x3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_UniformMatrix2x4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_UniformMatrix3x2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_UniformMatrix3x4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_UniformMatrix4x2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_UniformMatrix4x3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;

void (CODEGEN_FUNCPTR *gl_BeginConditionalRender)(GLuint, GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_BeginTransformFeedback)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_BindBufferBase)(GLenum, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_BindBufferRange)(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr) = NULL;
void (CODEGEN_FUNCPTR *gl_BindFragDataLocation)(GLuint, GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_BindFramebuffer)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_BindRenderbuffer)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_BindVertexArray)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_BlitFramebuffer)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum) = NULL;
GLenum (CODEGEN_FUNCPTR *gl_CheckFramebufferStatus)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_ClampColor)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_ClearBufferfi)(GLenum, GLint, GLfloat, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_ClearBufferfv)(GLenum, GLint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_ClearBufferiv)(GLenum, GLint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ClearBufferuiv)(GLenum, GLint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ColorMaski)(GLuint, GLboolean, GLboolean, GLboolean, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *gl_DeleteFramebuffers)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_DeleteRenderbuffers)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_DeleteVertexArrays)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_Disablei)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_Enablei)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_EndConditionalRender)() = NULL;
void (CODEGEN_FUNCPTR *gl_EndTransformFeedback)() = NULL;
void (CODEGEN_FUNCPTR *gl_FlushMappedBufferRange)(GLenum, GLintptr, GLsizeiptr) = NULL;
void (CODEGEN_FUNCPTR *gl_FramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_FramebufferTexture1D)(GLenum, GLenum, GLenum, GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_FramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_FramebufferTexture3D)(GLenum, GLenum, GLenum, GLuint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_FramebufferTextureLayer)(GLenum, GLenum, GLuint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_GenFramebuffers)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GenRenderbuffers)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GenVertexArrays)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GenerateMipmap)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_GetBooleani_v)(GLenum, GLuint, GLboolean *) = NULL;
GLint (CODEGEN_FUNCPTR *gl_GetFragDataLocation)(GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetFramebufferAttachmentParameteriv)(GLenum, GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetIntegeri_v)(GLenum, GLuint, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetRenderbufferParameteriv)(GLenum, GLenum, GLint *) = NULL;
const GLubyte * (CODEGEN_FUNCPTR *gl_GetStringi)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_GetTexParameterIiv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetTexParameterIuiv)(GLenum, GLenum, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetTransformFeedbackVarying)(GLuint, GLuint, GLsizei, GLsizei *, GLsizei *, GLenum *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetUniformuiv)(GLuint, GLint, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetVertexAttribIiv)(GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetVertexAttribIuiv)(GLuint, GLenum, GLuint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsEnabledi)(GLenum, GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsFramebuffer)(GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsRenderbuffer)(GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsVertexArray)(GLuint) = NULL;
void * (CODEGEN_FUNCPTR *gl_MapBufferRange)(GLenum, GLintptr, GLsizeiptr, GLbitfield) = NULL;
void (CODEGEN_FUNCPTR *gl_RenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_RenderbufferStorageMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_TexParameterIiv)(GLenum, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_TexParameterIuiv)(GLenum, GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_TransformFeedbackVaryings)(GLuint, GLsizei, const GLchar *const*, GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform1ui)(GLint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform1uiv)(GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform2ui)(GLint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform2uiv)(GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform3ui)(GLint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform3uiv)(GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform4ui)(GLint, GLuint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_Uniform4uiv)(GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI1i)(GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI1iv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI1ui)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI1uiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI2i)(GLuint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI2iv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI2ui)(GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI2uiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI3i)(GLuint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI3iv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI3ui)(GLuint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI3uiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI4bv)(GLuint, const GLbyte *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI4i)(GLuint, GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI4iv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI4sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI4ubv)(GLuint, const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI4ui)(GLuint, GLuint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI4uiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribI4usv)(GLuint, const GLushort *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribIPointer)(GLuint, GLint, GLenum, GLsizei, const void *) = NULL;

void (CODEGEN_FUNCPTR *gl_CopyBufferSubData)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr) = NULL;
void (CODEGEN_FUNCPTR *gl_DrawArraysInstanced)(GLenum, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_DrawElementsInstanced)(GLenum, GLsizei, GLenum, const void *, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *gl_GetActiveUniformBlockName)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetActiveUniformBlockiv)(GLuint, GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetActiveUniformName)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetActiveUniformsiv)(GLuint, GLsizei, const GLuint *, GLenum, GLint *) = NULL;
GLuint (CODEGEN_FUNCPTR *gl_GetUniformBlockIndex)(GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetUniformIndices)(GLuint, GLsizei, const GLchar *const*, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_PrimitiveRestartIndex)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_TexBuffer)(GLenum, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_UniformBlockBinding)(GLuint, GLuint, GLuint) = NULL;

GLenum (CODEGEN_FUNCPTR *gl_ClientWaitSync)(GLsync, GLbitfield, GLuint64) = NULL;
void (CODEGEN_FUNCPTR *gl_DeleteSync)(GLsync) = NULL;
void (CODEGEN_FUNCPTR *gl_DrawElementsBaseVertex)(GLenum, GLsizei, GLenum, const void *, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_DrawElementsInstancedBaseVertex)(GLenum, GLsizei, GLenum, const void *, GLsizei, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_DrawRangeElementsBaseVertex)(GLenum, GLuint, GLuint, GLsizei, GLenum, const void *, GLint) = NULL;
GLsync (CODEGEN_FUNCPTR *gl_FenceSync)(GLenum, GLbitfield) = NULL;
void (CODEGEN_FUNCPTR *gl_FramebufferTexture)(GLenum, GLenum, GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_GetBufferParameteri64v)(GLenum, GLenum, GLint64 *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetInteger64i_v)(GLenum, GLuint, GLint64 *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetInteger64v)(GLenum, GLint64 *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetMultisamplefv)(GLenum, GLuint, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetSynciv)(GLsync, GLenum, GLsizei, GLsizei *, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsSync)(GLsync) = NULL;
void (CODEGEN_FUNCPTR *gl_MultiDrawElementsBaseVertex)(GLenum, const GLsizei *, GLenum, const void *const*, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_ProvokingVertex)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_SampleMaski)(GLuint, GLbitfield) = NULL;
void (CODEGEN_FUNCPTR *gl_TexImage2DMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *gl_TexImage3DMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *gl_WaitSync)(GLsync, GLbitfield, GLuint64) = NULL;

void (CODEGEN_FUNCPTR *gl_BindFragDataLocationIndexed)(GLuint, GLuint, GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_BindSampler)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_DeleteSamplers)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GenSamplers)(GLsizei, GLuint *) = NULL;
GLint (CODEGEN_FUNCPTR *gl_GetFragDataIndex)(GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetQueryObjecti64v)(GLuint, GLenum, GLint64 *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetQueryObjectui64v)(GLuint, GLenum, GLuint64 *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetSamplerParameterIiv)(GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetSamplerParameterIuiv)(GLuint, GLenum, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetSamplerParameterfv)(GLuint, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_GetSamplerParameteriv)(GLuint, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *gl_IsSampler)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_QueryCounter)(GLuint, GLenum) = NULL;
void (CODEGEN_FUNCPTR *gl_SamplerParameterIiv)(GLuint, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_SamplerParameterIuiv)(GLuint, GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_SamplerParameterf)(GLuint, GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *gl_SamplerParameterfv)(GLuint, GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *gl_SamplerParameteri)(GLuint, GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *gl_SamplerParameteriv)(GLuint, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribDivisor)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribP1ui)(GLuint, GLenum, GLboolean, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribP1uiv)(GLuint, GLenum, GLboolean, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribP2ui)(GLuint, GLenum, GLboolean, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribP2uiv)(GLuint, GLenum, GLboolean, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribP3ui)(GLuint, GLenum, GLboolean, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribP3uiv)(GLuint, GLenum, GLboolean, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribP4ui)(GLuint, GLenum, GLboolean, GLuint) = NULL;
void (CODEGEN_FUNCPTR *gl_VertexAttribP4uiv)(GLuint, GLenum, GLboolean, const GLuint *) = NULL;

static int Load_Version_3_3()
{
	int numFailed = 0;
	gl_BlendFunc = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glBlendFunc");
	if(!gl_BlendFunc) numFailed++;
	gl_Clear = (void (CODEGEN_FUNCPTR *)(GLbitfield))IntGetProcAddress("glClear");
	if(!gl_Clear) numFailed++;
	gl_ClearColor = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glClearColor");
	if(!gl_ClearColor) numFailed++;
	gl_ClearDepth = (void (CODEGEN_FUNCPTR *)(GLdouble))IntGetProcAddress("glClearDepth");
	if(!gl_ClearDepth) numFailed++;
	gl_ClearStencil = (void (CODEGEN_FUNCPTR *)(GLint))IntGetProcAddress("glClearStencil");
	if(!gl_ClearStencil) numFailed++;
	gl_ColorMask = (void (CODEGEN_FUNCPTR *)(GLboolean, GLboolean, GLboolean, GLboolean))IntGetProcAddress("glColorMask");
	if(!gl_ColorMask) numFailed++;
	gl_CullFace = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glCullFace");
	if(!gl_CullFace) numFailed++;
	gl_DepthFunc = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glDepthFunc");
	if(!gl_DepthFunc) numFailed++;
	gl_DepthMask = (void (CODEGEN_FUNCPTR *)(GLboolean))IntGetProcAddress("glDepthMask");
	if(!gl_DepthMask) numFailed++;
	gl_DepthRange = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble))IntGetProcAddress("glDepthRange");
	if(!gl_DepthRange) numFailed++;
	gl_Disable = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glDisable");
	if(!gl_Disable) numFailed++;
	gl_DrawBuffer = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glDrawBuffer");
	if(!gl_DrawBuffer) numFailed++;
	gl_Enable = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glEnable");
	if(!gl_Enable) numFailed++;
	gl_Finish = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glFinish");
	if(!gl_Finish) numFailed++;
	gl_Flush = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glFlush");
	if(!gl_Flush) numFailed++;
	gl_FrontFace = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glFrontFace");
	if(!gl_FrontFace) numFailed++;
	gl_GetBooleanv = (void (CODEGEN_FUNCPTR *)(GLenum, GLboolean *))IntGetProcAddress("glGetBooleanv");
	if(!gl_GetBooleanv) numFailed++;
	gl_GetDoublev = (void (CODEGEN_FUNCPTR *)(GLenum, GLdouble *))IntGetProcAddress("glGetDoublev");
	if(!gl_GetDoublev) numFailed++;
	gl_GetError = (GLenum (CODEGEN_FUNCPTR *)())IntGetProcAddress("glGetError");
	if(!gl_GetError) numFailed++;
	gl_GetFloatv = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat *))IntGetProcAddress("glGetFloatv");
	if(!gl_GetFloatv) numFailed++;
	gl_GetIntegerv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint *))IntGetProcAddress("glGetIntegerv");
	if(!gl_GetIntegerv) numFailed++;
	gl_GetString = (const GLubyte * (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glGetString");
	if(!gl_GetString) numFailed++;
	gl_GetTexImage = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLenum, void *))IntGetProcAddress("glGetTexImage");
	if(!gl_GetTexImage) numFailed++;
	gl_GetTexLevelParameterfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLfloat *))IntGetProcAddress("glGetTexLevelParameterfv");
	if(!gl_GetTexLevelParameterfv) numFailed++;
	gl_GetTexLevelParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLint *))IntGetProcAddress("glGetTexLevelParameteriv");
	if(!gl_GetTexLevelParameteriv) numFailed++;
	gl_GetTexParameterfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat *))IntGetProcAddress("glGetTexParameterfv");
	if(!gl_GetTexParameterfv) numFailed++;
	gl_GetTexParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetTexParameteriv");
	if(!gl_GetTexParameteriv) numFailed++;
	gl_Hint = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glHint");
	if(!gl_Hint) numFailed++;
	gl_IsEnabled = (GLboolean (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glIsEnabled");
	if(!gl_IsEnabled) numFailed++;
	gl_LineWidth = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glLineWidth");
	if(!gl_LineWidth) numFailed++;
	gl_LogicOp = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glLogicOp");
	if(!gl_LogicOp) numFailed++;
	gl_PixelStoref = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat))IntGetProcAddress("glPixelStoref");
	if(!gl_PixelStoref) numFailed++;
	gl_PixelStorei = (void (CODEGEN_FUNCPTR *)(GLenum, GLint))IntGetProcAddress("glPixelStorei");
	if(!gl_PixelStorei) numFailed++;
	gl_PointSize = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glPointSize");
	if(!gl_PointSize) numFailed++;
	gl_PolygonMode = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glPolygonMode");
	if(!gl_PolygonMode) numFailed++;
	gl_ReadBuffer = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glReadBuffer");
	if(!gl_ReadBuffer) numFailed++;
	gl_ReadPixels = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *))IntGetProcAddress("glReadPixels");
	if(!gl_ReadPixels) numFailed++;
	gl_Scissor = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glScissor");
	if(!gl_Scissor) numFailed++;
	gl_StencilFunc = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLuint))IntGetProcAddress("glStencilFunc");
	if(!gl_StencilFunc) numFailed++;
	gl_StencilMask = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glStencilMask");
	if(!gl_StencilMask) numFailed++;
	gl_StencilOp = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum))IntGetProcAddress("glStencilOp");
	if(!gl_StencilOp) numFailed++;
	gl_TexImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const void *))IntGetProcAddress("glTexImage1D");
	if(!gl_TexImage1D) numFailed++;
	gl_TexImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *))IntGetProcAddress("glTexImage2D");
	if(!gl_TexImage2D) numFailed++;
	gl_TexParameterf = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat))IntGetProcAddress("glTexParameterf");
	if(!gl_TexParameterf) numFailed++;
	gl_TexParameterfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLfloat *))IntGetProcAddress("glTexParameterfv");
	if(!gl_TexParameterfv) numFailed++;
	gl_TexParameteri = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint))IntGetProcAddress("glTexParameteri");
	if(!gl_TexParameteri) numFailed++;
	gl_TexParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLint *))IntGetProcAddress("glTexParameteriv");
	if(!gl_TexParameteriv) numFailed++;
	gl_Viewport = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glViewport");
	if(!gl_Viewport) numFailed++;
	gl_BindTexture = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindTexture");
	if(!gl_BindTexture) numFailed++;
	gl_CopyTexImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint))IntGetProcAddress("glCopyTexImage1D");
	if(!gl_CopyTexImage1D) numFailed++;
	gl_CopyTexImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint))IntGetProcAddress("glCopyTexImage2D");
	if(!gl_CopyTexImage2D) numFailed++;
	gl_CopyTexSubImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLsizei))IntGetProcAddress("glCopyTexSubImage1D");
	if(!gl_CopyTexSubImage1D) numFailed++;
	gl_CopyTexSubImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glCopyTexSubImage2D");
	if(!gl_CopyTexSubImage2D) numFailed++;
	gl_DeleteTextures = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteTextures");
	if(!gl_DeleteTextures) numFailed++;
	gl_DrawArrays = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLsizei))IntGetProcAddress("glDrawArrays");
	if(!gl_DrawArrays) numFailed++;
	gl_DrawElements = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, const void *))IntGetProcAddress("glDrawElements");
	if(!gl_DrawElements) numFailed++;
	gl_GenTextures = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenTextures");
	if(!gl_GenTextures) numFailed++;
	gl_IsTexture = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsTexture");
	if(!gl_IsTexture) numFailed++;
	gl_PolygonOffset = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat))IntGetProcAddress("glPolygonOffset");
	if(!gl_PolygonOffset) numFailed++;
	gl_TexSubImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const void *))IntGetProcAddress("glTexSubImage1D");
	if(!gl_TexSubImage1D) numFailed++;
	gl_TexSubImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *))IntGetProcAddress("glTexSubImage2D");
	if(!gl_TexSubImage2D) numFailed++;
	gl_CopyTexSubImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glCopyTexSubImage3D");
	if(!gl_CopyTexSubImage3D) numFailed++;
	gl_DrawRangeElements = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLuint, GLsizei, GLenum, const void *))IntGetProcAddress("glDrawRangeElements");
	if(!gl_DrawRangeElements) numFailed++;
	gl_TexImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *))IntGetProcAddress("glTexImage3D");
	if(!gl_TexImage3D) numFailed++;
	gl_TexSubImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *))IntGetProcAddress("glTexSubImage3D");
	if(!gl_TexSubImage3D) numFailed++;
	gl_ActiveTexture = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glActiveTexture");
	if(!gl_ActiveTexture) numFailed++;
	gl_CompressedTexImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const void *))IntGetProcAddress("glCompressedTexImage1D");
	if(!gl_CompressedTexImage1D) numFailed++;
	gl_CompressedTexImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void *))IntGetProcAddress("glCompressedTexImage2D");
	if(!gl_CompressedTexImage2D) numFailed++;
	gl_CompressedTexImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void *))IntGetProcAddress("glCompressedTexImage3D");
	if(!gl_CompressedTexImage3D) numFailed++;
	gl_CompressedTexSubImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const void *))IntGetProcAddress("glCompressedTexSubImage1D");
	if(!gl_CompressedTexSubImage1D) numFailed++;
	gl_CompressedTexSubImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void *))IntGetProcAddress("glCompressedTexSubImage2D");
	if(!gl_CompressedTexSubImage2D) numFailed++;
	gl_CompressedTexSubImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void *))IntGetProcAddress("glCompressedTexSubImage3D");
	if(!gl_CompressedTexSubImage3D) numFailed++;
	gl_GetCompressedTexImage = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, void *))IntGetProcAddress("glGetCompressedTexImage");
	if(!gl_GetCompressedTexImage) numFailed++;
	gl_SampleCoverage = (void (CODEGEN_FUNCPTR *)(GLfloat, GLboolean))IntGetProcAddress("glSampleCoverage");
	if(!gl_SampleCoverage) numFailed++;
	gl_BlendFuncSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLenum))IntGetProcAddress("glBlendFuncSeparate");
	if(!gl_BlendFuncSeparate) numFailed++;
	gl_MultiDrawArrays = (void (CODEGEN_FUNCPTR *)(GLenum, const GLint *, const GLsizei *, GLsizei))IntGetProcAddress("glMultiDrawArrays");
	if(!gl_MultiDrawArrays) numFailed++;
	gl_MultiDrawElements = (void (CODEGEN_FUNCPTR *)(GLenum, const GLsizei *, GLenum, const void *const*, GLsizei))IntGetProcAddress("glMultiDrawElements");
	if(!gl_MultiDrawElements) numFailed++;
	gl_PointParameterf = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat))IntGetProcAddress("glPointParameterf");
	if(!gl_PointParameterf) numFailed++;
	gl_PointParameterfv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLfloat *))IntGetProcAddress("glPointParameterfv");
	if(!gl_PointParameterfv) numFailed++;
	gl_PointParameteri = (void (CODEGEN_FUNCPTR *)(GLenum, GLint))IntGetProcAddress("glPointParameteri");
	if(!gl_PointParameteri) numFailed++;
	gl_PointParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLint *))IntGetProcAddress("glPointParameteriv");
	if(!gl_PointParameteriv) numFailed++;
	gl_BeginQuery = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBeginQuery");
	if(!gl_BeginQuery) numFailed++;
	gl_BindBuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindBuffer");
	if(!gl_BindBuffer) numFailed++;
	gl_BufferData = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizeiptr, const void *, GLenum))IntGetProcAddress("glBufferData");
	if(!gl_BufferData) numFailed++;
	gl_BufferSubData = (void (CODEGEN_FUNCPTR *)(GLenum, GLintptr, GLsizeiptr, const void *))IntGetProcAddress("glBufferSubData");
	if(!gl_BufferSubData) numFailed++;
	gl_DeleteBuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteBuffers");
	if(!gl_DeleteBuffers) numFailed++;
	gl_DeleteQueries = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteQueries");
	if(!gl_DeleteQueries) numFailed++;
	gl_EndQuery = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glEndQuery");
	if(!gl_EndQuery) numFailed++;
	gl_GenBuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenBuffers");
	if(!gl_GenBuffers) numFailed++;
	gl_GenQueries = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenQueries");
	if(!gl_GenQueries) numFailed++;
	gl_GetBufferParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetBufferParameteriv");
	if(!gl_GetBufferParameteriv) numFailed++;
	gl_GetBufferPointerv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, void **))IntGetProcAddress("glGetBufferPointerv");
	if(!gl_GetBufferPointerv) numFailed++;
	gl_GetBufferSubData = (void (CODEGEN_FUNCPTR *)(GLenum, GLintptr, GLsizeiptr, void *))IntGetProcAddress("glGetBufferSubData");
	if(!gl_GetBufferSubData) numFailed++;
	gl_GetQueryObjectiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetQueryObjectiv");
	if(!gl_GetQueryObjectiv) numFailed++;
	gl_GetQueryObjectuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint *))IntGetProcAddress("glGetQueryObjectuiv");
	if(!gl_GetQueryObjectuiv) numFailed++;
	gl_GetQueryiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetQueryiv");
	if(!gl_GetQueryiv) numFailed++;
	gl_IsBuffer = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsBuffer");
	if(!gl_IsBuffer) numFailed++;
	gl_IsQuery = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsQuery");
	if(!gl_IsQuery) numFailed++;
	gl_MapBuffer = (void * (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glMapBuffer");
	if(!gl_MapBuffer) numFailed++;
	gl_UnmapBuffer = (GLboolean (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glUnmapBuffer");
	if(!gl_UnmapBuffer) numFailed++;
	gl_AttachShader = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glAttachShader");
	if(!gl_AttachShader) numFailed++;
	gl_BindAttribLocation = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, const GLchar *))IntGetProcAddress("glBindAttribLocation");
	if(!gl_BindAttribLocation) numFailed++;
	gl_BlendEquationSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glBlendEquationSeparate");
	if(!gl_BlendEquationSeparate) numFailed++;
	gl_CompileShader = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glCompileShader");
	if(!gl_CompileShader) numFailed++;
	gl_CreateProgram = (GLuint (CODEGEN_FUNCPTR *)())IntGetProcAddress("glCreateProgram");
	if(!gl_CreateProgram) numFailed++;
	gl_CreateShader = (GLuint (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glCreateShader");
	if(!gl_CreateShader) numFailed++;
	gl_DeleteProgram = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glDeleteProgram");
	if(!gl_DeleteProgram) numFailed++;
	gl_DeleteShader = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glDeleteShader");
	if(!gl_DeleteShader) numFailed++;
	gl_DetachShader = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glDetachShader");
	if(!gl_DetachShader) numFailed++;
	gl_DisableVertexAttribArray = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glDisableVertexAttribArray");
	if(!gl_DisableVertexAttribArray) numFailed++;
	gl_DrawBuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLenum *))IntGetProcAddress("glDrawBuffers");
	if(!gl_DrawBuffers) numFailed++;
	gl_EnableVertexAttribArray = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glEnableVertexAttribArray");
	if(!gl_EnableVertexAttribArray) numFailed++;
	gl_GetActiveAttrib = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *))IntGetProcAddress("glGetActiveAttrib");
	if(!gl_GetActiveAttrib) numFailed++;
	gl_GetActiveUniform = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *))IntGetProcAddress("glGetActiveUniform");
	if(!gl_GetActiveUniform) numFailed++;
	gl_GetAttachedShaders = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLuint *))IntGetProcAddress("glGetAttachedShaders");
	if(!gl_GetAttachedShaders) numFailed++;
	gl_GetAttribLocation = (GLint (CODEGEN_FUNCPTR *)(GLuint, const GLchar *))IntGetProcAddress("glGetAttribLocation");
	if(!gl_GetAttribLocation) numFailed++;
	gl_GetProgramInfoLog = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetProgramInfoLog");
	if(!gl_GetProgramInfoLog) numFailed++;
	gl_GetProgramiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetProgramiv");
	if(!gl_GetProgramiv) numFailed++;
	gl_GetShaderInfoLog = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetShaderInfoLog");
	if(!gl_GetShaderInfoLog) numFailed++;
	gl_GetShaderSource = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetShaderSource");
	if(!gl_GetShaderSource) numFailed++;
	gl_GetShaderiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetShaderiv");
	if(!gl_GetShaderiv) numFailed++;
	gl_GetUniformLocation = (GLint (CODEGEN_FUNCPTR *)(GLuint, const GLchar *))IntGetProcAddress("glGetUniformLocation");
	if(!gl_GetUniformLocation) numFailed++;
	gl_GetUniformfv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLfloat *))IntGetProcAddress("glGetUniformfv");
	if(!gl_GetUniformfv) numFailed++;
	gl_GetUniformiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint *))IntGetProcAddress("glGetUniformiv");
	if(!gl_GetUniformiv) numFailed++;
	gl_GetVertexAttribPointerv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, void **))IntGetProcAddress("glGetVertexAttribPointerv");
	if(!gl_GetVertexAttribPointerv) numFailed++;
	gl_GetVertexAttribdv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLdouble *))IntGetProcAddress("glGetVertexAttribdv");
	if(!gl_GetVertexAttribdv) numFailed++;
	gl_GetVertexAttribfv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLfloat *))IntGetProcAddress("glGetVertexAttribfv");
	if(!gl_GetVertexAttribfv) numFailed++;
	gl_GetVertexAttribiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetVertexAttribiv");
	if(!gl_GetVertexAttribiv) numFailed++;
	gl_IsProgram = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsProgram");
	if(!gl_IsProgram) numFailed++;
	gl_IsShader = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsShader");
	if(!gl_IsShader) numFailed++;
	gl_LinkProgram = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glLinkProgram");
	if(!gl_LinkProgram) numFailed++;
	gl_ShaderSource = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLchar *const*, const GLint *))IntGetProcAddress("glShaderSource");
	if(!gl_ShaderSource) numFailed++;
	gl_StencilFuncSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint, GLuint))IntGetProcAddress("glStencilFuncSeparate");
	if(!gl_StencilFuncSeparate) numFailed++;
	gl_StencilMaskSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glStencilMaskSeparate");
	if(!gl_StencilMaskSeparate) numFailed++;
	gl_StencilOpSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLenum))IntGetProcAddress("glStencilOpSeparate");
	if(!gl_StencilOpSeparate) numFailed++;
	gl_Uniform1f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat))IntGetProcAddress("glUniform1f");
	if(!gl_Uniform1f) numFailed++;
	gl_Uniform1fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLfloat *))IntGetProcAddress("glUniform1fv");
	if(!gl_Uniform1fv) numFailed++;
	gl_Uniform1i = (void (CODEGEN_FUNCPTR *)(GLint, GLint))IntGetProcAddress("glUniform1i");
	if(!gl_Uniform1i) numFailed++;
	gl_Uniform1iv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLint *))IntGetProcAddress("glUniform1iv");
	if(!gl_Uniform1iv) numFailed++;
	gl_Uniform2f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat, GLfloat))IntGetProcAddress("glUniform2f");
	if(!gl_Uniform2f) numFailed++;
	gl_Uniform2fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLfloat *))IntGetProcAddress("glUniform2fv");
	if(!gl_Uniform2fv) numFailed++;
	gl_Uniform2i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint))IntGetProcAddress("glUniform2i");
	if(!gl_Uniform2i) numFailed++;
	gl_Uniform2iv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLint *))IntGetProcAddress("glUniform2iv");
	if(!gl_Uniform2iv) numFailed++;
	gl_Uniform3f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glUniform3f");
	if(!gl_Uniform3f) numFailed++;
	gl_Uniform3fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLfloat *))IntGetProcAddress("glUniform3fv");
	if(!gl_Uniform3fv) numFailed++;
	gl_Uniform3i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint))IntGetProcAddress("glUniform3i");
	if(!gl_Uniform3i) numFailed++;
	gl_Uniform3iv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLint *))IntGetProcAddress("glUniform3iv");
	if(!gl_Uniform3iv) numFailed++;
	gl_Uniform4f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glUniform4f");
	if(!gl_Uniform4f) numFailed++;
	gl_Uniform4fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLfloat *))IntGetProcAddress("glUniform4fv");
	if(!gl_Uniform4fv) numFailed++;
	gl_Uniform4i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint, GLint))IntGetProcAddress("glUniform4i");
	if(!gl_Uniform4i) numFailed++;
	gl_Uniform4iv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLint *))IntGetProcAddress("glUniform4iv");
	if(!gl_Uniform4iv) numFailed++;
	gl_UniformMatrix2fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix2fv");
	if(!gl_UniformMatrix2fv) numFailed++;
	gl_UniformMatrix3fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix3fv");
	if(!gl_UniformMatrix3fv) numFailed++;
	gl_UniformMatrix4fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix4fv");
	if(!gl_UniformMatrix4fv) numFailed++;
	gl_UseProgram = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glUseProgram");
	if(!gl_UseProgram) numFailed++;
	gl_ValidateProgram = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glValidateProgram");
	if(!gl_ValidateProgram) numFailed++;
	gl_VertexAttrib1d = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble))IntGetProcAddress("glVertexAttrib1d");
	if(!gl_VertexAttrib1d) numFailed++;
	gl_VertexAttrib1dv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLdouble *))IntGetProcAddress("glVertexAttrib1dv");
	if(!gl_VertexAttrib1dv) numFailed++;
	gl_VertexAttrib1f = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat))IntGetProcAddress("glVertexAttrib1f");
	if(!gl_VertexAttrib1f) numFailed++;
	gl_VertexAttrib1fv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glVertexAttrib1fv");
	if(!gl_VertexAttrib1fv) numFailed++;
	gl_VertexAttrib1s = (void (CODEGEN_FUNCPTR *)(GLuint, GLshort))IntGetProcAddress("glVertexAttrib1s");
	if(!gl_VertexAttrib1s) numFailed++;
	gl_VertexAttrib1sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib1sv");
	if(!gl_VertexAttrib1sv) numFailed++;
	gl_VertexAttrib2d = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble, GLdouble))IntGetProcAddress("glVertexAttrib2d");
	if(!gl_VertexAttrib2d) numFailed++;
	gl_VertexAttrib2dv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLdouble *))IntGetProcAddress("glVertexAttrib2dv");
	if(!gl_VertexAttrib2dv) numFailed++;
	gl_VertexAttrib2f = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat, GLfloat))IntGetProcAddress("glVertexAttrib2f");
	if(!gl_VertexAttrib2f) numFailed++;
	gl_VertexAttrib2fv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glVertexAttrib2fv");
	if(!gl_VertexAttrib2fv) numFailed++;
	gl_VertexAttrib2s = (void (CODEGEN_FUNCPTR *)(GLuint, GLshort, GLshort))IntGetProcAddress("glVertexAttrib2s");
	if(!gl_VertexAttrib2s) numFailed++;
	gl_VertexAttrib2sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib2sv");
	if(!gl_VertexAttrib2sv) numFailed++;
	gl_VertexAttrib3d = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glVertexAttrib3d");
	if(!gl_VertexAttrib3d) numFailed++;
	gl_VertexAttrib3dv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLdouble *))IntGetProcAddress("glVertexAttrib3dv");
	if(!gl_VertexAttrib3dv) numFailed++;
	gl_VertexAttrib3f = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glVertexAttrib3f");
	if(!gl_VertexAttrib3f) numFailed++;
	gl_VertexAttrib3fv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glVertexAttrib3fv");
	if(!gl_VertexAttrib3fv) numFailed++;
	gl_VertexAttrib3s = (void (CODEGEN_FUNCPTR *)(GLuint, GLshort, GLshort, GLshort))IntGetProcAddress("glVertexAttrib3s");
	if(!gl_VertexAttrib3s) numFailed++;
	gl_VertexAttrib3sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib3sv");
	if(!gl_VertexAttrib3sv) numFailed++;
	gl_VertexAttrib4Nbv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLbyte *))IntGetProcAddress("glVertexAttrib4Nbv");
	if(!gl_VertexAttrib4Nbv) numFailed++;
	gl_VertexAttrib4Niv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttrib4Niv");
	if(!gl_VertexAttrib4Niv) numFailed++;
	gl_VertexAttrib4Nsv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib4Nsv");
	if(!gl_VertexAttrib4Nsv) numFailed++;
	gl_VertexAttrib4Nub = (void (CODEGEN_FUNCPTR *)(GLuint, GLubyte, GLubyte, GLubyte, GLubyte))IntGetProcAddress("glVertexAttrib4Nub");
	if(!gl_VertexAttrib4Nub) numFailed++;
	gl_VertexAttrib4Nubv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLubyte *))IntGetProcAddress("glVertexAttrib4Nubv");
	if(!gl_VertexAttrib4Nubv) numFailed++;
	gl_VertexAttrib4Nuiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttrib4Nuiv");
	if(!gl_VertexAttrib4Nuiv) numFailed++;
	gl_VertexAttrib4Nusv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLushort *))IntGetProcAddress("glVertexAttrib4Nusv");
	if(!gl_VertexAttrib4Nusv) numFailed++;
	gl_VertexAttrib4bv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLbyte *))IntGetProcAddress("glVertexAttrib4bv");
	if(!gl_VertexAttrib4bv) numFailed++;
	gl_VertexAttrib4d = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glVertexAttrib4d");
	if(!gl_VertexAttrib4d) numFailed++;
	gl_VertexAttrib4dv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLdouble *))IntGetProcAddress("glVertexAttrib4dv");
	if(!gl_VertexAttrib4dv) numFailed++;
	gl_VertexAttrib4f = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glVertexAttrib4f");
	if(!gl_VertexAttrib4f) numFailed++;
	gl_VertexAttrib4fv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glVertexAttrib4fv");
	if(!gl_VertexAttrib4fv) numFailed++;
	gl_VertexAttrib4iv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttrib4iv");
	if(!gl_VertexAttrib4iv) numFailed++;
	gl_VertexAttrib4s = (void (CODEGEN_FUNCPTR *)(GLuint, GLshort, GLshort, GLshort, GLshort))IntGetProcAddress("glVertexAttrib4s");
	if(!gl_VertexAttrib4s) numFailed++;
	gl_VertexAttrib4sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib4sv");
	if(!gl_VertexAttrib4sv) numFailed++;
	gl_VertexAttrib4ubv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLubyte *))IntGetProcAddress("glVertexAttrib4ubv");
	if(!gl_VertexAttrib4ubv) numFailed++;
	gl_VertexAttrib4uiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttrib4uiv");
	if(!gl_VertexAttrib4uiv) numFailed++;
	gl_VertexAttrib4usv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLushort *))IntGetProcAddress("glVertexAttrib4usv");
	if(!gl_VertexAttrib4usv) numFailed++;
	gl_VertexAttribPointer = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *))IntGetProcAddress("glVertexAttribPointer");
	if(!gl_VertexAttribPointer) numFailed++;
	gl_UniformMatrix2x3fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix2x3fv");
	if(!gl_UniformMatrix2x3fv) numFailed++;
	gl_UniformMatrix2x4fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix2x4fv");
	if(!gl_UniformMatrix2x4fv) numFailed++;
	gl_UniformMatrix3x2fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix3x2fv");
	if(!gl_UniformMatrix3x2fv) numFailed++;
	gl_UniformMatrix3x4fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix3x4fv");
	if(!gl_UniformMatrix3x4fv) numFailed++;
	gl_UniformMatrix4x2fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix4x2fv");
	if(!gl_UniformMatrix4x2fv) numFailed++;
	gl_UniformMatrix4x3fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix4x3fv");
	if(!gl_UniformMatrix4x3fv) numFailed++;
	gl_BeginConditionalRender = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum))IntGetProcAddress("glBeginConditionalRender");
	if(!gl_BeginConditionalRender) numFailed++;
	gl_BeginTransformFeedback = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glBeginTransformFeedback");
	if(!gl_BeginTransformFeedback) numFailed++;
	gl_BindBufferBase = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLuint))IntGetProcAddress("glBindBufferBase");
	if(!gl_BindBufferBase) numFailed++;
	gl_BindBufferRange = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr))IntGetProcAddress("glBindBufferRange");
	if(!gl_BindBufferRange) numFailed++;
	gl_BindFragDataLocation = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, const GLchar *))IntGetProcAddress("glBindFragDataLocation");
	if(!gl_BindFragDataLocation) numFailed++;
	gl_BindFramebuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindFramebuffer");
	if(!gl_BindFramebuffer) numFailed++;
	gl_BindRenderbuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindRenderbuffer");
	if(!gl_BindRenderbuffer) numFailed++;
	gl_BindVertexArray = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glBindVertexArray");
	if(!gl_BindVertexArray) numFailed++;
	gl_BlitFramebuffer = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum))IntGetProcAddress("glBlitFramebuffer");
	if(!gl_BlitFramebuffer) numFailed++;
	gl_CheckFramebufferStatus = (GLenum (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glCheckFramebufferStatus");
	if(!gl_CheckFramebufferStatus) numFailed++;
	gl_ClampColor = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glClampColor");
	if(!gl_ClampColor) numFailed++;
	gl_ClearBufferfi = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLfloat, GLint))IntGetProcAddress("glClearBufferfi");
	if(!gl_ClearBufferfi) numFailed++;
	gl_ClearBufferfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, const GLfloat *))IntGetProcAddress("glClearBufferfv");
	if(!gl_ClearBufferfv) numFailed++;
	gl_ClearBufferiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, const GLint *))IntGetProcAddress("glClearBufferiv");
	if(!gl_ClearBufferiv) numFailed++;
	gl_ClearBufferuiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, const GLuint *))IntGetProcAddress("glClearBufferuiv");
	if(!gl_ClearBufferuiv) numFailed++;
	gl_ColorMaski = (void (CODEGEN_FUNCPTR *)(GLuint, GLboolean, GLboolean, GLboolean, GLboolean))IntGetProcAddress("glColorMaski");
	if(!gl_ColorMaski) numFailed++;
	gl_DeleteFramebuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteFramebuffers");
	if(!gl_DeleteFramebuffers) numFailed++;
	gl_DeleteRenderbuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteRenderbuffers");
	if(!gl_DeleteRenderbuffers) numFailed++;
	gl_DeleteVertexArrays = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteVertexArrays");
	if(!gl_DeleteVertexArrays) numFailed++;
	gl_Disablei = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glDisablei");
	if(!gl_Disablei) numFailed++;
	gl_Enablei = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glEnablei");
	if(!gl_Enablei) numFailed++;
	gl_EndConditionalRender = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glEndConditionalRender");
	if(!gl_EndConditionalRender) numFailed++;
	gl_EndTransformFeedback = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glEndTransformFeedback");
	if(!gl_EndTransformFeedback) numFailed++;
	gl_FlushMappedBufferRange = (void (CODEGEN_FUNCPTR *)(GLenum, GLintptr, GLsizeiptr))IntGetProcAddress("glFlushMappedBufferRange");
	if(!gl_FlushMappedBufferRange) numFailed++;
	gl_FramebufferRenderbuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint))IntGetProcAddress("glFramebufferRenderbuffer");
	if(!gl_FramebufferRenderbuffer) numFailed++;
	gl_FramebufferTexture1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint, GLint))IntGetProcAddress("glFramebufferTexture1D");
	if(!gl_FramebufferTexture1D) numFailed++;
	gl_FramebufferTexture2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint, GLint))IntGetProcAddress("glFramebufferTexture2D");
	if(!gl_FramebufferTexture2D) numFailed++;
	gl_FramebufferTexture3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint, GLint, GLint))IntGetProcAddress("glFramebufferTexture3D");
	if(!gl_FramebufferTexture3D) numFailed++;
	gl_FramebufferTextureLayer = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint, GLint, GLint))IntGetProcAddress("glFramebufferTextureLayer");
	if(!gl_FramebufferTextureLayer) numFailed++;
	gl_GenFramebuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenFramebuffers");
	if(!gl_GenFramebuffers) numFailed++;
	gl_GenRenderbuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenRenderbuffers");
	if(!gl_GenRenderbuffers) numFailed++;
	gl_GenVertexArrays = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenVertexArrays");
	if(!gl_GenVertexArrays) numFailed++;
	gl_GenerateMipmap = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glGenerateMipmap");
	if(!gl_GenerateMipmap) numFailed++;
	gl_GetBooleani_v = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLboolean *))IntGetProcAddress("glGetBooleani_v");
	if(!gl_GetBooleani_v) numFailed++;
	gl_GetFragDataLocation = (GLint (CODEGEN_FUNCPTR *)(GLuint, const GLchar *))IntGetProcAddress("glGetFragDataLocation");
	if(!gl_GetFragDataLocation) numFailed++;
	gl_GetFramebufferAttachmentParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLint *))IntGetProcAddress("glGetFramebufferAttachmentParameteriv");
	if(!gl_GetFramebufferAttachmentParameteriv) numFailed++;
	gl_GetIntegeri_v = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLint *))IntGetProcAddress("glGetIntegeri_v");
	if(!gl_GetIntegeri_v) numFailed++;
	gl_GetRenderbufferParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetRenderbufferParameteriv");
	if(!gl_GetRenderbufferParameteriv) numFailed++;
	gl_GetStringi = (const GLubyte * (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glGetStringi");
	if(!gl_GetStringi) numFailed++;
	gl_GetTexParameterIiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetTexParameterIiv");
	if(!gl_GetTexParameterIiv) numFailed++;
	gl_GetTexParameterIuiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint *))IntGetProcAddress("glGetTexParameterIuiv");
	if(!gl_GetTexParameterIuiv) numFailed++;
	gl_GetTransformFeedbackVarying = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLsizei, GLsizei *, GLsizei *, GLenum *, GLchar *))IntGetProcAddress("glGetTransformFeedbackVarying");
	if(!gl_GetTransformFeedbackVarying) numFailed++;
	gl_GetUniformuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLuint *))IntGetProcAddress("glGetUniformuiv");
	if(!gl_GetUniformuiv) numFailed++;
	gl_GetVertexAttribIiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetVertexAttribIiv");
	if(!gl_GetVertexAttribIiv) numFailed++;
	gl_GetVertexAttribIuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint *))IntGetProcAddress("glGetVertexAttribIuiv");
	if(!gl_GetVertexAttribIuiv) numFailed++;
	gl_IsEnabledi = (GLboolean (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glIsEnabledi");
	if(!gl_IsEnabledi) numFailed++;
	gl_IsFramebuffer = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsFramebuffer");
	if(!gl_IsFramebuffer) numFailed++;
	gl_IsRenderbuffer = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsRenderbuffer");
	if(!gl_IsRenderbuffer) numFailed++;
	gl_IsVertexArray = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsVertexArray");
	if(!gl_IsVertexArray) numFailed++;
	gl_MapBufferRange = (void * (CODEGEN_FUNCPTR *)(GLenum, GLintptr, GLsizeiptr, GLbitfield))IntGetProcAddress("glMapBufferRange");
	if(!gl_MapBufferRange) numFailed++;
	gl_RenderbufferStorage = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLsizei, GLsizei))IntGetProcAddress("glRenderbufferStorage");
	if(!gl_RenderbufferStorage) numFailed++;
	gl_RenderbufferStorageMultisample = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei))IntGetProcAddress("glRenderbufferStorageMultisample");
	if(!gl_RenderbufferStorageMultisample) numFailed++;
	gl_TexParameterIiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLint *))IntGetProcAddress("glTexParameterIiv");
	if(!gl_TexParameterIiv) numFailed++;
	gl_TexParameterIuiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLuint *))IntGetProcAddress("glTexParameterIuiv");
	if(!gl_TexParameterIuiv) numFailed++;
	gl_TransformFeedbackVaryings = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLchar *const*, GLenum))IntGetProcAddress("glTransformFeedbackVaryings");
	if(!gl_TransformFeedbackVaryings) numFailed++;
	gl_Uniform1ui = (void (CODEGEN_FUNCPTR *)(GLint, GLuint))IntGetProcAddress("glUniform1ui");
	if(!gl_Uniform1ui) numFailed++;
	gl_Uniform1uiv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLuint *))IntGetProcAddress("glUniform1uiv");
	if(!gl_Uniform1uiv) numFailed++;
	gl_Uniform2ui = (void (CODEGEN_FUNCPTR *)(GLint, GLuint, GLuint))IntGetProcAddress("glUniform2ui");
	if(!gl_Uniform2ui) numFailed++;
	gl_Uniform2uiv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLuint *))IntGetProcAddress("glUniform2uiv");
	if(!gl_Uniform2uiv) numFailed++;
	gl_Uniform3ui = (void (CODEGEN_FUNCPTR *)(GLint, GLuint, GLuint, GLuint))IntGetProcAddress("glUniform3ui");
	if(!gl_Uniform3ui) numFailed++;
	gl_Uniform3uiv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLuint *))IntGetProcAddress("glUniform3uiv");
	if(!gl_Uniform3uiv) numFailed++;
	gl_Uniform4ui = (void (CODEGEN_FUNCPTR *)(GLint, GLuint, GLuint, GLuint, GLuint))IntGetProcAddress("glUniform4ui");
	if(!gl_Uniform4ui) numFailed++;
	gl_Uniform4uiv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLuint *))IntGetProcAddress("glUniform4uiv");
	if(!gl_Uniform4uiv) numFailed++;
	gl_VertexAttribI1i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint))IntGetProcAddress("glVertexAttribI1i");
	if(!gl_VertexAttribI1i) numFailed++;
	gl_VertexAttribI1iv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttribI1iv");
	if(!gl_VertexAttribI1iv) numFailed++;
	gl_VertexAttribI1ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glVertexAttribI1ui");
	if(!gl_VertexAttribI1ui) numFailed++;
	gl_VertexAttribI1uiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttribI1uiv");
	if(!gl_VertexAttribI1uiv) numFailed++;
	gl_VertexAttribI2i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint))IntGetProcAddress("glVertexAttribI2i");
	if(!gl_VertexAttribI2i) numFailed++;
	gl_VertexAttribI2iv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttribI2iv");
	if(!gl_VertexAttribI2iv) numFailed++;
	gl_VertexAttribI2ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint))IntGetProcAddress("glVertexAttribI2ui");
	if(!gl_VertexAttribI2ui) numFailed++;
	gl_VertexAttribI2uiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttribI2uiv");
	if(!gl_VertexAttribI2uiv) numFailed++;
	gl_VertexAttribI3i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLint))IntGetProcAddress("glVertexAttribI3i");
	if(!gl_VertexAttribI3i) numFailed++;
	gl_VertexAttribI3iv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttribI3iv");
	if(!gl_VertexAttribI3iv) numFailed++;
	gl_VertexAttribI3ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint, GLuint))IntGetProcAddress("glVertexAttribI3ui");
	if(!gl_VertexAttribI3ui) numFailed++;
	gl_VertexAttribI3uiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttribI3uiv");
	if(!gl_VertexAttribI3uiv) numFailed++;
	gl_VertexAttribI4bv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLbyte *))IntGetProcAddress("glVertexAttribI4bv");
	if(!gl_VertexAttribI4bv) numFailed++;
	gl_VertexAttribI4i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLint, GLint))IntGetProcAddress("glVertexAttribI4i");
	if(!gl_VertexAttribI4i) numFailed++;
	gl_VertexAttribI4iv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttribI4iv");
	if(!gl_VertexAttribI4iv) numFailed++;
	gl_VertexAttribI4sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttribI4sv");
	if(!gl_VertexAttribI4sv) numFailed++;
	gl_VertexAttribI4ubv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLubyte *))IntGetProcAddress("glVertexAttribI4ubv");
	if(!gl_VertexAttribI4ubv) numFailed++;
	gl_VertexAttribI4ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint, GLuint, GLuint))IntGetProcAddress("glVertexAttribI4ui");
	if(!gl_VertexAttribI4ui) numFailed++;
	gl_VertexAttribI4uiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttribI4uiv");
	if(!gl_VertexAttribI4uiv) numFailed++;
	gl_VertexAttribI4usv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLushort *))IntGetProcAddress("glVertexAttribI4usv");
	if(!gl_VertexAttribI4usv) numFailed++;
	gl_VertexAttribIPointer = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLenum, GLsizei, const void *))IntGetProcAddress("glVertexAttribIPointer");
	if(!gl_VertexAttribIPointer) numFailed++;
	gl_CopyBufferSubData = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr))IntGetProcAddress("glCopyBufferSubData");
	if(!gl_CopyBufferSubData) numFailed++;
	gl_DrawArraysInstanced = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLsizei, GLsizei))IntGetProcAddress("glDrawArraysInstanced");
	if(!gl_DrawArraysInstanced) numFailed++;
	gl_DrawElementsInstanced = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, const void *, GLsizei))IntGetProcAddress("glDrawElementsInstanced");
	if(!gl_DrawElementsInstanced) numFailed++;
	gl_GetActiveUniformBlockName = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetActiveUniformBlockName");
	if(!gl_GetActiveUniformBlockName) numFailed++;
	gl_GetActiveUniformBlockiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLenum, GLint *))IntGetProcAddress("glGetActiveUniformBlockiv");
	if(!gl_GetActiveUniformBlockiv) numFailed++;
	gl_GetActiveUniformName = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetActiveUniformName");
	if(!gl_GetActiveUniformName) numFailed++;
	gl_GetActiveUniformsiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLuint *, GLenum, GLint *))IntGetProcAddress("glGetActiveUniformsiv");
	if(!gl_GetActiveUniformsiv) numFailed++;
	gl_GetUniformBlockIndex = (GLuint (CODEGEN_FUNCPTR *)(GLuint, const GLchar *))IntGetProcAddress("glGetUniformBlockIndex");
	if(!gl_GetUniformBlockIndex) numFailed++;
	gl_GetUniformIndices = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLchar *const*, GLuint *))IntGetProcAddress("glGetUniformIndices");
	if(!gl_GetUniformIndices) numFailed++;
	gl_PrimitiveRestartIndex = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glPrimitiveRestartIndex");
	if(!gl_PrimitiveRestartIndex) numFailed++;
	gl_TexBuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint))IntGetProcAddress("glTexBuffer");
	if(!gl_TexBuffer) numFailed++;
	gl_UniformBlockBinding = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint))IntGetProcAddress("glUniformBlockBinding");
	if(!gl_UniformBlockBinding) numFailed++;
	gl_ClientWaitSync = (GLenum (CODEGEN_FUNCPTR *)(GLsync, GLbitfield, GLuint64))IntGetProcAddress("glClientWaitSync");
	if(!gl_ClientWaitSync) numFailed++;
	gl_DeleteSync = (void (CODEGEN_FUNCPTR *)(GLsync))IntGetProcAddress("glDeleteSync");
	if(!gl_DeleteSync) numFailed++;
	gl_DrawElementsBaseVertex = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, const void *, GLint))IntGetProcAddress("glDrawElementsBaseVertex");
	if(!gl_DrawElementsBaseVertex) numFailed++;
	gl_DrawElementsInstancedBaseVertex = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, const void *, GLsizei, GLint))IntGetProcAddress("glDrawElementsInstancedBaseVertex");
	if(!gl_DrawElementsInstancedBaseVertex) numFailed++;
	gl_DrawRangeElementsBaseVertex = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLuint, GLsizei, GLenum, const void *, GLint))IntGetProcAddress("glDrawRangeElementsBaseVertex");
	if(!gl_DrawRangeElementsBaseVertex) numFailed++;
	gl_FenceSync = (GLsync (CODEGEN_FUNCPTR *)(GLenum, GLbitfield))IntGetProcAddress("glFenceSync");
	if(!gl_FenceSync) numFailed++;
	gl_FramebufferTexture = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint, GLint))IntGetProcAddress("glFramebufferTexture");
	if(!gl_FramebufferTexture) numFailed++;
	gl_GetBufferParameteri64v = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint64 *))IntGetProcAddress("glGetBufferParameteri64v");
	if(!gl_GetBufferParameteri64v) numFailed++;
	gl_GetInteger64i_v = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLint64 *))IntGetProcAddress("glGetInteger64i_v");
	if(!gl_GetInteger64i_v) numFailed++;
	gl_GetInteger64v = (void (CODEGEN_FUNCPTR *)(GLenum, GLint64 *))IntGetProcAddress("glGetInteger64v");
	if(!gl_GetInteger64v) numFailed++;
	gl_GetMultisamplefv = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLfloat *))IntGetProcAddress("glGetMultisamplefv");
	if(!gl_GetMultisamplefv) numFailed++;
	gl_GetSynciv = (void (CODEGEN_FUNCPTR *)(GLsync, GLenum, GLsizei, GLsizei *, GLint *))IntGetProcAddress("glGetSynciv");
	if(!gl_GetSynciv) numFailed++;
	gl_IsSync = (GLboolean (CODEGEN_FUNCPTR *)(GLsync))IntGetProcAddress("glIsSync");
	if(!gl_IsSync) numFailed++;
	gl_MultiDrawElementsBaseVertex = (void (CODEGEN_FUNCPTR *)(GLenum, const GLsizei *, GLenum, const void *const*, GLsizei, const GLint *))IntGetProcAddress("glMultiDrawElementsBaseVertex");
	if(!gl_MultiDrawElementsBaseVertex) numFailed++;
	gl_ProvokingVertex = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glProvokingVertex");
	if(!gl_ProvokingVertex) numFailed++;
	gl_SampleMaski = (void (CODEGEN_FUNCPTR *)(GLuint, GLbitfield))IntGetProcAddress("glSampleMaski");
	if(!gl_SampleMaski) numFailed++;
	gl_TexImage2DMultisample = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean))IntGetProcAddress("glTexImage2DMultisample");
	if(!gl_TexImage2DMultisample) numFailed++;
	gl_TexImage3DMultisample = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean))IntGetProcAddress("glTexImage3DMultisample");
	if(!gl_TexImage3DMultisample) numFailed++;
	gl_WaitSync = (void (CODEGEN_FUNCPTR *)(GLsync, GLbitfield, GLuint64))IntGetProcAddress("glWaitSync");
	if(!gl_WaitSync) numFailed++;
	gl_BindFragDataLocationIndexed = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint, const GLchar *))IntGetProcAddress("glBindFragDataLocationIndexed");
	if(!gl_BindFragDataLocationIndexed) numFailed++;
	gl_BindSampler = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glBindSampler");
	if(!gl_BindSampler) numFailed++;
	gl_DeleteSamplers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteSamplers");
	if(!gl_DeleteSamplers) numFailed++;
	gl_GenSamplers = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenSamplers");
	if(!gl_GenSamplers) numFailed++;
	gl_GetFragDataIndex = (GLint (CODEGEN_FUNCPTR *)(GLuint, const GLchar *))IntGetProcAddress("glGetFragDataIndex");
	if(!gl_GetFragDataIndex) numFailed++;
	gl_GetQueryObjecti64v = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint64 *))IntGetProcAddress("glGetQueryObjecti64v");
	if(!gl_GetQueryObjecti64v) numFailed++;
	gl_GetQueryObjectui64v = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint64 *))IntGetProcAddress("glGetQueryObjectui64v");
	if(!gl_GetQueryObjectui64v) numFailed++;
	gl_GetSamplerParameterIiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetSamplerParameterIiv");
	if(!gl_GetSamplerParameterIiv) numFailed++;
	gl_GetSamplerParameterIuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint *))IntGetProcAddress("glGetSamplerParameterIuiv");
	if(!gl_GetSamplerParameterIuiv) numFailed++;
	gl_GetSamplerParameterfv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLfloat *))IntGetProcAddress("glGetSamplerParameterfv");
	if(!gl_GetSamplerParameterfv) numFailed++;
	gl_GetSamplerParameteriv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetSamplerParameteriv");
	if(!gl_GetSamplerParameteriv) numFailed++;
	gl_IsSampler = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsSampler");
	if(!gl_IsSampler) numFailed++;
	gl_QueryCounter = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum))IntGetProcAddress("glQueryCounter");
	if(!gl_QueryCounter) numFailed++;
	gl_SamplerParameterIiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLint *))IntGetProcAddress("glSamplerParameterIiv");
	if(!gl_SamplerParameterIiv) numFailed++;
	gl_SamplerParameterIuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLuint *))IntGetProcAddress("glSamplerParameterIuiv");
	if(!gl_SamplerParameterIuiv) numFailed++;
	gl_SamplerParameterf = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLfloat))IntGetProcAddress("glSamplerParameterf");
	if(!gl_SamplerParameterf) numFailed++;
	gl_SamplerParameterfv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLfloat *))IntGetProcAddress("glSamplerParameterfv");
	if(!gl_SamplerParameterfv) numFailed++;
	gl_SamplerParameteri = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint))IntGetProcAddress("glSamplerParameteri");
	if(!gl_SamplerParameteri) numFailed++;
	gl_SamplerParameteriv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLint *))IntGetProcAddress("glSamplerParameteriv");
	if(!gl_SamplerParameteriv) numFailed++;
	gl_VertexAttribDivisor = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glVertexAttribDivisor");
	if(!gl_VertexAttribDivisor) numFailed++;
	gl_VertexAttribP1ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, GLuint))IntGetProcAddress("glVertexAttribP1ui");
	if(!gl_VertexAttribP1ui) numFailed++;
	gl_VertexAttribP1uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, const GLuint *))IntGetProcAddress("glVertexAttribP1uiv");
	if(!gl_VertexAttribP1uiv) numFailed++;
	gl_VertexAttribP2ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, GLuint))IntGetProcAddress("glVertexAttribP2ui");
	if(!gl_VertexAttribP2ui) numFailed++;
	gl_VertexAttribP2uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, const GLuint *))IntGetProcAddress("glVertexAttribP2uiv");
	if(!gl_VertexAttribP2uiv) numFailed++;
	gl_VertexAttribP3ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, GLuint))IntGetProcAddress("glVertexAttribP3ui");
	if(!gl_VertexAttribP3ui) numFailed++;
	gl_VertexAttribP3uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, const GLuint *))IntGetProcAddress("glVertexAttribP3uiv");
	if(!gl_VertexAttribP3uiv) numFailed++;
	gl_VertexAttribP4ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, GLuint))IntGetProcAddress("glVertexAttribP4ui");
	if(!gl_VertexAttribP4ui) numFailed++;
	gl_VertexAttribP4uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, const GLuint *))IntGetProcAddress("glVertexAttribP4uiv");
	if(!gl_VertexAttribP4uiv) numFailed++;
	return numFailed;
}

typedef int (*PFN_LOADFUNCPOINTERS)();
typedef struct ogl_StrToExtMap_s
{
	char *extensionName;
	int *extensionVariable;
	PFN_LOADFUNCPOINTERS LoadExtension;
} ogl_StrToExtMap;

static ogl_StrToExtMap ExtensionMap[32] = {
	{"GL_EXT_texture_compression_s3tc", &ogl_ext_EXT_texture_compression_s3tc, NULL},
	{"GL_EXT_texture_sRGB", &ogl_ext_EXT_texture_sRGB, NULL},
	{"GL_EXT_texture_filter_anisotropic", &ogl_ext_EXT_texture_filter_anisotropic, NULL},
	{"GL_ARB_compressed_texture_pixel_storage", &ogl_ext_ARB_compressed_texture_pixel_storage, NULL},
	{"GL_ARB_conservative_depth", &ogl_ext_ARB_conservative_depth, NULL},
	{"GL_ARB_ES2_compatibility", &ogl_ext_ARB_ES2_compatibility, Load_ARB_ES2_compatibility},
	{"GL_ARB_get_program_binary", &ogl_ext_ARB_get_program_binary, Load_ARB_get_program_binary},
	{"GL_ARB_explicit_uniform_location", &ogl_ext_ARB_explicit_uniform_location, NULL},
	{"GL_ARB_internalformat_query", &ogl_ext_ARB_internalformat_query, Load_ARB_internalformat_query},
	{"GL_ARB_internalformat_query2", &ogl_ext_ARB_internalformat_query2, Load_ARB_internalformat_query2},
	{"GL_ARB_map_buffer_alignment", &ogl_ext_ARB_map_buffer_alignment, NULL},
	{"GL_ARB_program_interface_query", &ogl_ext_ARB_program_interface_query, Load_ARB_program_interface_query},
	{"GL_ARB_separate_shader_objects", &ogl_ext_ARB_separate_shader_objects, Load_ARB_separate_shader_objects},
	{"GL_ARB_shading_language_420pack", &ogl_ext_ARB_shading_language_420pack, NULL},
	{"GL_ARB_shading_language_packing", &ogl_ext_ARB_shading_language_packing, NULL},
	{"GL_ARB_texture_buffer_range", &ogl_ext_ARB_texture_buffer_range, Load_ARB_texture_buffer_range},
	{"GL_ARB_texture_storage", &ogl_ext_ARB_texture_storage, Load_ARB_texture_storage},
	{"GL_ARB_texture_view", &ogl_ext_ARB_texture_view, Load_ARB_texture_view},
	{"GL_ARB_vertex_attrib_binding", &ogl_ext_ARB_vertex_attrib_binding, Load_ARB_vertex_attrib_binding},
	{"GL_ARB_viewport_array", &ogl_ext_ARB_viewport_array, Load_ARB_viewport_array},
	{"GL_ARB_arrays_of_arrays", &ogl_ext_ARB_arrays_of_arrays, NULL},
	{"GL_ARB_clear_buffer_object", &ogl_ext_ARB_clear_buffer_object, Load_ARB_clear_buffer_object},
	{"GL_ARB_copy_image", &ogl_ext_ARB_copy_image, Load_ARB_copy_image},
	{"GL_ARB_ES3_compatibility", &ogl_ext_ARB_ES3_compatibility, NULL},
	{"GL_ARB_fragment_layer_viewport", &ogl_ext_ARB_fragment_layer_viewport, NULL},
	{"GL_ARB_framebuffer_no_attachments", &ogl_ext_ARB_framebuffer_no_attachments, Load_ARB_framebuffer_no_attachments},
	{"GL_ARB_invalidate_subdata", &ogl_ext_ARB_invalidate_subdata, Load_ARB_invalidate_subdata},
	{"GL_ARB_robust_buffer_access_behavior", &ogl_ext_ARB_robust_buffer_access_behavior, NULL},
	{"GL_ARB_stencil_texturing", &ogl_ext_ARB_stencil_texturing, NULL},
	{"GL_ARB_texture_query_levels", &ogl_ext_ARB_texture_query_levels, NULL},
	{"GL_ARB_texture_storage_multisample", &ogl_ext_ARB_texture_storage_multisample, Load_ARB_texture_storage_multisample},
	{"GL_KHR_debug", &ogl_ext_KHR_debug, Load_KHR_debug},
};

static int g_extensionMapSize = 32;

static ogl_StrToExtMap *FindExtEntry(const char *extensionName)
{
	int loop;
	ogl_StrToExtMap *currLoc = ExtensionMap;
	for(loop = 0; loop < g_extensionMapSize; ++loop, ++currLoc)
	{
		if(strcmp(extensionName, currLoc->extensionName) == 0)
			return currLoc;
	}
	
	return NULL;
}

static void ClearExtensionVars()
{
	ogl_ext_EXT_texture_compression_s3tc = ogl_LOAD_FAILED;
	ogl_ext_EXT_texture_sRGB = ogl_LOAD_FAILED;
	ogl_ext_EXT_texture_filter_anisotropic = ogl_LOAD_FAILED;
	ogl_ext_ARB_compressed_texture_pixel_storage = ogl_LOAD_FAILED;
	ogl_ext_ARB_conservative_depth = ogl_LOAD_FAILED;
	ogl_ext_ARB_ES2_compatibility = ogl_LOAD_FAILED;
	ogl_ext_ARB_get_program_binary = ogl_LOAD_FAILED;
	ogl_ext_ARB_explicit_uniform_location = ogl_LOAD_FAILED;
	ogl_ext_ARB_internalformat_query = ogl_LOAD_FAILED;
	ogl_ext_ARB_internalformat_query2 = ogl_LOAD_FAILED;
	ogl_ext_ARB_map_buffer_alignment = ogl_LOAD_FAILED;
	ogl_ext_ARB_program_interface_query = ogl_LOAD_FAILED;
	ogl_ext_ARB_separate_shader_objects = ogl_LOAD_FAILED;
	ogl_ext_ARB_shading_language_420pack = ogl_LOAD_FAILED;
	ogl_ext_ARB_shading_language_packing = ogl_LOAD_FAILED;
	ogl_ext_ARB_texture_buffer_range = ogl_LOAD_FAILED;
	ogl_ext_ARB_texture_storage = ogl_LOAD_FAILED;
	ogl_ext_ARB_texture_view = ogl_LOAD_FAILED;
	ogl_ext_ARB_vertex_attrib_binding = ogl_LOAD_FAILED;
	ogl_ext_ARB_viewport_array = ogl_LOAD_FAILED;
	ogl_ext_ARB_arrays_of_arrays = ogl_LOAD_FAILED;
	ogl_ext_ARB_clear_buffer_object = ogl_LOAD_FAILED;
	ogl_ext_ARB_copy_image = ogl_LOAD_FAILED;
	ogl_ext_ARB_ES3_compatibility = ogl_LOAD_FAILED;
	ogl_ext_ARB_fragment_layer_viewport = ogl_LOAD_FAILED;
	ogl_ext_ARB_framebuffer_no_attachments = ogl_LOAD_FAILED;
	ogl_ext_ARB_invalidate_subdata = ogl_LOAD_FAILED;
	ogl_ext_ARB_robust_buffer_access_behavior = ogl_LOAD_FAILED;
	ogl_ext_ARB_stencil_texturing = ogl_LOAD_FAILED;
	ogl_ext_ARB_texture_query_levels = ogl_LOAD_FAILED;
	ogl_ext_ARB_texture_storage_multisample = ogl_LOAD_FAILED;
	ogl_ext_KHR_debug = ogl_LOAD_FAILED;
}


static void LoadExtByName(const char *extensionName)
{
	ogl_StrToExtMap *entry = NULL;
	entry = FindExtEntry(extensionName);
	if(entry)
	{
		if(entry->LoadExtension)
		{
			int numFailed = entry->LoadExtension();
			if(numFailed == 0)
			{
				*(entry->extensionVariable) = ogl_LOAD_SUCCEEDED;
			}
			else
			{
				*(entry->extensionVariable) = ogl_LOAD_SUCCEEDED + numFailed;
			}
		}
		else
		{
			*(entry->extensionVariable) = ogl_LOAD_SUCCEEDED;
		}
	}
}


static void ProcExtsFromExtList()
{
	GLint iLoop;
	GLint iNumExtensions = 0;
	gl_GetIntegerv(GL_NUM_EXTENSIONS, &iNumExtensions);

	for(iLoop = 0; iLoop < iNumExtensions; iLoop++)
	{
		const char *strExtensionName = (const char *)gl_GetStringi(GL_EXTENSIONS, iLoop);
		LoadExtByName(strExtensionName);
	}
}

int ogl_LoadFunctions()
{
	int numFailed = 0;
	ClearExtensionVars();
	
	gl_GetIntegerv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint *))IntGetProcAddress("glGetIntegerv");
	if(!gl_GetIntegerv) return ogl_LOAD_FAILED;
	gl_GetStringi = (const GLubyte * (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glGetStringi");
	if(!gl_GetStringi) return ogl_LOAD_FAILED;
	
	ProcExtsFromExtList();
	numFailed = Load_Version_3_3();
	
	if(numFailed == 0)
		return ogl_LOAD_SUCCEEDED;
	else
		return ogl_LOAD_SUCCEEDED + numFailed;
}

static int g_major_version = 0;
static int g_minor_version = 0;

static void GetGLVersion()
{
	glGetIntegerv(GL_MAJOR_VERSION, &g_major_version);
	glGetIntegerv(GL_MINOR_VERSION, &g_minor_version);
}

int ogl_GetMajorVersion()
{
	if(g_major_version == 0)
		GetGLVersion();
	return g_major_version;
}

int ogl_GetMinorVersion()
{
	if(g_major_version == 0) //Yes, check the major version to get the minor one.
		GetGLVersion();
	return g_minor_version;
}

int ogl_IsVersionGEQ(int majorVersion, int minorVersion)
{
	if(g_major_version == 0)
		GetGLVersion();
		
	if(majorVersion > g_major_version) return 1;
	if(majorVersion < g_major_version) return 0;
	if(minorVersion >= g_minor_version) return 1;
	return 0;
}

