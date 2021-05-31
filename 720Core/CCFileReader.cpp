#include "CCFileReader.h"
#include "720Core.h"

CCFileReader::CCFileReader() = default;
CCFileReader::CCFileReader(std::wstring & path)
{
#if defined(_MSC_VER) && _MSC_VER > 1600
	_wfopen_s(&file, path.c_str(), L"r");
#else
	file = _wfopen(path.c_str(), L"r");
#endif
	if (file) {
		fseek(file, 0, SEEK_END);
		len = ftell(file);
		fseek(file, 0, SEEK_SET);
	}
}
CCFileReader::~CCFileReader()
{
	CloseFileHandle();
}

bool CCFileReader::Opened()
{
	return file != nullptr;
}
void CCFileReader::Close()
{
	CloseFileHandle();
}
size_t CCFileReader::Length()
{
	return len;
}
void CCFileReader::Seek(size_t i, int seekType)
{
	fseek(file, i, seekType);
}
void CCFileReader::Seek(size_t i) {
	fseek(file, i, SEEK_SET);
}
FILE* CCFileReader::Handle()
{
	return file;
}

void CCFileReader::Read(BYTE * arr, size_t offset, size_t count)
{
#if defined(_MSC_VER) && _MSC_VER > 1600
	fread_s(arr, count, 1, count, file);
#else
	fread(arr, 1, count, file);
#endif
}
BYTE CCFileReader::ReadByte()
{
	BYTE buf;
	Read(&buf, 0, 1);
	return buf;
}
BYTE* CCFileReader::ReadAllByte(size_t * size) {
	BYTE* buffer = (BYTE*)malloc(len);
	fseek(file, 0, SEEK_SET);
	Read(buffer, 0, len);
	return buffer;
}

void CCFileReader::CloseFileHandle() {
	if (file) {
		fclose(file);
		file = nullptr;
	}
}



