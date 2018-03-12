//
//    Many useful helper functions for GLSL shaders - gleaned from various sources including orange book
//    Created by zwood on 2/21/10.
//    Modified by sueda 10/15/15.
//

#pragma once
#ifndef __GLSL__
#define __GLSL__

#include <glad/glad.h>

///////////////////////////////////////////////////////////////////////////////
// For printing out the current file and line number                         //
///////////////////////////////////////////////////////////////////////////////
#include <sstream>

template <typename T>
std::string NumberToString(T x)
{
	std::ostringstream ss;
	ss << x;
	return ss.str();
}

#define GET_FILE_LINE (std::string(__FILE__) + ":" + NumberToString(__LINE__)).c_str()
///////////////////////////////////////////////////////////////////////////////

namespace GLSL {

    void printOpenGLErrors(char const * const Function, char const * const File, int const Line);
	void printProgramInfoLog(GLuint program);
	void printShaderInfoLog(GLuint shader);
	void checkVersion();
	int textFileWrite(const char *filename, char *s);
	char *textFileRead(const char *filename);
    GLuint createShader(std::string name, GLenum type);
}

#ifdef OPENGL_ERROR_CHECKS
#define CHECK_GL_CALL(x) do { GLSL::printOpenGLErrors("{{BEFORE}} "#x, __FILE__, __LINE__); (x); GLSL::printOpenGLErrors(#x, __FILE__, __LINE__); } while (0)
#else
#define CHECK_GL_CALL(x) (x)
#endif

#endif
