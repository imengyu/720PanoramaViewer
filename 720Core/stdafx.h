#pragma once

#ifdef VR720_EXPORTS
#define VR720_EXP __declspec(dllexport)

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
#include <intrin.h>
#include <tchar.h>

// C 运行时头文件
#include <stdlib.h>
#include <stdio.h>
//Cpp
#include <string>

#include <glm.hpp>
#include <ext.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/constants.hpp>

//OpenGL includes

#include "messages.h"
#include "Logger.h"   

//API Export
#else
#define VR720_EXP __declspec(dllimport)

#endif

#define GLEW_STATIC
#include <gl/glew.h>
#include <gl/wglew.h>

#define VR720_C_API(z) EXTERN_C z VR720_EXP

