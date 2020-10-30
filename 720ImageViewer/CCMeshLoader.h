#pragma once
#ifndef VR720_CCMESHLOADER_H
#define VR720_CCMESHLOADER_H
#include "stdafx.h"

//Mesh ����
enum class CCMeshType {
	MeshTypeObj,
	MeshTypeFbx,
};

class CCMesh;
//Mesh ������
class CCMeshLoader
{
public:
	/**
	 * ��ȡָ�����͵�Mesh������
	 * @param type ���ͣ�CCMeshType
	 * @return ���ؼ�����
	 */
	static CCMeshLoader* GetMeshLoaderByType(CCMeshType type);
	/**
	 * ȫ�ֳ�ʼ��
	 */
	static void Init();
	/**
	 * ȫ���ͷ���Դ
	 */
	static void Destroy();

	/**
	 * ���ļ�����Mesh
	 * @param path �ļ�·��
	 * @param mesh Ҫ�����ص�Mesh
	 * @return �����Ƿ�ɹ�
	 */
	virtual bool Load(const vchar* path, CCMesh* mesh);
	/**
	 * ���ڴ����ݼ���Mesh
	 * @param buffer mesh�����ڴ�
	 * @param bufferSize mesh���ݴ�С
	 * @param mesh Ҫ�����ص�Mesh
	 * @return �����Ƿ�ɹ�
	 */
	virtual bool Load(BYTE* buffer, size_t bufferSize, CCMesh* mesh);
	/**
	 * ��ȡ��һ�μ��ط����Ĵ���
	 * @return ���ش���
	 */
	virtual const vchar* GetLastError();

protected:
	/**
	 * ���ü��ش���
	 * @param err ���ش���
	 */
	void SetLastError(const vchar* err);
private:
	vstring lastErr;
};

#endif
