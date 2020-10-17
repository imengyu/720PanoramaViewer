#pragma once
#include "stdafx.h"

class CCFileReader
{
public:
	CCFileReader(std::wstring path);
	~CCFileReader();

	bool Opened();
	void Close();

	size_t Length();
	void Seek(size_t i, int seekType);
	FILE* Handle();

    void Read(BYTE* arr, size_t offset, size_t count);
	BYTE ReadByte();

private:

	FILE* file = nullptr;
	size_t len = 0;

};

