#pragma once
#include "stdafx.h"
#include "CGdiPlusUtils.h"

class VR720_EXP CCThumbnailGenerator
{
public:
	static wchar_t* GetImageThumbnailAuto(const wchar_t* filePath);
	static Bitmap* GetImageThumbnail(const wchar_t* filePath, int w, int h);
};

