#include "CImageLoader.h"
#include "CBMPLoader.h"
#include "CJpgLoader.h"
#include "CPngLoader.h"
#include "StringHlp.h"

const BYTE pngHead[8] = { 0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a };
const BYTE bmpHead[2] = { 0x42, 0x4d };
const BYTE jpgHead[3] = { 0xff,0xd8,0xff };

ImageType CImageLoader::CheckImageType(const wchar_t* path)
{
	FILE* file;
	_wfopen_s(&file, path, L"rb");
	if (file) {
		BYTE buffer[8];
		fread(buffer, 1, 8, file);
		fclose(file);

		if (memcmp(buffer, pngHead, 8) == 0)
			return ImageType::PNG;
		if (memcmp(buffer, jpgHead, 3) == 0)
			return ImageType::JPG;
		if (memcmp(buffer, bmpHead, 2) == 0)
			return ImageType::BMP;
	}
	return ImageType::Unknow;
}

CImageLoader* CImageLoader::CreateImageLoaderType(ImageType type)
{
	switch (type)
	{
	case BMP:
		return new CBMPLoader();
	case JPG:
		return new CJpgLoader();
	case PNG:
		return new CPngLoader();
	}
	return nullptr;
}

glm::vec2 CImageLoader::GetImageSize()
{
	return glm::vec2();
}

BYTE* CImageLoader::GetAllImageData()
{
	return nullptr;
}

USHORT CImageLoader::GetImageDepth()
{
	return 0;
}

BYTE* CImageLoader::GetImageChunkData(int x, int y, int chunkW, int chunkH)
{
	return nullptr;
}

const wchar_t* CImageLoader::GetLastError()
{
	return lastError.c_str();;
}

 unsigned long CImageLoader::GetFullDataSize()
{
	return fullDataSize;
}
 unsigned long CImageLoader::GetChunkDataSize()
 {
	 return chunkDataSize;
 }

const wchar_t* CImageLoader::GetPath()
{
	return nullptr;
}

bool CImageLoader::Load(const wchar_t* path)
{
	return false;
}

void CImageLoader::Destroy()
{
}

bool CImageLoader::IsOpened()
{
	return false;
}

float CImageLoader::GetLoadingPrecent()
{
	return loadingPrecent;
}

void CImageLoader::SetLoadingPrecent(float v)
{
	loadingPrecent = v;
}


void CImageLoader::SetLastError(const wchar_t* err)
{
	lastError = err;
}

void CImageLoader::SetLastError(const char* err)
{
	wchar_t* e = StringHlp::AnsiToUnicode(err);
	SetLastError(e);
	StringHlp::FreeStringPtr(e);
}

void CImageLoader::SetFullDataSize(unsigned long size)
{
	fullDataSize = size;
}
void CImageLoader::SetChunkDataSize(unsigned long size)
{
	chunkDataSize = size;
}
