#pragma once
#include "stdafx.h"

EXTERN_C void VR720_EXP AppSetUnhandledExceptionFilter();
EXTERN_C void VR720_EXP AppCreate(void**ptr);
EXTERN_C void VR720_EXP AppFree(void* ptr);