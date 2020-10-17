#include "CCFileReader.h"

CCFileReader::CCFileReader(std::wstring path)
{
	_wfopen_s(&file, path.c_str(), L"r");
	if (file) {
		fseek(file, 0, SEEK_END);
		len = ftell(file);
		fseek(file, 0, SEEK_SET);
	}
}
CCFileReader::~CCFileReader()
{
	Close();
}

bool CCFileReader::Opened()
{
	return file != nullptr;
}
void CCFileReader::Close()
{
	if (file) {
		fclose(file);
		file = nullptr;
	}
}
size_t CCFileReader::Length()
{
	return len;
}
void CCFileReader::Seek(size_t i, int seekType)
{
	fseek(file, i, seekType);
}
FILE* CCFileReader::Handle()
{
	return file;
}

void CCFileReader::Read(BYTE* arr, size_t offset, size_t count)
{
	fread_s(arr, count, 1, count, file);
}
BYTE CCFileReader::ReadByte()
{
	BYTE buf;
	fread_s(&buf, 1, 1, 1, file);
	return buf;
}

