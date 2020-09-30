#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>

// C 运行时头文件
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
//Cpp
#include <string>
#include <intrin.h>
// 在此处引用程序需要的其他标头


#include <glm.hpp>
#include <ext.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/constants.hpp>
#define GLEW_STATIC
#include <gl/glew.h>
#include <gl/wglew.h>

#include "Logger.h"
#include "messages.h"
