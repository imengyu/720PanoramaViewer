#pragma once
#include "CCMeshLoader.h"

//Obj ������
class CCObjLoader :  public CCMeshLoader
{
	bool Load(const wchar_t* path, CCMesh* mesh) override;
};

