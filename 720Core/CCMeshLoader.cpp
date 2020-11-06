#include "CCMeshLoader.h"
#include "CCMesh.h"
#include "CCObjLoader.h"
#include "PathHelper.h"

CCObjLoader* objLoader = nullptr;

CCMeshLoader* CCMeshLoader::GetMeshLoaderByType(CCMeshType type)
{
	switch (type)
	{
	case CCMeshType::MeshTypeObj:
		return objLoader;
	case CCMeshType::MeshTypeFbx:
		break;
	}
	return nullptr;
}
CCMeshLoader* CCMeshLoader::GetMeshLoaderByFilePath(const wchar_t* path) {
	std::wstring ext = Path::GetExtension(path);
	if (ext == L"obj")
		return objLoader;
	return nullptr;
}

void CCMeshLoader::Init()
{
	objLoader = new CCObjLoader();
}
void CCMeshLoader::Destroy()
{
	delete objLoader;
}
bool CCMeshLoader::Load(const wchar_t* path, CCMesh* mesh)
{
	return false;
}
bool CCMeshLoader::Load(BYTE* buffer, size_t bufferSize, CCMesh* mesh) {
	return false;
}
const wchar_t* CCMeshLoader::GetLastError()
{
	return lastErr.c_str();
}
void CCMeshLoader::SetLastError(const wchar_t* err)
{
	lastErr = err;
}


