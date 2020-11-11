#pragma once
#include "stdafx.h"
#include <helper/SAdapterBase.h>
#include <vector>
#include <string>

class CMainAlbumItem
{
public:
	CMainAlbumItem(const wchar_t* filePath, const wchar_t* fileName);
	~CMainAlbumItem();

	int Type = 0;
	std::wstring FilePath;
	std::wstring FileName;
	std::wstring ThumbnailPath;
	bool Checked = false;
};

class CMainAlbumAdapter : public SAdapterBase
{
public:

	CMainAlbumAdapter(std::vector<CMainAlbumItem>* items);

	virtual void InitByTemplate(pugi::xml_node xmlTemplate) override;
	virtual int getCount() override;
	virtual int getViewTypeCount() override;
	virtual int getItemViewType(int position, DWORD dwState) override;
	virtual SIZE getViewDesiredSize(int position, SWindow* pItem, int nWid, int nHei) override;
	virtual void getView(int position, SWindow* pItem, pugi::xml_node xmlTemplate);

	void SetCheckMode(bool c);

protected:

	bool isCheckMode = false;


	bool OnItemStateChanged(EventArgs* e);
	bool OnItemClick(EventArgs* e);
	bool OnButtonClick(EventArgs* pEvt);

protected:
	int m_nItemHeight[3] = { 0 };
	std::vector<CMainAlbumItem>* items = nullptr;
};


