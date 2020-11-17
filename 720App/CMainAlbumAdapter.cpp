#include "stdafx.h"
#include "CMainAlbumAdapter.h"
#include "MainDlg.h"
#include "../720Core/CCThumbnailGenerator.h"
#include "../720Core/CCThreadMessageCenter.h"

const wchar_t* KAttrName_Height[] = {
	L"imageHeight",
	L"titleHeight",
};
const wchar_t* KNodeName_Item[] = {
	L"itemImage",
	L"itemTitle",
};

CMainAlbumAdapter::CMainAlbumAdapter(std::vector<CMainAlbumItem*>* items)
{
	this->items = items;
}
void CMainAlbumAdapter::InitByTemplate(pugi::xml_node xmlTemplate)
{
	m_nItemHeight[0] = xmlTemplate.attribute(KAttrName_Height[0]).as_int(156);
	m_nItemHeight[1] = xmlTemplate.attribute(KAttrName_Height[1]).as_int(30);
}
int CMainAlbumAdapter::getCount() { return items->size(); }
int CMainAlbumAdapter::getViewTypeCount() { return 2; }
int CMainAlbumAdapter::getItemViewType(int position, DWORD dwState) { return (*items)[position]->Type; }
SIZE CMainAlbumAdapter::getViewDesiredSize(int position, SWindow* pItem, int nWid, int nHei)
{
	DWORD dwState = pItem->GetState();
	int viewType = getItemViewType(position, dwState);
	return CSize(0, m_nItemHeight[viewType]);//cx在listview，mclistview中没有使用，不需要计算
}
void CMainAlbumAdapter::getView(int position, SWindow* pItem, pugi::xml_node xmlTemplate)
{
	int nViewType = getItemViewType(position, pItem->GetState());
	if (pItem->GetChildrenCount() == 0)
		pItem->InitFromXml(xmlTemplate.child(KNodeName_Item[nViewType]));
	pItem->GetEventSet()->subscribeEvent(EventSwndStateChanged::EventID, Subscriber(&CMainAlbumAdapter::OnItemStateChanged, this));
	pItem->SetUserData(position);

	SCheckBox* pCheck = pItem->FindChildByName2<SCheckBox>(L"chk_check");
	SImageWnd* pImg = pItem->FindChildByName2<SImageWnd>(L"img_thumbnail");
	SImageWnd* pImgOverlay = pItem->FindChildByName2<SImageWnd>(L"img_thumbnail_overlay"); 
	SStatic* pTextTitle = pItem->FindChildByName2<SStatic>(L"text_title");

	CMainAlbumItem* item = (*items)[position];

	if (pImgOverlay) {
		pImgOverlay->SetUserData(position);
		pImgOverlay->GetEventSet()->subscribeEvent(EventSwndStateChanged::EventID, Subscriber(&CMainAlbumAdapter::OnItemStateChanged, this));
	}
	if (pCheck) {
		pCheck->SetVisible(isCheckMode);
		pCheck->GetEventSet()->subscribeEvent(EventLButtonUp::EventID, Subscriber(&CMainAlbumAdapter::OnItemCheckMouseUp, this));
		pCheck->SetUserData(position);
	}

	pTextTitle->SetWindowText(item->FileName.c_str());
	if (nViewType == 1)  {
		if (pImg) pImg->SetVisible(false);
	}
	else if (nViewType == 0 )  {
		pCheck->SetCheck(item->Checked);
		pImg->SetVisible(true);

		//state
		DWORD state = pItem->GetState();
		if((state & WndState_Check) == WndState_Check)
			pImgOverlay->SetAttribute(L"skin", L"skin_img_thumbnail_select_overlay");
		else if ((state & WndState_Hover) == WndState_Hover)
			pImgOverlay->SetAttribute(L"skin", L"skin_img_thumbnail_hover_overlay");
		else
			pImgOverlay->SetAttribute(L"skin", L"skin_img_thumbnail_border_overlay");

		pImg->SetAttribute(L"skin", L"skin_img_thumbnail_def");
		if (item->ThumbnailPath != L"") {
			SAutoRefPtr<IBitmap> bmp;
			if (GETRENDERFACTORY->CreateBitmap(&bmp)) {
				if (SUCCEEDED(bmp->LoadFromFile(item->ThumbnailPath.c_str()))) {
					pImg->SetImage(bmp);
					pImg->SetAttribute(L"skin", L"");
				}
			}
		}
		else if (!item->ThumbnailLoadFailed && !item->ThumbnailLoadStarted) {
			item->ThumbnailLoadStarted = true;
			LoadThumbnail(item);
		}
	}
}

//===================================================
//公共方法
//===================================================

