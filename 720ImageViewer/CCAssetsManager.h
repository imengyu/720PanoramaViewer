//
// Created by roger on 2020/10/29.
//

#ifndef VR720_CCASSETSMANAGER_H
#define VR720_CCASSETSMANAGER_H

#include "stdafx.h"

/**
 * ��Դ����
 */
class CCAssetsManager {

public:
#ifdef VR720_WINDOWS
    static const std::wstring GetResourcePath(const wchar_t* typeName, const wchar_t* name);
    static const std::wstring GetDirResourcePath(const wchar_t* dirName, const wchar_t* name);
#endif
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
    static BYTE* LoadResource(const vchar* path, size_t* bufferLength);

#ifdef VR720_ANDROID
    /**
     * ��Դ�����JNI��ʼ��
     * @param env JNIEnv
     * @param assetManager getAssets
     */
    static void Android_InitFromJni(JNIEnv* env, jobject assetManager);
    /**
     * ���� Android assets �ļ�����Դ�����ػ�����ʹ����ɺ���Ҫ free
     * @param path ��Դ·��
     * @param buffer ������Ż�������ַ
     * @param bufferLength ������Ż�������С
     * @return �����Ƿ�ɹ�
     */
    static bool Android_LoadAsset(const char* path, BYTE** buffer, size_t* bufferLength);

#endif
};


#endif //VR720_CCASSETSMANAGER_H
