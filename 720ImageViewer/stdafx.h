#pragma once
#ifndef STDAFX_H
#define STDAFX_H
#include "config.h"
#include "platform.h"

#if defined(VR720_WINDOWS)

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ�ļ����ų�����ʹ�õ�����
// Windows ͷ�ļ�
#include <windows.h>
#include <intrin.h>
#include <tchar.h>

#elif defined(VR720_ANDROID)

//android ndk support
#include <jni.h>
#include <sys/types.h>
#include <unistd.h>
#include "type-defines.h"

//android log
#include <android/log.h>

#endif

// C ����ʱͷ�ļ�
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
//Cpp
#include <string>
#include <memory>
#include "Logger.h"

#include <glm.hpp>
#include <ext.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/constants.hpp>

//OpenGL includes

#if defined(VR720_WINDOWS)

#define GLEW_STATIC
#include <gl/glew.h>
#include <gl/wglew.h>

#include "messages.h"

#elif defined(VR720_ANDROID) //VR720_WINDOWS

#include <GLES2/gl2.h>
#include <GLES/gl.h>

#endif //VR720_ANDROID

#include "api-defines.h"

#endif //STDAFX_H
