#pragma once
#include "stdafx.h"

class CCMesh;
class CCTexture;
/**
 * ��Դ����
 */
class CCAssetsManager {

public:

    static const std::wstring GetResourcePath(const wchar_t* typeName, const wchar_t* name);
    static const std::wstring GetDirResourcePath(const wchar_t* dirName, const wchar_t* name);
    /**
     * ��ȡ��Դ·��
     * @param typeName �����ļ���
     * @param name ��Դ����
     * @return
     */
    static std::string GetResourcePath(const char* typeName, const char* name);
    /**
     * ��ȡ�Զ����ļ���·��
     * @param dirName �ļ���
     * @param name ��Դ����
     * @return
     */
    static std::string GetDirResourcePath(const char* dirName, const char* name);

    /**
     * ������Դ���ڴ�
     * @param path ��Դ����·��
     * @param bufferLength ������Ż�������С
     * @return �������ݻ�����
     */
    static BYTE* LoadResource(const wchar_t* path, size_t* bufferLength);

    /**
     * ���ļ�������ͼ
     * @param path ��ͼ·��
     * @return ������ͼ���������ʧ�ܷ���nullptr
     */
    static CCTexture* LoadTexture(const wchar_t* path);

    /**
     * ���ļ���������
     * @param path ����·��
     * @return ���������������ʧ�ܷ���nullptr
     */
    static CCMesh* LoadMesh(const wchar_t* path);
};
