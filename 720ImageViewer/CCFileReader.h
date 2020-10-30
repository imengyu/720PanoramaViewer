#pragma once
#include "stdafx.h"

//�ļ���ȡ��
class CCFileReader
{
public:
	CCFileReader();
	/**
	 * ���������ļ���ȡ��
	 * @param path Ŀ���ļ�·��
	 */
	CCFileReader(vstring& path);
	virtual ~CCFileReader();

	/**
	 * ��ȡ�ļ��Ƿ��Ѿ���
	 * @return
	 */
	virtual bool Opened();
	/**
	 * �ر��ļ�
	 */
	virtual void Close();

	/**
	 * ��ȡ�ļ�����
	 * @return �ļ�����
	 */
	virtual size_t Length();
	/**
	 * �ƶ�fpָ�����ļ���ʼ��ƫ��λ��
	 * @param i ָ��λ��
	 * @param seekType
	 */
	virtual void Seek(size_t i);
	/**
	 * �ƶ�fpָ��
	 * @param i ָ��λ��
	 * @param seekType λ�õ����ͣ�SEEK_*
	 */
	virtual void Seek(size_t i, int seekType);
	/**
	 * ��ȡ�ļ���� FILE*
	 * @return
	 */
	virtual FILE* Handle();

	/**
	 *
	 * @param arr ������
	 * @param offset ��ȡƫ��
	 * @param count ��ȡ����
	 */
	virtual void Read(BYTE* arr, size_t offset, size_t count);
	/**
	 * ��ȡһ���ֽ�
	 * @return �����ֽ�
	 */
	virtual BYTE ReadByte();
	/**
	 * ��ȡ�����ļ�
	 * @param size ���ػ�������С
	 * @return ���ػ�����
	 */
	virtual BYTE* ReadAllByte(size_t* size);

private:

	FILE* file = nullptr;
	size_t len = 0;

	void CloseFileHandle();
};

