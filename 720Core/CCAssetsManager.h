#pragma once
#include "stdafx.h"

class CCMesh;
class CCTexture;
/**
 * 资源管理
 */
class CCAssetsManager {

public:

    static const std::wstring GetResourcePath(const wchar_t* typeName, const wchar_t* name);
    static const std::wstring GetDirResourcePath(const wchar_t* dirName, const wchar_t* name);
    /**
     * 获取资源路径
     * @param typeName 类型文件夹
     * @param name 资源名称
     * @return
     */
    static std::string GetResourcePath(const char* typeName, const char* name);
    /**
     * 获取自定义文件夹路径
     * @param dirName 文件夹
     * @param name 资源名称
     * @return
     */
    static std::string GetDirResourcePath(const char* dirName, const char* name);

    /**
     * 加载资源到内存
     * @param path 资源完整路径
     * @param bufferLength 用来存放缓冲区大小
     * @return 返回数据缓冲区
     */
    static BYTE* LoadResource(const wchar_t* path, size_t* bufferLength);

    /**
     * 从文件加载贴图
     * @param path 贴图路径
     * @return 返回贴图，如果加载失败返回nullptr
     */
    static CCTexture* LoadTexture(const wchar_t* path);

    /**
     * 从文件加载网格
     * @param path 网格路径
     * @return 返回网格，如果加载失败返回nullptr
     */
    static CCMesh* LoadMesh(const wchar_t* path);
};
