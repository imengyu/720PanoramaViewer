#pragma once
#include "CImageLoader.h"
#include <jconfig.h>
#include <jpeglib.h>

class CJpgLoader : public CImageLoader
{
	glm::vec2 GetImageSize() override;
	BYTE* GetAllImageData() override;
	BYTE* GetImageChunkData(int x, int y, int chunkW, int chunkH) override;

	bool Load(const wchar_t* path) override;
	const wchar_t* GetPath() override;
	void Destroy() override;
	bool IsOpened() override;
	USHORT GetImageDepth() override;

	std::wstring path;
	ULONG width = 0;
	ULONG height = 0;
	USHORT depth = 0;
	bool decodeSuccess = false;
};

