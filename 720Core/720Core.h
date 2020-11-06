#pragma once
#include "stdafx.h"

EXTERN_C void VR720_EXP SetAppUnhandledExceptionFilter();
EXTERN_C void VR720_EXP SetAppCreate(void**ptr);
EXTERN_C void VR720_EXP SetAppFree(void* ptr);

