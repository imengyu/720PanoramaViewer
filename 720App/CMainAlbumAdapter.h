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
	bool ThumbnailLoadStarted = false;
	bool ThumbnailLoadFailed = false;
};

typedef void(*CMainCheckItemChangeCallback)(void* data, int checkCount);
typedef void(*CMainItemClickCallback)(void* data, int clickIndex);

class CMainAlbumAdapter : public SAdapterBase
{
public:

	CMainAlbumAdapter(std::vector<CMainAlbumItem*>* items);

	virtual void InitByTemplate(pugi::xml_node xmlTemplate) override;
	virtual int getCount() override;
	virtual int getViewTypeCount() override;
	virtual int getItemViewType(int position, DWORD dwState) override;
	virtual SIZE getViewDesiredSize(int position, SWindow* pItem, int nWid, int nHei) override;
	virtual void getView(int position, SWindow* pItem, pugi::xml_node xmlTemplate);

	void CheckAll();
	void CheckNone();

	void SetCheckMode(bool c);
	bool IsCheckMode();
	void SetCheckItemChangeCallback(CMainCheckItemChangeCallback callback, void*data);
	void SetItemClickCallback(CMainItemClickCallback callback, void* data);

	std::vector<CMainAlbumItem*>* GetCheckItems();
	int GetCheckItemCount();
	int GetItemIndex(CMainAlbumItem* item);

protected:

	bool isCheckMode = false;

	bool OnItemStateChanged(EventArgs* e);
	bool OnItemCheckMouseUp(EventArgs* e);

	void CallCheckItemChangeCallback(int checkCount);

	void LoadThumbnail(CMainAlbumItem* item);
	static DWORD __stdcall LoadThumbnailThread(LPVOID lpParam);
	static void LoadThumbnailThreadMainThreadRun(void* data);

protected:
	int m_nItemHeight[3] = { 0 };
	std::vector<CMainAlbumItem*>* items = nullptr;
	std::vector<CMainAlbumItem*> checkItems;

	CMainItemClickCallback itemClickCallback = nullptr;
	void* itemClickCallbackData = nullptr;
	CMainCheckItemChangeCallback checkItemChangeCallback = nullptr;
	void* checkItemChangeCallbackData = nullptr;
};


