#include "stdafx.h"
#include "CMainAlbumAdapter.h"
#include "MainDlg.h"

const wchar_t* KAttrName_Height[] = {
	L"imageHeight",
	L"titleHeight",
};
const wchar_t* KNodeName_Item[] = {
	L"itemImage",
	L"itemTitle",
};

CMainAlbumAdapter::CMainAlbumAdapter(std::vector<CMainAlbumItem>* items)
{
	this->items = items;
}
void CMainAlbumAdapter::InitByTemplate(pugi::xml_node xmlTemplate)
{
	m_nItemHeight[0] = xmlTemplate.attribute(KAttrName_Height[0]).as_int(160);
	m_nItemHeight[1] = xmlTemplate.attribute(KAttrName_Height[1]).as_int(30);
}
int CMainAlbumAdapter::getCount() { return items->size(); }
int CMainAlbumAdapter::getViewTypeCount() { return 2; }
int CMainAlbumAdapter::getItemViewType(int position, DWORD dwState) { return (*items)[position].Type; }
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
	pItem->GetEventSet()->subscribeEvent(EventLButtonUp::EventID, Subscriber(&CMainAlbumAdapter::OnItemClick, this));
	pItem->SetUserData(position);

	SCheckBox* pCheck = pItem->FindChildByName2<SCheckBox>(L"chk_check");
	SImageWnd* pImg = pItem->FindChildByName2<SImageWnd>(L"img_thumbnail");
	SStatic* pTextTitle = pItem->FindChildByName2<SStatic>(L"text_title");
	CMainAlbumItem& item = (*items)[position];

	pTextTitle->SetWindowText(item.FileName.c_str());
	pCheck->SetVisible(isCheckMode);
	if (nViewType == 1) 
		pImg->SetVisible(false);
	else if (nViewType == 0) {
		pCheck->SetCheck(item.Checked);
		pImg->SetVisible(true);

		if (item.ThumbnailPath != L"") {
			SAutoRefPtr<IBitmap> bmp;
			GETRENDERFACTORY->CreateBitmap(&bmp);
			if (SUCCEEDED(bmp->LoadFromFile(item.ThumbnailPath.c_str())))
				pImg->SetImage(bmp);
			else
				pImg->SetAttribute(L"skin", L"skin_img_thumbnail_def");
		} else 
			pImg->SetAttribute(L"skin", L"skin_img_thumbnail_def");
	}
}

void CMainAlbumAdapter::SetCheckMode(bool c)
{
	isCheckMode = c;
	notifyDataSetChanged();
}

bool CMainAlbumAdapter::OnItemStateChanged(EventArgs* e)
{
	EventSwndStateChanged* e2 = sobj_cast<EventSwndStateChanged>(e);
	if (!e2->CheckState(WndState_Hover)) return false;
	//通知界面重绘
	notifyDataSetInvalidated();
	return true;
}
bool CMainAlbumAdapter::OnItemClick(EventArgs* e)
{
	EventLButtonUp* e2 = sobj_cast<EventLButtonUp>(e);
	LOGIF(L"OnItemClick %d", ((SWindow*)e2->sender)->GetUserData());
	return true;
}

CMainAlbumItem::CMainAlbumItem(const wchar_t* filePath, const wchar_t* fileName)
{
	FilePath = filePath;
	FileName = fileName;
}
CMainAlbumItem::~CMainAlbumItem() = default;