void CMainAlbumAdapter::CheckAll()
{
	checkItems.clear();
	for (auto it = items->begin(); it != items->end(); it++) {
		(*it)->Checked = true;
		checkItems.push_back((*it));
	}
	notifyDataSetChanged();
	CallCheckItemChangeCallback(checkItems.size());
}
void CMainAlbumAdapter::CheckNone()
{
	for (auto it = items->begin(); it != items->end(); it++)
		(*it)->Checked = false;
	checkItems.clear();
	notifyDataSetChanged();
	CallCheckItemChangeCallback(0);
}

void CMainAlbumAdapter::SetCheckMode(bool c)
{
	isCheckMode = c;
	notifyDataSetChanged();
}
void CMainAlbumAdapter::SetCheckItemChangeCallback(CMainCheckItemChangeCallback callback, void* data)
{
	checkItemChangeCallback = callback; 
	checkItemChangeCallbackData = data;
}
void CMainAlbumAdapter::SetItemClickCallback(CMainItemClickCallback callback, void* data)
{
	itemClickCallback = callback;
	itemClickCallbackData = data;
}
std::vector<CMainAlbumItem*>* CMainAlbumAdapter::GetCheckItems()
{
	return &checkItems;
}
int CMainAlbumAdapter::GetCheckItemCount()
{
	return checkItems.size();
}
int CMainAlbumAdapter::GetItemIndex(CMainAlbumItem*item)
{
	for (auto it = items->begin(); it != items->end(); it++)
		if((*it) == item)
			return distance(items->begin(), it);
	return -1;
}

//===================================================
//条目事件
//===================================================

bool CMainAlbumAdapter::OnItemStateChanged(EventArgs* e)
{
	EventSwndStateChanged* e2 = sobj_cast<EventSwndStateChanged>(e);
	if (e2->CheckState(WndState_Hover) || e2->CheckState(WndState_Check)) {
		//通知界面重绘
		notifyDataSetInvalidated();
		return true;
	}
	return false;
}
bool CMainAlbumAdapter::OnItemCheckMouseUp(EventArgs* e)
{
	EventLButtonUp* e2 = sobj_cast<EventLButtonUp>(e);
	int index = ((SWindow*)e2->sender)->GetUserData();

	CMainAlbumItem* item = (*items)[index];
	item->Checked = ((SCheckBox*)e2->sender)->IsChecked();

	LOGIF(L"OnItemCheckChanged %d", index);

	auto it = std::find(checkItems.begin(), checkItems.end(), item);
	if (it == checkItems.end()) {
		if (item->Checked) {
			checkItems.push_back(item);
			CallCheckItemChangeCallback(checkItems.size());
		}
	}
	else {
		if (!item->Checked) {
			checkItems.erase(it);
			CallCheckItemChangeCallback(checkItems.size());
		}
	}


	return true;
}

void CMainAlbumAdapter::CallCheckItemChangeCallback(int checkCount)
{
	if (checkItemChangeCallback)
		checkItemChangeCallback(checkItemChangeCallbackData, checkCount);
}

//===================================================
//缩略图加载
//===================================================

struct LoadThumbnailData {
	CMainAlbumItem* item;
	CMainAlbumAdapter* _this;
};

void CMainAlbumAdapter::LoadThumbnail(CMainAlbumItem* item) {
	LoadThumbnailData* data = new LoadThumbnailData();
	data->item = item;
	data->_this = this;
	CreateThread(NULL, NULL, LoadThumbnailThread, data, NULL, NULL);
}
DWORD WINAPI CMainAlbumAdapter::LoadThumbnailThread(LPVOID lpParam) {
	LoadThumbnailData* data = (LoadThumbnailData*)lpParam;
	AppGetAppInstance()->GetLogger()->Log(L"LoadThumbnailThread : %s", data->item->FileName.c_str());
	wchar_t*str = CCThumbnailGenerator::GetImageThumbnailAuto(data->item->FilePath.c_str());
	if (str) {
		data->item->ThumbnailPath = str;
		AppFree(str);
	}
	else
		data->item->ThumbnailLoadFailed = true;
	data->item->ThumbnailLoadStarted = false;
	AppGetAppInstance()->GetMessageCenter()->RunOnUIThread(data->_this, LoadThumbnailThreadMainThreadRun);
	free(data);
	return 0;
}
void CMainAlbumAdapter::LoadThumbnailThreadMainThreadRun(void* data) {
	((CMainAlbumAdapter*)data)->notifyDataSetChanged();
}

CMainAlbumItem::CMainAlbumItem(const wchar_t* filePath, const wchar_t* fileName)
{
	FilePath = filePath;
	FileName = fileName;
}
CMainAlbumItem::~CMainAlbumItem() = default;
