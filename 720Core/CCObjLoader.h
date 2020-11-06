#pragma once
#include "CCMeshLoader.h"

//Obj ģ�ͼ�����
class CCObjLoader : public CCMeshLoader
{
	/**
	 * ���ļ�����Mesh
	 * @param path �ļ�·��
	 * @param mesh Ҫ�����ص�Mesh
	 * @return �����Ƿ�ɹ�
	 */
	bool Load(const wchar_t* path, CCMesh* mesh) override;
	/**
	 * ���ڴ����ݼ���Mesh
	 * @param buffer mesh�����ڴ�
	 * @param bufferSize mesh���ݴ�С
	 * @param mesh Ҫ�����ص�Mesh
	 * @return �����Ƿ�ɹ�
	 */
	bool Load(BYTE* buffer, size_t bufferSize, CCMesh* mesh) override;
};

