//
// Created by roger on 2020/10/29.
//

#include "CCAssetsManager.h"
#include "CCFileReader.h"
#include "CCTexture.h"
#include "CCMesh.h"
#include "CCMeshLoader.h"
#include "CApp.h"

const std::wstring CCAssetsManager::GetResourcePath(const wchar_t* typeName, const wchar_t* name)
{
    std::wstring str(AppGetAppInstance()->GetCurrentDir());
    str += L"\\resources\\";
    str += typeName;
    str += L"\\";
    str += name;
    return str;
}
const std::wstring CCAssetsManager::GetDirResourcePath(const wchar_t* dirName, const wchar_t* name)
{
    std::wstring str(AppGetAppInstance()->GetCurrentDir());
    str += L"\\";
    str += dirName;
    str += L"\\";
    str += name;
    return str;
}
std::string CCAssetsManager::GetResourcePath(const char* typeName, const char* name)
{
    std::string str(AppGetAppInstance()->GetCurrentDirA());
    str += "\\resources\\";
    str += typeName;
    str += "\\";
    str += name;
    return str;
}
std::string CCAssetsManager::GetDirResourcePath(const char* dirName, const char* name)
{
    std::string str(AppGetAppInstance()->GetCurrentDirA());
    str += "\\";
    str += dirName;
    str += "\\";
    str += name;
    return str;
}
BYTE* CCAssetsManager::LoadResource(const wchar_t* path, size_t* bufferLength) {
    std::wstring strpath(path);
    CCFileReader reader(strpath);
    if (reader.Opened())
        return reader.ReadAllByte(bufferLength);
    return nullptr;
}
CCTexture* CCAssetsManager::LoadTexture(const wchar_t* path) {
    auto* tex = new CCTexture();
    tex->Load(path);
    return tex;
}
CCMesh* CCAssetsManager::LoadMesh(const wchar_t* path) {
    auto* mesh = new CCMesh();
    CCMeshLoader* meshLoader = CCMeshLoader::GetMeshLoaderByFilePath(path);
    if (meshLoader) 
        meshLoader->Load(path, mesh);
    return mesh;
}

