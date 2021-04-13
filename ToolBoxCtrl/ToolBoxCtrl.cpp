//==========================================================
// Author: Baradzenka Aleh (baradzenka@gmail.com)
//==========================================================
// 
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <cassert>
#include <vector>
#include <algorithm>
#include "ToolBoxCtrl.h"
/////////////////////////////////////////////////////////////////////////////
#pragma comment (lib, "Gdiplus.lib")
/////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4355)   // 'this' : used in base member initializer list.
#undef max
#undef min
#undef new
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// ToolBoxCtrl.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct ToolBoxCtrl::Private :
	IRecalc,
	Notify
{
	struct VirtualWindow : CDC
	{	VirtualWindow(CWnd *wnd)
		{	assert(wnd && ::IsWindow(wnd->m_hWnd));
			pwnd = wnd;
			pdc = pwnd->BeginPaint(&ps/*out*/);
			pwnd->GetClientRect(&rect/*out*/);
			if(CreateCompatibleDC(pdc) && bitmap.CreateCompatibleBitmap(pdc,rect.Width(),rect.Height()))
			{	SelectObject(&bitmap);
				SetBkMode(TRANSPARENT);
			}
		}
		~VirtualWindow()
		{	if(bitmap.m_hObject)
				pdc->BitBlt(0,0,rect.Width(),rect.Height(), this, 0,0, SRCCOPY);
			pwnd->EndPaint(&ps);
		}

	private:
		CWnd *pwnd;
		PAINTSTRUCT ps;
		CDC *pdc;
		CRect rect;
		CBitmap bitmap;
	};

public:
	Private(ToolBoxCtrl &owner);
	~Private();

private:
	ToolBoxCtrl &o;

private:   // ToolBoxCtrl::IRecalc.
	int GetBorderWidth(ToolBoxCtrl const *ctrl, IRecalc *base) override;
	CRect GetGroupPadding(ToolBoxCtrl const *ctrl, IRecalc *base) override;
	int GetGroupImageTextSpace(ToolBoxCtrl const *ctrl, IRecalc *base) override;
	CRect GetItemPadding(ToolBoxCtrl const *ctrl, IRecalc *base) override;
	int GetItemImageTextSpace(ToolBoxCtrl const *ctrl, IRecalc *base) override;

public:   // CmdUI.
	struct CmdUI : CCmdUI
	{	Private *p;
			// 
		int group, item;   // group index and item index in this group for UpdateModeIndividual.
		bool enable, check;   // for UpdateModeCommon.
		bool update;   // the need to redraw the control.
			// CmdUI.
		void Enable(BOOL enable) override;
		void SetCheck(int check) override;
	} m_CmdUI;

private:
	ULONG_PTR m_gdiPlusToken;

public:
	Draw *m_pDrawManager;
	IRecalc *m_pRecalcManager;
	ToolTip *m_pToolTipManager;
	Notify *m_pNotifyManager;
		// 
	struct Images
	{	Gdiplus::Bitmap *bmp;
		CSize size;
		COLORREF clrTransp;
	} m_Images[2];
	HCURSOR m_hGroupCursor;
	struct Font__
	{	CFont font, *fontRef;
	} m_Font[3];
		// 
	struct ActivMode
	{	ActivateMode mode;
		CPoint ptStartDragPos;
	} m_ActivMode;
	bool m_bDynamicItemsUpdate;
	UpdateMode m_UpdateMode;
		// 
	bool m_bBorder;
	bool m_bScroll;
	bool m_bOneGroupMode;
	bool m_bToolTip;
	DisabledItemImage m_DisabledItemImage;

public:
	CScrollBar m_Scroll;
	UINT_PTR m_TimerId;
		// 
	struct Item
	{	int id;   // can have any value, id for different items can be repeated. 
		CString text, desc;
		int image;   // or -1.
		Font font;
		COLORREF clrBack, clrFore;   // or CLR_NONE.
		bool enable, check;
		__int64 data;
	};
	struct Group
	{	CString text;
		bool expand;
		COLORREF clrFore;   // or CLR_NONE.
		std::vector<Item> items;
	};
	std::vector<Group> m_Groups;
	typedef std::vector<Group>::iterator i_groups;
	typedef std::vector<Group>::const_iterator ic_groups;
		// 
	CRect m_rcWork, m_rcScroll;
	int m_iGroupHeight, m_iItemHeight;
	int m_iCurPosY;   // current Y position (vertical offset of the control content).
		// 
	struct Unit
	{	int group, item;
	} m_Over, m_Pushed;
		// 
	CToolTipCtrl *m_pToolTip;

public:
	void Recalc();   // recalculation of control.
	int GetFullHeight() const;   // full height of all content including branches expansion.
	void GetTopVisible(int *group/*out*/, int *item/*out*/, int *deltaY/*out*/) const;
	void HitTest(CPoint const &pt, int *group/*out*/, int *item/*out*/) const;
	void LButtonDown(CPoint point);
	void MouseMove(CPoint point);
	bool LoadStateInner(CArchive *ar);
	bool SaveStateInner(CArchive *ar) const;
	void SetTimer();
	void KillTimer();
	void UpdateToolTips();
	int GetFontHeight(CFont *font) const;
	CSize GetTextSize(CFont *font, CString const &text) const;
	void SetScrollRange(CScrollBar *scroll, int min, int max, bool redraw) const;
	void SetScrollSize(CScrollBar *scroll, int size, bool redraw) const;
	void SetScrollPos(CScrollBar *scroll, int pos, bool redraw) const;
	int GetScrollPos(CScrollBar *scroll) const;
	bool LoadImage(HMODULE moduleRes/*or null*/, UINT resID, bool pngImage, Gdiplus::Bitmap **bmp/*out*/) const;
	bool CreateImageList(Gdiplus::Bitmap *bmp, int imageWidth, COLORREF clrMask/*or CLR_NONE*/, COLORREF clrBack/*or CLR_NONE*/, CImageList *imageList/*out*/) const;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(ToolBoxCtrl,CWnd)
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(ToolBoxCtrl, CWnd)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_CAPTURECHANGED()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// 
ToolBoxCtrl::ToolBoxCtrl() :
	p( *new Private(*this) )
{
}
// 
ToolBoxCtrl::~ToolBoxCtrl()
{	delete &p; 
}
/////////////////////////////////////////////////////////////////////////////
// 
ToolBoxCtrl::Private::Private(ToolBoxCtrl &owner) : o(owner)
{	m_CmdUI.p = this;
		// 
	m_pDrawManager = nullptr;
	m_pRecalcManager = this;
	m_pToolTipManager = nullptr;
	m_pNotifyManager = this;
		// 
	m_Images[ImageGroup].bmp = m_Images[ImageItem].bmp = nullptr;
	m_Images[ImageGroup].size.cx = m_Images[ImageGroup].size.cy = m_Images[ImageItem].size.cx = m_Images[ImageItem].size.cy = 0;
	m_hGroupCursor = nullptr;
	m_Font[ToolBoxCtrl::FontGroup].fontRef = m_Font[FontItemBasic].fontRef = m_Font[FontItemExtra].fontRef = nullptr;
		// 
	m_ActivMode.mode = ActivateModeClick;
	m_bDynamicItemsUpdate = false;
	m_UpdateMode = UpdateModeIndividual;
		// 
	m_bBorder = true;
	m_bScroll = true;
	m_bOneGroupMode = false;
	m_bToolTip = true;
	m_DisabledItemImage = DisabledItemImageTranslucent;
		// 
	m_gdiPlusToken = 0;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiPlusToken/*out*/,&gdiplusStartupInput,nullptr);
}
// 
ToolBoxCtrl::Private::~Private()
{	o.DestroyWindow();
		// 
	::delete m_Images[ImageGroup].bmp;
	::delete m_Images[ImageItem].bmp;
	if(m_hGroupCursor)
		::DestroyCursor(m_hGroupCursor);
	if(m_gdiPlusToken)
		Gdiplus::GdiplusShutdown(m_gdiPlusToken);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
BOOL ToolBoxCtrl::Create(LPCTSTR /*lpszClassName*/, LPCTSTR /*lpszWindowName*/, DWORD style, const RECT& rect, CWnd* parentWnd, UINT id, CCreateContext* /*context*/)
{	return Create(parentWnd,style,rect,id);
}
// 
bool ToolBoxCtrl::Create(CWnd *parent, DWORD style, RECT const &rect, UINT id)
{	assert( !GetSafeHwnd() );
		// 
	p.m_TimerId = 0;
		// 
	p.m_iCurPosY = 0;
	p.m_Over.group = p.m_Over.item = -1;
	p.m_Pushed.group = p.m_Pushed.item = -1;
	p.m_pToolTip = nullptr;
		// 
	const CString className = AfxRegisterWndClass(CS_DBLCLKS,::LoadCursor(nullptr,IDC_ARROW),nullptr,nullptr);
	if( !CWnd::Create(className,_T(""),style | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,rect,parent,id) ||
		!p.m_Scroll.Create(WS_CHILD | WS_CLIPCHILDREN | SBS_VERT,CRect(0,0,0,0),this,100) )
		return false;
		// 
	CFont *font = CFont::FromHandle( static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT)) );
	if( !GetFont(FontGroup) )
		SetFont(FontGroup,font);
	if( !GetFont(FontItemBasic) )
		SetFont(FontItemBasic,font);
	if( !GetFont(FontItemExtra) )
		SetFont(FontItemExtra,font);
		//
	if( IsDynamicItemsUpdateEnable() )
		p.SetTimer();
		// 
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::OnDestroy()
{	if(p.m_pToolTipManager && p.m_pToolTip) 
		p.m_pToolTipManager->DestroyToolTip(p.m_pToolTip);
	DeleteAll();
	p.KillTimer();
		// 
	CWnd::OnDestroy();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::Update(bool recalc)
{	Invalidate(FALSE);
	if(recalc)
		p.Recalc();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetDrawManager(Draw *ptr/*or null*/)
{	p.m_pDrawManager = ptr;
}
// 
ToolBoxCtrl::Draw *ToolBoxCtrl::GetDrawManager() const
{	return p.m_pDrawManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetRecalcManager(IRecalc *ptr/*or null*/)
{	p.m_pRecalcManager = (ptr ? ptr : &p);
}
// 
ToolBoxCtrl::IRecalc *ToolBoxCtrl::GetRecalcManager() const
{	return p.m_pRecalcManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetToolTipManager(ToolTip *ptr/*or null*/)
{	if(ptr!=p.m_pToolTipManager)
	{	if(p.m_pToolTipManager && p.m_pToolTip) 
			p.m_pToolTipManager->DestroyToolTip(p.m_pToolTip);
		p.m_pToolTip = nullptr;
		p.m_pToolTipManager = ptr;
	}
}
// 
ToolBoxCtrl::ToolTip *ToolBoxCtrl::GetToolTipManager() const
{	return p.m_pToolTipManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetNotifyManager(Notify *ptr/*or null*/)
{	p.m_pNotifyManager = (ptr ? ptr : &p);
}
// 
ToolBoxCtrl::Notify *ToolBoxCtrl::GetNotifyManager() const
{	return p.m_pNotifyManager;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool ToolBoxCtrl::CreateImages(Image type, HMODULE moduleRes/*or null*/, UINT resID/*or 0*/, bool pngImage, int imageWidth, COLORREF clrTransp/*=CLR_NONE*/)
{	assert(!resID || imageWidth>0);
		// 
	if(p.m_Images[type].bmp)
	{	::delete p.m_Images[type].bmp;
		p.m_Images[type].bmp = nullptr;
	}
		// 
	const bool res = (!resID || p.LoadImage(moduleRes,resID,pngImage,&p.m_Images[type].bmp/*out*/));
	(p.m_Images[type].bmp ?
		p.m_Images[type].size.SetSize(imageWidth, p.m_Images[type].bmp->GetHeight() ) :
		p.m_Images[type].size.SetSize(0,0));
	p.m_Images[type].clrTransp = clrTransp;
		// 
	return res;
}
//
Gdiplus::Bitmap *ToolBoxCtrl::GetImages(Image type) const
{	return p.m_Images[type].bmp;
}
//
bool ToolBoxCtrl::GetImageList(Image type, COLORREF clrDstBack/*or CLR_NONE*/, CImageList *imageList/*out*/) const
{	assert(imageList);
		// 
	return (p.m_Images[type].bmp && p.CreateImageList(p.m_Images[type].bmp, p.m_Images[type].size.cx, p.m_Images[type].clrTransp, clrDstBack, imageList/*out*/));
}
// 
CSize ToolBoxCtrl::GetImageSize(Image type) const
{	return p.m_Images[type].size;
}
//
COLORREF ToolBoxCtrl::GetImageTranspColor(Image type) const
{	return p.m_Images[type].clrTransp;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool ToolBoxCtrl::SetGroupCursor(HMODULE module, UINT resID)
{	if(p.m_hGroupCursor)
	{	::DestroyCursor(p.m_hGroupCursor);
		p.m_hGroupCursor = nullptr;
	}
		// 
	if(!resID)
		return true;
	if(!module)
	{	module = AfxGetResourceHandle();
		if(!module)
			return false;
	}
	p.m_hGroupCursor = ::LoadCursor(module,MAKEINTRESOURCE(resID));
	return p.m_hGroupCursor!=nullptr;
}
// 
bool ToolBoxCtrl::SetGroupCursor(HCURSOR cursor)
{	if(p.m_hGroupCursor)
	{	::DestroyCursor(p.m_hGroupCursor);
		p.m_hGroupCursor = nullptr;
	}
		// 
	if(!cursor)
		return true;
	p.m_hGroupCursor = static_cast<HCURSOR>( CopyImage(cursor,IMAGE_CURSOR,0,0,0) );
	return p.m_hGroupCursor!=nullptr;
}
// 
HCURSOR ToolBoxCtrl::GetGroupCursor() const
{	return p.m_hGroupCursor;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool ToolBoxCtrl::SetFont(Font type, CFont *font)
{	assert(font && font->m_hObject);
		// 
	LOGFONT lf;
	font->GetLogFont(&lf/*out*/);
	return SetFont(type,&lf);
}
// 
void ToolBoxCtrl::SetFontRef(Font type, CFont *font)
{	assert(font && font->m_hObject);
		// 
	if(p.m_Font[type].font.m_hObject)
		p.m_Font[type].font.DeleteObject();
	p.m_Font[type].fontRef = font;
}
// 
bool ToolBoxCtrl::SetFont(Font type, LOGFONT const *lf)
{	assert(lf);
		// 
	if(p.m_Font[type].font.m_hObject)
		p.m_Font[type].font.DeleteObject();
	p.m_Font[type].fontRef = nullptr;
	if( !p.m_Font[type].font.CreateFontIndirect(lf) )
		return false;
	p.m_Font[type].fontRef = &p.m_Font[type].font;
	return true;
}
// 
CFont *ToolBoxCtrl::GetFont(Font type) const
{	return p.m_Font[type].fontRef;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::AddGroup(TCHAR const *text, COLORREF clrFore/*=CLR_NONE*/)
{	assert(text);
		// 
	Private::Group group;
	group.text = text;
	group.expand = true;
	group.clrFore = clrFore;
	p.m_Groups.push_back(group);
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::AddItem(int group, int id, TCHAR const *text, TCHAR const *desc/*=null*/, int image/*=-1*/, 
	Font font/*=FontItemBasic*/, COLORREF clrBack/*=CLR_NONE*/, COLORREF clrFore/*=CLR_NONE*/)
{
	assert(group>=0 && group<GetNumberGroups());
	assert(text);
	assert(image>=-1);
	assert(font==FontItemBasic || font==FontItemExtra);
		// 
	Private::Item item;
	item.id = id;
	item.text = text;
	if(desc)
		item.desc = desc;
	item.image = image;
	item.font = font;
	item.clrBack = clrBack;
	item.clrFore = clrFore;
	item.enable = true;
	item.check = false;
	item.data = 0;
	p.m_Groups[group].items.push_back(item);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::DeleteGroup(int group)
{	assert(group>=0 && group<GetNumberGroups());
		// 
	p.m_Groups.erase( p.m_Groups.begin()+group );
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::DeleteItem(int group, int item)
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	p.m_Groups[group].items.erase( p.m_Groups[group].items.begin()+item );
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::DeleteAll()
{	p.m_Groups.clear();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int ToolBoxCtrl::GetNumberGroups() const
{	return static_cast<int>( p.m_Groups.size() );
}
/////////////////////////////////////////////////////////////////////////////
// 
int ToolBoxCtrl::GetNumberItems(int group) const
{	assert(group>=0 && group<GetNumberGroups());
		// 
	return static_cast<int>( p.m_Groups[group].items.size() );
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetActivateMode(ActivateMode mode)
{	p.m_ActivMode.mode = mode;
}
// 
ToolBoxCtrl::ActivateMode ToolBoxCtrl::GetActivateMode() const
{	return p.m_ActivMode.mode;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::CancelDragMode()
{	assert( GetActivateMode()==ActivateModeDrag );
		// 
	p.m_Pushed.group = p.m_Pushed.item = -1;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetUpdateMode(UpdateMode mode)
{	p.m_UpdateMode = mode;
}
// 
ToolBoxCtrl::UpdateMode ToolBoxCtrl::GetUpdateMode() const
{	return p.m_UpdateMode;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetGroupText(int group, TCHAR const *text)
{	assert(group>=0 && group<GetNumberGroups());
	assert(text);
		// 
	p.m_Groups[group].text = text;
}
// 
CString ToolBoxCtrl::GetGroupText(int group) const
{	assert(group>=0 && group<GetNumberGroups());
		// 
	return p.m_Groups[group].text;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetGroupForeColor(int group, COLORREF color)
{	assert(group>=0 && group<GetNumberGroups());
		// 
	p.m_Groups[group].clrFore = color;
}
// 
COLORREF ToolBoxCtrl::GetGroupForeColor(int group) const
{	assert(group>=0 && group<GetNumberGroups());
		// 
	return p.m_Groups[group].clrFore;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetItemId(int group, int item, int id)
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	p.m_Groups[group].items[item].id = id;
}
// 
int ToolBoxCtrl::GetItemId(int group, int item) const
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	return p.m_Groups[group].items[item].id;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetItemText(int group, int item, TCHAR const *text)
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
	assert(text);
		// 
	p.m_Groups[group].items[item].text = text;
}
// 
CString ToolBoxCtrl::GetItemText(int group, int item) const
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	return p.m_Groups[group].items[item].text;
}
/////////////////////////////////////////////////////////////////////////////
//
void ToolBoxCtrl::SetItemDescription(int group, int item, TCHAR const *text)
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
	assert(text);
		// 
	p.m_Groups[group].items[item].desc = text;
}
// 
CString ToolBoxCtrl::GetItemDescription(int group, int item) const
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	return p.m_Groups[group].items[item].desc;
}
/////////////////////////////////////////////////////////////////////////////
//
void ToolBoxCtrl::SetItemImage(int group, int item, int image)
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
	assert(image>=-1);
		// 
	p.m_Groups[group].items[item].image = image;
}
// 
int ToolBoxCtrl::GetItemImage(int group, int item) const
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	return p.m_Groups[group].items[item].image;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetItemFont(int group, int item, Font font)
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
	assert(font==FontItemBasic || font==FontItemExtra);
		// 
	p.m_Groups[group].items[item].font = font;
}
// 
ToolBoxCtrl::Font ToolBoxCtrl::GetItemFont(int group, int item) const
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	return p.m_Groups[group].items[item].font;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetItemBackColor(int group, int item, COLORREF color)
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	p.m_Groups[group].items[item].clrBack = color;
}
// 
COLORREF ToolBoxCtrl::GetItemBackColor(int group, int item) const
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	return p.m_Groups[group].items[item].clrBack;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetItemForeColor(int group, int item, COLORREF color)
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	p.m_Groups[group].items[item].clrFore = color;
}
// 
COLORREF ToolBoxCtrl::GetItemForeColor(int group, int item) const
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	return p.m_Groups[group].items[item].clrFore;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::EnableItem(int group, int item, bool enable)
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	p.m_Groups[group].items[item].enable = enable;
}
// 
bool ToolBoxCtrl::IsItemEnabled(int group, int item) const
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	return p.m_Groups[group].items[item].enable;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::CheckItem(int group, int item, bool check)
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	p.m_Groups[group].items[item].check = check;
}
// 
bool ToolBoxCtrl::IsItemChecked(int group, int item) const
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	return p.m_Groups[group].items[item].check;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetItemData(int group, int item, __int64 data)
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	p.m_Groups[group].items[item].data = data;
}
// 
__int64 ToolBoxCtrl::GetItemData(int group, int item) const
{	assert(group>=0 && group<GetNumberGroups());
	assert(item>=0 && item<GetNumberItems(group));
		// 
	return p.m_Groups[group].items[item].data;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::ShowBorder(bool show)
{	p.m_bBorder = show;
}
// 
bool ToolBoxCtrl::IsBorderVisible() const
{	return p.m_bBorder;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::ShowScroll(bool show)
{	p.m_bScroll = show;
}
// 
bool ToolBoxCtrl::IsScrollVisible() const
{	return p.m_bScroll;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::EnableOneGroupMode(bool enable)
{	p.m_bOneGroupMode = enable;
}
// 
bool ToolBoxCtrl::IsOneGroupModeEnable() const
{	return p.m_bOneGroupMode;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::EnableToolTip(bool enable)
{	if(enable != p.m_bToolTip)
	{	if(p.m_pToolTipManager && p.m_pToolTip) 
			p.m_pToolTipManager->DestroyToolTip(p.m_pToolTip);
		p.m_pToolTip = nullptr;
		p.m_bToolTip = enable;
	}
}
// 
bool ToolBoxCtrl::IsToolTipEnable() const
{	return p.m_bToolTip;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::SetDisabledItemImageType(DisabledItemImage type)
{	p.m_DisabledItemImage = type;
}
// 
ToolBoxCtrl::DisabledItemImage ToolBoxCtrl::GetDisabledItemImageType() const
{	return p.m_DisabledItemImage;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::EnableDynamicItemsUpdate(bool enable)
{	if(enable!=p.m_bDynamicItemsUpdate)
	{	p.m_bDynamicItemsUpdate = enable;
		(enable ? p.SetTimer() : p.KillTimer());
	}
}
// 
bool ToolBoxCtrl::IsDynamicItemsUpdateEnable() const
{	return p.m_bDynamicItemsUpdate;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::CollapseGroup(int group)
{	assert(group>=0 && group<GetNumberGroups());
		// 
	p.m_Groups[group].expand = false;
}
// 
void ToolBoxCtrl::ExpandGroup(int group)
{	assert(group>=0 && group<GetNumberGroups());
		// 
	p.m_Groups[group].expand = true;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool ToolBoxCtrl::IsGroupExpanded(int group) const
{	assert(group>=0 && group<GetNumberGroups());
		// 
	return p.m_Groups[group].expand;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::CollapseAllGroups()
{	for(Private::i_groups i=p.m_Groups.begin(), e=p.m_Groups.end(); i!=e; ++i)
		i->expand = false;
}
// 
void ToolBoxCtrl::ExpandAllGroups()
{	for(Private::i_groups i=p.m_Groups.begin(), e=p.m_Groups.end(); i!=e; ++i)
		i->expand = true;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::GetItemUnderCursor(int *group/*out,or null*/, int *item/*out,or null*/) const
{	if(group)
		*group = p.m_Over.group;
	if(item)
		*item = p.m_Over.item;
}
// 
void ToolBoxCtrl::GetItemPushed(int *group/*out,or null*/, int *item/*out,or null*/) const
{	if(group)
		*group = p.m_Pushed.group;
	if(item)
		*item = p.m_Pushed.item;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int ToolBoxCtrl::Private::GetBorderWidth(ToolBoxCtrl const * /*ctrl*/, IRecalc * /*base*/) { return 1; }
CRect ToolBoxCtrl::Private::GetGroupPadding(ToolBoxCtrl const * /*ctrl*/, IRecalc * /*base*/) { return CRect(5,4,2,5); }
int ToolBoxCtrl::Private::GetGroupImageTextSpace(ToolBoxCtrl const * /*ctrl*/, IRecalc * /*base*/) { return 5; }
CRect ToolBoxCtrl::Private::GetItemPadding(ToolBoxCtrl const * /*ctrl*/, IRecalc * /*base*/) { return CRect(6,2,2,2); }
int ToolBoxCtrl::Private::GetItemImageTextSpace(ToolBoxCtrl const * /*ctrl*/, IRecalc * /*base*/) { return 10; }
// 
/////////////////////////////////////////////////////////////////////////////
// 
int ToolBoxCtrl::GetBorderWidth() const { return p.m_pRecalcManager->GetBorderWidth(this,&p); }
CRect ToolBoxCtrl::GetGroupPadding() const { return p.m_pRecalcManager->GetGroupPadding(this,&p); }
int ToolBoxCtrl::GetGroupImageTextSpace() const { return p.m_pRecalcManager->GetGroupImageTextSpace(this,&p); }
CRect ToolBoxCtrl::GetItemPadding() const { return p.m_pRecalcManager->GetItemPadding(this,&p); }
int ToolBoxCtrl::GetItemImageTextSpace() const { return p.m_pRecalcManager->GetItemImageTextSpace(this,&p); }
// 
/////////////////////////////////////////////////////////////////////////////
//
CToolTipCtrl *ToolBoxCtrl::GetToolTip() const { return p.m_pToolTip; }
// 
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::OnSize(UINT nType, int cx, int cy)
{	CWnd::OnSize(nType, cx, cy);
		//
	if(m_hWnd && p.m_Scroll.m_hWnd)
	{	p.Recalc();
		Invalidate(FALSE);
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::Private::Recalc()
{	o.GetClientRect(&m_rcWork);
		// 
	if( o.IsBorderVisible() )
	{	const int borderWidth = o.GetBorderWidth();
		m_rcWork.DeflateRect(borderWidth,borderWidth);
	}
		// 
	const CRect rcGroupPadding = o.GetGroupPadding();
	const CRect rcItemPadding = o.GetItemPadding();
		// 
	m_iGroupHeight = rcGroupPadding.top + std::max<int>(m_Images[ImageGroup].size.cy, GetFontHeight(m_Font[ToolBoxCtrl::FontGroup].fontRef) ) + rcGroupPadding.bottom;   // group height.
	int itemTextHeight = GetFontHeight(m_Font[FontItemBasic].fontRef);   // the height of the item's text.
	if(m_Font[FontItemExtra].fontRef)
		itemTextHeight = std::max(itemTextHeight, GetFontHeight(m_Font[FontItemExtra].fontRef) );   // the height of the item's text.
	m_iItemHeight = rcItemPadding.top + std::max<int>(m_Images[ImageItem].size.cy,itemTextHeight) + rcItemPadding.bottom;   // item height. 
		// 
	const int iFullHeight = GetFullHeight();   // the height of the entire scroll.
	const bool allContentVisible = (iFullHeight <= m_rcWork.Height());
		// 
	if(!allContentVisible)
	{	if(m_iCurPosY < 0)
			m_iCurPosY = 0;
		if(m_iCurPosY > iFullHeight-m_rcWork.Height()) 
			m_iCurPosY = iFullHeight-m_rcWork.Height();
	}
	else
		m_iCurPosY = 0;
		// 
	if(!m_rcWork.IsRectEmpty() &&
		!allContentVisible && 
		o.IsScrollVisible())
	{
		m_rcScroll = m_rcWork;
		m_rcScroll.left = m_rcWork.right -= ::GetSystemMetrics(SM_CXVSCROLL)-4;
			// 
		SetScrollRange(&m_Scroll,0,iFullHeight-1,false);
		SetScrollSize(&m_Scroll,m_rcWork.Height(),false);
		SetScrollPos(&m_Scroll,m_iCurPosY,true);
			// 
		CRect rcScrollOld;
		m_Scroll.GetWindowRect(&rcScrollOld);
		o.ScreenToClient(&rcScrollOld);
		if(m_rcScroll != rcScrollOld)
			m_Scroll.SetWindowPos(nullptr, m_rcScroll.left,m_rcScroll.top, m_rcScroll.Width(),m_rcScroll.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
			// 
		if((m_Scroll.GetStyle() & WS_VISIBLE)==0)
			::ShowWindow(m_Scroll.m_hWnd,SW_SHOWNA);
	}
	else   // the scroll should not be displayed.
	{	m_rcScroll.SetRectEmpty();
			// 
		if((m_Scroll.GetStyle() & WS_VISIBLE)!=0)
		{	::ShowWindow(m_Scroll.m_hWnd,SW_HIDE);
			o.Invalidate(FALSE);
		}
	}
		// 
	UpdateToolTips();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Don't use CWnd::PreTranslateMessage to call CToolTipCtrl::RelayEvent.
//  If TabCtrl is in a Regular MFC DLL, then CWnd::PreTranslateMessage is not called. 
// 
LRESULT ToolBoxCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{	if(p.m_bToolTip)
		if(p.m_pToolTip && p.m_pToolTip->m_hWnd)
			if(message==WM_LBUTTONDOWN || message==WM_LBUTTONUP || message==WM_MBUTTONDOWN || message==WM_MBUTTONUP ||   // All messages required for TTM_RELAYEVENT.
				message==WM_MOUSEMOVE || message==WM_NCMOUSEMOVE || message==WM_RBUTTONDOWN || message==WM_RBUTTONUP)
			{
				// Don't use AfxGetCurrentMessage(). If TabCtrl is in a Regular MFC DLL, then we get an empty MSG. 
				MSG msg = {m_hWnd,message,wParam,lParam,0,{0,0}};
				p.m_pToolTip->RelayEvent(&msg);
			}
	return CWnd::WindowProc(message,wParam,lParam);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::Private::UpdateToolTips()
{	if(m_bToolTip && m_pToolTipManager)
	{	if(!m_pToolTip)
			m_pToolTip = m_pToolTipManager->CreateToolTip(&o);
			// 
		if(m_pToolTip && m_pToolTip->m_hWnd)
		{	for(int c=m_pToolTip->GetToolCount(); c>0; --c)
				m_pToolTip->DelTool(&o,c);
				//
			if( !m_rcWork.IsRectEmpty() )
			{	int group,item, deltaY;
				GetTopVisible(&group/*out*/,&item/*out*/, &deltaY/*out*/);
					// 
				if(group!=-1)
				{	const CRect rcGroupPadding = o.GetGroupPadding();
					const int groupImageTextSpace = o.GetGroupImageTextSpace();
					const CRect rcItemPadding = o.GetItemPadding();
					const int itemImageTextSpace = o.GetItemImageTextSpace();
						//
					const Gdiplus::Bitmap *bmpImageGroup = m_Images[ToolBoxCtrl::ImageGroup].bmp;
					const CSize szImageGroup = m_Images[ToolBoxCtrl::ImageGroup].size;
					const Gdiplus::Bitmap *bmpImageItem = m_Images[ToolBoxCtrl::ImageItem].bmp;
					const CSize szImageItem = m_Images[ToolBoxCtrl::ImageItem].size;
						// 
					int c = 1;
					for(int y=m_rcWork.top+deltaY; y<m_rcWork.bottom; )
						if(item==-1)   // group.
						{	int availableWidth = m_rcWork.right - m_rcWork.left - rcGroupPadding.left - rcGroupPadding.right;
							if(bmpImageGroup)
								availableWidth -= szImageGroup.cx + groupImageTextSpace;
							const int textHeight = GetTextSize( m_Font[FontGroup].fontRef, m_Groups[group].text).cx;   // ширина текста группы.
							if(textHeight > availableWidth)
							{	const CRect rc(m_rcWork.left,y,m_rcWork.right,y+m_iGroupHeight);
								m_pToolTip->AddTool(&o,m_Groups[group].text,&rc,c++);
							}
								// 
							y += m_iGroupHeight;
								// 
							if(o.GetNumberItems(group)>0 && o.IsGroupExpanded(group))
								item = 0;
							else if(++group==o.GetNumberGroups())
								break;
						}
						else   // item.
						{	int availableWidth = m_rcWork.right - m_rcWork.left - rcItemPadding.left - rcItemPadding.right;
							if(bmpImageItem)
								availableWidth -= szImageItem.cx + itemImageTextSpace;
							const Font font = m_Groups[group].items[item].font;
							CString const &text = m_Groups[group].items[item].text;
							const int textHeight = GetTextSize( m_Font[font].fontRef, text).cx;   // ширина текста группы.
							if(textHeight > availableWidth)
							{	const CRect rc(m_rcWork.left,y,m_rcWork.right,y+m_iItemHeight);
								m_pToolTip->AddTool(&o,text,&rc,c++);
							}
								// 
							y += m_iItemHeight;
								// 
							if(++item==o.GetNumberItems(group))
							{	if(++group==o.GetNumberGroups())
									break;
								item = -1;
							}
						}
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::OnPaint()
{	if(!p.m_pDrawManager)
	{	CPaintDC dc(this);
		return;
	}
		// 
	Private::VirtualWindow virtwnd(this);
	if( !virtwnd.GetSafeHdc() )
	{	CPaintDC dc(this);
		return;
	}
		// 
	p.m_pDrawManager->DrawBegin(this,&virtwnd);
		// 
	if( !p.m_rcWork.IsRectEmpty() )
	{		// Painting the background of the work area.
		p.m_pDrawManager->DrawWorkAreaBack(this,&virtwnd,&p.m_rcWork);
			// 
			// Drawing items.
		int group,item, deltaY;
		p.GetTopVisible(&group/*out*/,&item/*out*/, &deltaY/*out*/);
			// 
		if(group!=-1)
		{	CRect rc;
			for(int y=p.m_rcWork.top+deltaY; y<p.m_rcWork.bottom; )
				if(item==-1)   // group drawing.
				{	rc.SetRect(p.m_rcWork.left,y,p.m_rcWork.right,y+p.m_iGroupHeight);
					p.m_pDrawManager->DrawGroup(this,&virtwnd,group,&rc);
					y += p.m_iGroupHeight;
						// 
					if(GetNumberItems(group)>0 && IsGroupExpanded(group))
						item = 0;
					else if(++group==GetNumberGroups())
						break;
				}
				else   // item drawing.
				{	rc.SetRect(p.m_rcWork.left,y,p.m_rcWork.right,y+p.m_iItemHeight);
					p.m_pDrawManager->DrawItem(this,&virtwnd,group,item,&rc);
					y += p.m_iItemHeight;
						// 
					if(++item==GetNumberItems(group))
					{	if(++group==GetNumberGroups())
							break;
						item = -1;
					}
				}
		}
	}
		// 
	if(IsBorderVisible() && GetBorderWidth()>0)
	{	CRect rc;
		GetClientRect(&rc/*out*/);
		p.m_pDrawManager->DrawBorder(this,&virtwnd,&rc);
	}
		// 
	p.m_pDrawManager->DrawEnd(this,&virtwnd);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{	switch(nSBCode)
	{	case SB_LINEUP: p.m_iCurPosY -= 1; break;
		case SB_LINEDOWN: p.m_iCurPosY += 1; break;
		case SB_PAGEUP: p.m_iCurPosY -= p.m_rcWork.Height(); break;
		case SB_PAGEDOWN: p.m_iCurPosY += p.m_rcWork.Height(); break;
		case SB_THUMBTRACK: p.m_iCurPosY = p.GetScrollPos(pScrollBar); break;
	}
	Update(true);
		// 
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}
/////////////////////////////////////////////////////////////////////////////
// 
BOOL ToolBoxCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{	if(nFlags==0)
	{	UINT lines;
		if( ::SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,&lines/*out*/,0) )
		{	p.m_iCurPosY += p.m_iItemHeight * lines * (zDelta>0 ? -1 : 1);   // don't use group heights.
			Update(true);
				// 
			CPoint ptClient = pt;
			ScreenToClient(&ptClient);
			p.MouseMove(ptClient);
				// 
			if(p.m_hGroupCursor)
				::SetCursor(p.m_Over.group!=-1 && p.m_Over.item==-1 ? p.m_hGroupCursor : ::LoadCursor(nullptr,IDC_ARROW));
		}
	}
		// 
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::Private::SetTimer()
{	if(!m_TimerId)
		m_TimerId = o.SetTimer(1,40,nullptr);
}
// 
void ToolBoxCtrl::Private::KillTimer()
{	if(m_TimerId)
	{	o.KillTimer(m_TimerId);
		m_TimerId = 0;
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::OnTimer(UINT_PTR nIDEvent)
{	if(::IsWindowVisible(m_hWnd) && !p.m_rcWork.IsRectEmpty())
		if(p.m_pNotifyManager!=&p)
		{	int group,item, deltaY;
			p.GetTopVisible(&group/*out*/,&item/*out*/, &deltaY/*out*/);
				// 
			if(group!=-1)
			{	p.m_CmdUI.update = false;
				p.m_CmdUI.m_nID = 0;   // CCmdUI::m_nID.
					// 
				for(int y=p.m_rcWork.top+deltaY; y<p.m_rcWork.bottom; )
					if(item==-1)   // group.
					{	if(GetNumberItems(group)>0 && IsGroupExpanded(group))
							item = 0;
						else if(++group==GetNumberGroups())
							break;
						y += p.m_iGroupHeight;
					}
					else   // item.
					{	Private::Item &i = p.m_Groups[group].items[item];
							// 
						if(p.m_UpdateMode==UpdateModeIndividual)
						{	p.m_CmdUI.m_nID = i.id;   // CCmdUI::m_nID.
							p.m_CmdUI.group = group;
							p.m_CmdUI.item = item;
							p.m_pNotifyManager->OnUpdateItemState(this,group,item,i.id,&p.m_CmdUI);
						}
						else   // UpdateModeCommon.
						{	if(p.m_CmdUI.m_nID==0)   // the first item encountered.
							{	p.m_CmdUI.enable = true;
								p.m_CmdUI.check = false;
								p.m_pNotifyManager->OnUpdateItemState(this,-1,-1,0,&p.m_CmdUI);
								p.m_CmdUI.m_nID = 1;   // CCmdUI::m_nID.
							}
								// 
							if(i.enable!=p.m_CmdUI.enable || i.check!=p.m_CmdUI.check)
							{	i.enable = p.m_CmdUI.enable;
								i.check = p.m_CmdUI.check;
								p.m_CmdUI.update = true;
							}
						}
							// 
						if(++item==GetNumberItems(group))
						{	if(++group==GetNumberGroups())
								break;
							item = -1;
						}
						y += p.m_iItemHeight;
					}
					// 
				if(p.m_CmdUI.update)
					InvalidateRect(&p.m_rcWork,FALSE);
			}
		}
		// 
	CWnd::OnTimer(nIDEvent);
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::Private::CmdUI::Enable(BOOL enableNew)
{	if(p->m_UpdateMode==UpdateModeIndividual)
	{	Item &i = p->m_Groups[CmdUI::group].items[CmdUI::item];
		if((enableNew!=0) != i.enable)
		{	i.enable = enableNew!=0;
			CmdUI::update = true;
		}
	}
	else
		if((enableNew!=0) != p->m_CmdUI.enable)
			p->m_CmdUI.enable = enableNew!=0;
}
// 
void ToolBoxCtrl::Private::CmdUI::SetCheck(int checkNew)
{	if(p->m_UpdateMode==UpdateModeIndividual)
	{	Item &i = p->m_Groups[CmdUI::group].items[CmdUI::item];
		if((checkNew!=0) != i.check)
		{	i.check = checkNew!=0;
			CmdUI::update = true;
		}
	}
	else
		if((checkNew!=0) != p->m_CmdUI.check)
			p->m_CmdUI.check = checkNew!=0;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{	CWnd::OnLButtonDown(nFlags,point);
		// 
	p.LButtonDown(point);
}
// 
void ToolBoxCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{	CWnd::OnLButtonDblClk(nFlags,point);
		// 
	p.LButtonDown(point);
}
// 
void ToolBoxCtrl::Private::LButtonDown(CPoint point)
{	HitTest(point,&m_Pushed.group/*out*/,&m_Pushed.item/*out*/);
		// 
	if(m_Pushed.group!=-1)
		if(m_Pushed.item==-1 || m_Groups[m_Pushed.group].items[m_Pushed.item].enable)   // clicking on a group or on an active item.
		{	m_ActivMode.ptStartDragPos = point;   // for ActivateModeDrag mode.
			o.SetCapture();
			o.InvalidateRect(&m_rcWork,FALSE);
		}
		else   // clicking on an inactive item.
			m_Pushed.group = m_Pushed.item = -1;
}
// 
void ToolBoxCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{	CWnd::OnLButtonUp(nFlags,point);
		// 
	if(p.m_Pushed.group!=-1)
	{	int group, item;
		p.HitTest(point,&group/*out*/,&item/*out*/);
		const bool clicked = (group==p.m_Pushed.group && item==p.m_Pushed.item);
			// 
		p.m_Pushed.group = p.m_Pushed.item = -1;   // !!! should be before ReleaseCapture().
		if( ::GetCapture()==m_hWnd )
			::ReleaseCapture();
			// 
		if(clicked)
			if(item==-1)   // clicking on a group.
			{	const bool expand = IsGroupExpanded(group);
				if(p.m_bOneGroupMode)
					CollapseAllGroups();
				(expand ? CollapseGroup(group) : ExpandGroup(group));
				p.Recalc();
			}
			else if(p.m_ActivMode.mode==ActivateModeClick)   // clicking on an item in the ActivateModeClick mode.
			{	const int id = p.m_Groups[group].items[item].id;
				p.m_pNotifyManager->OnClickItem(this,group,item,id);
			}
			// 
		InvalidateRect(&p.m_rcWork,FALSE);
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::OnMouseMove(UINT nFlags, CPoint point)
{	CWnd::OnMouseMove(nFlags, point);
		// 
	p.MouseMove(point);
}
// 
void ToolBoxCtrl::Private::MouseMove(CPoint point)
{	int group, item;
	HitTest(point,&group/*out*/,&item/*out*/);
		// 
	if(group!=m_Over.group || item!=m_Over.item)
	{	m_Over.group = group;
		m_Over.item = item;
			// 
		const bool showItemDesc = (group!=-1 && item!=-1 && !m_Groups[group].items[item].desc.IsEmpty() && (m_ActivMode.mode==ActivateModeClick || m_Pushed.item==-1));
		m_pNotifyManager->OnShowItemDescription(&o,(showItemDesc ? m_Groups[group].items[item].desc.GetString() : nullptr));
			// 
		o.InvalidateRect(&m_rcWork,FALSE);
	}
		// 
	if(m_ActivMode.mode==ActivateModeDrag && ::GetCapture()==o.m_hWnd && m_Pushed.item!=-1)   // moving the pressed item in the ActivateModeDrag mode, but there was no call to OnDragItemStart yet (we still have the mouse focus).
	{	const SIZE startDragZone = { ::GetSystemMetrics(SM_CXDRAG), ::GetSystemMetrics(SM_CYDRAG) };
			// 
		if(abs(point.x-m_ActivMode.ptStartDragPos.x) > startDragZone.cx || 
			abs(point.y-m_ActivMode.ptStartDragPos.y) > startDragZone.cy)
		{
			const Unit pushed = m_Pushed;   // save m_Pushed, and then restore it, because when you call ReleaseCapture(), OnCaptureChanged is called, in which m_Pushed is reset.
			::ReleaseCapture();
			m_Pushed = pushed;
				// 
			const int id = m_Groups[m_Pushed.group].items[m_Pushed.item].id;
			if( !m_pNotifyManager->OnDragItemStart(&o,m_Pushed.group,m_Pushed.item,id) )
				o.CancelDragMode();
		}
	}
		// 
	TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT),TME_LEAVE,o.m_hWnd,0 };
	::TrackMouseEvent(&tme);
}
/////////////////////////////////////////////////////////////////////////////
// 
LRESULT ToolBoxCtrl::OnMouseLeave(WPARAM wp, LPARAM lp)
{	if(p.m_Over.group!=-1)
	{	p.m_Over.group = p.m_Over.item = -1;
		InvalidateRect(&p.m_rcWork,FALSE);
	}
	p.m_pNotifyManager->OnShowItemDescription(this,nullptr);
		// 
	return CWnd::DefWindowProc(WM_MOUSELEAVE,wp,lp);
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::OnCaptureChanged(CWnd *pWnd)
{	if(pWnd!=this)
		if(p.m_Pushed.group!=-1)
		{	p.m_Pushed.group = p.m_Pushed.item = -1;
			InvalidateRect(&p.m_rcWork,FALSE);
		}
		// 
	CWnd::OnCaptureChanged(pWnd);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{	p.m_pNotifyManager->OnContextMenu(this,point);
		// 
	CWnd::OnRButtonUp(nFlags,point);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
BOOL ToolBoxCtrl::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
#ifdef _AFX_USING_CONTROL_BARS
	if( CMFCPopupMenu::GetActiveMenu()==nullptr )
#endif
	{	CPoint pt;
		::GetCursorPos(&pt/*out*/);
		ScreenToClient(&pt);
			// 
		int group, item;
		p.HitTest(pt,&group/*out*/,&item/*out*/);
			// 
		if(group!=-1 && item==-1)
			if(p.m_hGroupCursor)
			{	::SetCursor(p.m_hGroupCursor);
				return TRUE;
			}
	}
		// 
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int ToolBoxCtrl::Private::GetFullHeight() const
{	int height = 0;
		// 
	for(Private::ic_groups i=m_Groups.begin(), e=m_Groups.end(); i!=e; ++i)
	{	height += m_iGroupHeight;
		if(i->expand) 
			height += static_cast<int>( i->items.size() ) * m_iItemHeight;
	}
	return height;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::Private::GetTopVisible(int *group/*out*/, int *item/*out*/, int *deltaY/*out*/) const
{	int curPosY = 0;
	for(int g=0, gc=o.GetNumberGroups(); g<gc; ++g)
	{	curPosY += m_iGroupHeight;
		if(curPosY > m_iCurPosY)
		{	*group = g;
			*item = -1;
			*deltaY = curPosY-m_iGroupHeight - m_iCurPosY;
			return;
		}
			// 
		if( o.IsGroupExpanded(g) )
		{	const int childrenNum = o.GetNumberItems(g);
			const int newCurPos = curPosY + childrenNum * m_iItemHeight;
				// 
			if(newCurPos <= m_iCurPosY)
				curPosY = newCurPos;
			else
				for(int i=0; i<childrenNum; ++i)
				{	curPosY += m_iItemHeight;
					if(curPosY > m_iCurPosY)
					{	*group = g;
						*item = i;
						*deltaY = curPosY-m_iItemHeight - m_iCurPosY;
						return;
					}
				}
		}
	}
		// 
	*group = -1;
	*item = -1;
	*deltaY = 0;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::Private::HitTest(CPoint const &pt, int *groupOut/*out*/, int *itemOut/*out*/) const
{	if( m_rcWork.PtInRect(pt) )
	{	int group,item, deltaY;
		GetTopVisible(&group/*out*/,&item/*out*/, &deltaY/*out*/);
			// 
		if(group!=-1)
			for(int y=m_rcWork.top+deltaY; y<m_rcWork.bottom; )
				if(item==-1)   // group drawing.
				{	if(pt.y>=y && pt.y<y+m_iGroupHeight)
					{	*groupOut = group;
						*itemOut = item;
						return;
					}
						// 
					if(o.GetNumberItems(group)>0 && o.IsGroupExpanded(group))
						item = 0;
					else if(++group==o.GetNumberGroups())
						break;
					y += m_iGroupHeight;
				}
				else   // item drawing.
				{	if(pt.y>=y && pt.y<y+m_iItemHeight)
					{	*groupOut = group;
						*itemOut = item;
						return;
					}
						// 
					if(++item==o.GetNumberItems(group))
					{	if(++group==o.GetNumberGroups())
							break;
						item = -1;
					}
					y += m_iItemHeight;
				}
	}
		// 
	*groupOut = *itemOut = -1;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool ToolBoxCtrl::LoadState(CWinApp *app, TCHAR const *section, TCHAR const *entry)
{	assert(app && section && entry);
		//
	bool res = false;
	BYTE *data = nullptr;
	UINT dataSize;
		// 
	try
	{	if( app->GetProfileBinary(section,entry,&data,&dataSize) )
		{	CMemFile file(data,dataSize);
			CArchive ar(&file,CArchive::load);
			res = LoadState(&ar);
		}
	}
	catch(CMemoryException* pEx)
	{	pEx->Delete();
	}
	if(data)
		delete [] data;
		// 
	return res;
}
// 
bool ToolBoxCtrl::SaveState(CWinApp *app, TCHAR const *section, TCHAR const *entry) const
{	assert(app && section && entry);
		// 
	CMemFile file;
	CArchive ar(&file,CArchive::store);
	if( !SaveState(&ar) )
		return false;
	ar.Flush();
	ar.Close();
		// 
	const UINT dataSize = static_cast<UINT>( file.GetLength() );
	BYTE *data = file.Detach();
	const bool res = app->WriteProfileBinary(section,entry,data,dataSize)!=0;
	free(data);
	return res;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool ToolBoxCtrl::LoadState(CArchive *ar)
{	try
	{	return p.LoadStateInner(ar);
	}
	catch(CMemoryException* pEx)
	{	pEx->Delete();
	}
	catch(CArchiveException* pEx)
	{	pEx->Delete();
	}
	catch(...)
	{
	}
	return false;
}
// 
bool ToolBoxCtrl::SaveState(CArchive *ar) const
{	try
	{	return p.SaveStateInner(ar);
	}
	catch(CMemoryException* pEx)
	{	pEx->Delete();
	}
	catch(CArchiveException* pEx)
	{	pEx->Delete();
	}
	catch(...)
	{
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool ToolBoxCtrl::Private::LoadStateInner(CArchive *ar)
{	int numGroup;
	*ar >> numGroup;
		// 
	assert(numGroup==o.GetNumberGroups());
		// 
	for(Private::i_groups i=m_Groups.begin(), e=m_Groups.end(); i!=e; ++i)
		*ar >> i->expand;
		// 
	return true;
}
// 
bool ToolBoxCtrl::Private::SaveStateInner(CArchive *ar) const
{	*ar << o.GetNumberGroups();
		// 
	for(Private::ic_groups i=m_Groups.begin(), e=m_Groups.end(); i!=e; ++i)
		*ar << i->expand;
		// 
	return true;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int ToolBoxCtrl::Private::GetFontHeight(CFont *font) const
{	CWindowDC dc(nullptr);
		// 
	CFont *oldFont = dc.SelectObject(font);
	const int height = dc.GetTextExtent(_T("H"),1).cy;
	dc.SelectObject(oldFont);
	return height;
}
/////////////////////////////////////////////////////////////////////////////
// 
CSize ToolBoxCtrl::Private::GetTextSize(CFont *font, CString const &text) const
{	CWindowDC dc(nullptr);
		// 
	CFont *oldFont = dc.SelectObject(font);
	const CSize size = dc.GetTextExtent(text,text.GetLength());
	dc.SelectObject(oldFont);
	return size;
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrl::Private::SetScrollRange(CScrollBar *scroll, int min, int max, bool redraw) const
{	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.fMask = SIF_RANGE;
	info.nMin = min;
	info.nMax = max;
	scroll->SetScrollInfo(&info,redraw);
}
// 
void ToolBoxCtrl::Private::SetScrollSize(CScrollBar *scroll, int size, bool redraw) const
{	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.fMask = SIF_PAGE;
	info.nPage = static_cast<UINT>(size);
	scroll->SetScrollInfo(&info,redraw);
}
// 
void ToolBoxCtrl::Private::SetScrollPos(CScrollBar *scroll, int pos, bool redraw) const
{	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.fMask = SIF_POS;
	info.nPos = pos;
	scroll->SetScrollInfo(&info,redraw);
}
int ToolBoxCtrl::Private::GetScrollPos(CScrollBar *scroll) const
{	SCROLLINFO info;
	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = SIF_TRACKPOS;
	return (scroll->GetScrollInfo(&info,SIF_TRACKPOS) ? info.nTrackPos : 0);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool ToolBoxCtrl::Private::LoadImage(HMODULE moduleRes/*or null*/, UINT resID, bool pngImage, Gdiplus::Bitmap **bmp/*out*/) const
{	assert(resID);
	assert(bmp);
		// 
	*bmp = nullptr;
		// 
	if(!moduleRes)
		moduleRes = AfxFindResourceHandle(MAKEINTRESOURCE(resID),(pngImage ? _T("PNG") : RT_BITMAP));
	if(moduleRes)
	{	if(!pngImage)   // bmp.
			*bmp = ::new (std::nothrow) Gdiplus::Bitmap(moduleRes,MAKEINTRESOURCEW(resID));
		else   // png.
		{	HRSRC rsrc = ::FindResource(moduleRes,MAKEINTRESOURCE(resID),_T("PNG"));
			if(rsrc)
			{	HGLOBAL rsrcMem = ::LoadResource(moduleRes,rsrc);
				if(rsrcMem)
				{	const void *rsrcBuffer = ::LockResource(rsrcMem);
					if(rsrcBuffer)
					{	const UINT rsrcSize = static_cast<UINT>( ::SizeofResource(moduleRes,rsrc) );
						HGLOBAL streamMem = ::GlobalAlloc(GMEM_MOVEABLE,rsrcSize);
						if(streamMem)
						{	void *streamBuffer = ::GlobalLock(streamMem);
							if(streamBuffer)
							{	memcpy(streamBuffer,rsrcBuffer,rsrcSize);
								::GlobalUnlock(streamBuffer);
									// 
								IStream *stream = nullptr;
								if(::CreateStreamOnHGlobal(streamMem,FALSE,&stream/*out*/)==S_OK)
								{	*bmp = ::new (std::nothrow) Gdiplus::Bitmap(stream,FALSE);
									stream->Release();
								}
							}
							::GlobalFree(streamMem);
						}
						::UnlockResource(rsrcMem);
					}
					::FreeResource(rsrcMem);
				}
			}
		}
	}
	if(*bmp && (*bmp)->GetLastStatus()!=Gdiplus::Ok)
	{	::delete *bmp;
		*bmp = nullptr;
		return false;
	}
	return (*bmp)!=nullptr;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool ToolBoxCtrl::Private::CreateImageList(Gdiplus::Bitmap *bmp, int imageWidth, 
	COLORREF clrMask/*or CLR_NONE*/, COLORREF clrBack/*or CLR_NONE*/, CImageList *imageList/*out*/) const
{
	assert(bmp);
	assert(imageWidth>0);
	assert(imageList);
		// 
	bool res = false;
	const Gdiplus::Rect rect(0,0,bmp->GetWidth(),bmp->GetHeight());
	if( imageList->Create(imageWidth,bmp->GetHeight(),ILC_COLOR24 | ILC_MASK,1,0) )
	{	Gdiplus::BitmapData bmpData;
		if(bmp->LockBits(&rect,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,&bmpData/*out*/)==Gdiplus::Ok)
		{	const int sizeBuffer = abs(bmpData.Stride) * static_cast<int>(bmpData.Height);
			char *buffer = ::new (std::nothrow) char[sizeBuffer];
			if(buffer)
			{	memcpy(buffer,bmpData.Scan0,sizeBuffer);
					// 
				CBitmap imageListBitmap;
				if( imageListBitmap.CreateBitmap(rect.Width,rect.Height,1,32,nullptr) )
				{	const UINT maskRGB = (clrMask & 0x0000ff00) | (clrMask & 0xff)<<16 | (clrMask & 0x00ff0000)>>16;
					const UINT pixelNumber = bmpData.Width * bmpData.Height;
					UINT32 *ptr = reinterpret_cast<UINT32 *>(buffer);
					for(UINT32 *e=ptr+pixelNumber; ptr!=e; ++ptr)
					{	const unsigned char a = static_cast<unsigned char>(*ptr >> 24);
						if(a==0)
							*ptr = maskRGB;
						else if(a==255)
						{	if(clrMask!=CLR_NONE)
								if((*ptr & 0x00ffffff)==maskRGB)
									*ptr = maskRGB;
						}
						else   // a!=255.
							if(clrBack!=CLR_NONE)
							{	const UINT _a = 255u - a;
								const UINT r = ((*ptr & 0xff) * a + (clrBack>>16 & 0xff) * _a) / 255u;
								const UINT g = ((*ptr>>8 & 0xff) * a + (clrBack>>8 & 0xff) * _a) / 255u;
								const UINT b = ((*ptr>>16 & 0xff) * a + (clrBack & 0xff) * _a) / 255u;
								*ptr = r | (g<<8) | (b<<16);
							}
					}
					imageListBitmap.SetBitmapBits(pixelNumber*4,buffer);
						// 
					res = imageList->Add(&imageListBitmap,clrMask & 0x00ffffff)!=-1;
				}
				::delete [] buffer;
			}
			bmp->UnlockBits(&bmpData);
		}
	}
	if(!res && imageList->m_hImageList)
		imageList->DeleteImageList();
	return res;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// ToolBoxCtrlStyle_base.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_base::Install(ToolBoxCtrl *ctrl)
{	ctrl->SetDrawManager(this);
	ctrl->SetToolTipManager(this);
	ctrl->SetRecalcManager(nullptr);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
CToolTipCtrl *ToolBoxCtrlStyle_base::CreateToolTip(ToolBoxCtrl *ctrl)
{
	#ifdef AFX_TOOLTIP_TYPE_ALL   // for MFC Feature Pack.
		CToolTipCtrl *tooltip = nullptr;
		return (CTooltipManager::CreateToolTip(tooltip/*out*/,ctrl,AFX_TOOLTIP_TYPE_TAB) ? tooltip : nullptr);
	#else
		CToolTipCtrl *toolTip = nullptr;
		try
		{	toolTip = new CToolTipCtrl;
		}
		catch(std::bad_alloc &)
		{	return nullptr;
		}
		if( !toolTip->Create(ctrl,TTS_ALWAYSTIP | TTS_NOPREFIX) )
		{	delete toolTip;
			return nullptr;
		}
			// 
		DWORD dwClassStyle = ::GetClassLong(toolTip->m_hWnd,GCL_STYLE);
		dwClassStyle |= 0x00020000/*CS_DROPSHADOW*/;   // enables the drop shadow effect.
		::SetClassLong(toolTip->m_hWnd,GCL_STYLE,dwClassStyle);
		return toolTip;
	#endif
}
// 
void ToolBoxCtrlStyle_base::DestroyToolTip(CToolTipCtrl *tooltip)
{	
	#ifdef AFX_TOOLTIP_TYPE_ALL   // for MFC Feature Pack.
		CTooltipManager::DeleteToolTip(tooltip);
	#else
		delete tooltip;
	#endif
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_base::DrawBorder(ToolBoxCtrl const * /*ctrl*/, CDC *dc, CRect const *rect)
{	DrawRect(dc,rect, GetBorderColor() );
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_base::DrawWorkAreaBack(ToolBoxCtrl const * /*ctrl*/, CDC *dc, CRect const *rect)
{	dc->FillSolidRect(rect, GetBackgroundColor() );
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_base::DrawGroup(ToolBoxCtrl const *ctrl, CDC *dc, int group, CRect const *rect)
{		// drawing the background.
	DrawGroupBack(ctrl,dc,rect);
		// 
	CRect rc = rect;
	const CRect rcGroupPadding = ctrl->GetGroupPadding();
	rc.DeflateRect(&rcGroupPadding);
		// drawing the collapsed label.
	Gdiplus::Bitmap *images = ctrl->GetImages(ToolBoxCtrl::ImageGroup);
	if(images)
	{	const CSize szImage = ctrl->GetImageSize(ToolBoxCtrl::ImageGroup);
		const CPoint pt(rc.left,(rc.top+rc.bottom-szImage.cy+(szImage.cy % 2))/2);
		const int img = static_cast<int>( ctrl->IsGroupExpanded(group) );
		DrawImageNormal(dc,images,pt, img, szImage, ctrl->GetImageTranspColor(ToolBoxCtrl::ImageGroup) );
		rc.left += szImage.cx + ctrl->GetGroupImageTextSpace();
	}
		// drawing text.
	COLORREF clrFore = ctrl->GetGroupForeColor(group);
	if(clrFore==CLR_NONE)
		clrFore = GetGroupTextColor();
	dc->SetTextColor(clrFore);
		// 
	CFont *oldFont = dc->SelectObject( ctrl->GetFont(ToolBoxCtrl::FontGroup) );
	DrawText(dc, ctrl->GetGroupText(group), rc);
	dc->SelectObject(oldFont);
}
/////////////////////////////////////////////////////////////////////////////
//
void ToolBoxCtrlStyle_base::DrawGroupBack(ToolBoxCtrl const * /*ctrl*/, CDC *dc, CRect rect)
{	CRect rc = rect;
	rc.DeflateRect(1,3);
	const COLORREF clrTop = GetGroupGradientTopColor();
	const COLORREF clrBottom = GetGroupGradientBottomColor();
	DrawGradient(dc,&rc,false,clrTop, clrBottom);
		// gradient stripe under the background.
	rc.top = rect.bottom-2;
	rc.bottom = rc.top + 1;
	DrawGradient(dc,&rc,true,clrBottom, GetBackgroundColor() );
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_base::DrawItem(ToolBoxCtrl const *ctrl, CDC *dc, int group, int item, CRect const *rect)
{	const int image = ctrl->GetItemImage(group,item);
	const ToolBoxCtrl::Font font = ctrl->GetItemFont(group,item);
	const bool enable = ctrl->IsItemEnabled(group,item);
	const bool checked = ctrl->IsItemChecked(group,item);
	const COLORREF clrBack = ctrl->GetItemBackColor(group,item);
		// 
		// drawing selection.
	DrawItemSelection(ctrl,dc, group,item, rect, enable,checked, clrBack);
		// 
	CRect rc = rect;
	const CRect rcItemPadding = ctrl->GetItemPadding();
	rc.DeflateRect(&rcItemPadding);
	DrawItemContent(ctrl, dc, group,item, rc, image, font, enable,checked);
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_base::DrawItemSelection(ToolBoxCtrl const *ctrl, CDC *dc, int group, int item, CRect rect, 
	bool enable, bool checked, COLORREF clrBack)
{
	if(enable)   // item is available.
	{	struct Item { int group, item; } itemUnderCursor, itemPushed;
		ctrl->GetItemUnderCursor(&itemUnderCursor.group/*out*/,&itemUnderCursor.item/*out*/);
		ctrl->GetItemPushed(&itemPushed.group/*out*/,&itemPushed.item/*out*/);
			// 
		const bool over = (itemUnderCursor.group==group && itemUnderCursor.item==item);
		const bool pushed = (itemPushed.group==group && itemPushed.item==item);
			// 
		if((itemPushed.group==-1 && checked!=over) || (itemPushed.group!=-1 && !pushed && checked))   // light highlight.
		{	dc->FillSolidRect(&rect, GetEnableSelectLightBackColor() );
			DrawRect(dc,&rect, GetEnableSelectLightBorderColor() );
		}
		else if(pushed || (itemPushed.group==-1 && checked && over))   // dark highlight.
		{	dc->FillSolidRect(&rect, GetEnableSelectDarkBackColor() );
			DrawRect(dc,&rect, GetEnableSelectDarkBorderColor()  );
		}
		else if(clrBack!=CLR_NONE)   // the item is available and unchecked.
			dc->FillSolidRect(&rect,clrBack);
	}
	else if(checked)   // the item is not available and checked.
	{	dc->FillSolidRect(&rect, GetDisableBackColor() );
		DrawRect(dc,&rect, GetDisableBorderColor()  );
	}
	else if(clrBack!=CLR_NONE)   // the item is not available or unchecked.
		dc->FillSolidRect(&rect,clrBack);
}
//
void ToolBoxCtrlStyle_base::DrawItemContent(ToolBoxCtrl const *ctrl, CDC *dc, int group, int item, CRect rect, int image, ToolBoxCtrl::Font font, bool enable, bool /*checked*/)
{		// drawing a picture.
	if(image>=0)
	{	Gdiplus::Bitmap *imageNorm = ctrl->GetImages(ToolBoxCtrl::ImageItem);
		const CSize szNormal = ctrl->GetImageSize(ToolBoxCtrl::ImageItem);
			// 
		if(imageNorm && image*szNormal.cx<=static_cast<long>( imageNorm->GetWidth()-szNormal.cx ))
		{	const CPoint pt(rect.left,(rect.top+rect.bottom-szNormal.cy+(szNormal.cy % 2))/2);
			const COLORREF clrTrans = ctrl->GetImageTranspColor(ToolBoxCtrl::ImageItem);
			(enable ?
				DrawImageNormal(dc,imageNorm,pt, image, szNormal, clrTrans) :
				DrawImageDisable(dc,imageNorm, pt, image, szNormal, clrTrans, ctrl->GetDisabledItemImageType(), 140));
			rect.left += szNormal.cx + ctrl->GetItemImageTextSpace();
		}
	}
		// drawing text.
	COLORREF clrFore = ctrl->GetItemForeColor(group,item);
	if(clrFore==CLR_NONE)
		clrFore = GetItemTextColor();
	if(!enable)
		 clrFore = MixingColors(clrFore,GetBackgroundColor(),60);
	dc->SetTextColor(clrFore);
		// 
	CFont *fontExtra = (font==ToolBoxCtrl::FontItemExtra ? ctrl->GetFont(ToolBoxCtrl::FontItemExtra) : NULL);
	CFont *oldFont = dc->SelectObject(fontExtra ? fontExtra : ctrl->GetFont(ToolBoxCtrl::FontItemBasic));
	DrawText(dc, ctrl->GetItemText(group,item), rect);
	dc->SelectObject(oldFont);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_base::DrawRect(CDC *dc, CRect const *rect, COLORREF clr) const
{	CPen pen(PS_SOLID,1,clr);
	CPen *pOldPen = dc->SelectObject(&pen);
	dc->MoveTo(rect->left,rect->top);
	dc->LineTo(rect->left,rect->bottom-1);
	dc->LineTo(rect->right-1,rect->bottom-1);
	dc->LineTo(rect->right-1,rect->top);
	dc->LineTo(rect->left,rect->top);
	dc->SelectObject(pOldPen);
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_base::DrawGradient(CDC *pDC, CRect const *pRect, bool horz, COLORREF clrTopLeft, COLORREF clrBottomRight) const
{	GRADIENT_RECT gRect = {0,1};
	TRIVERTEX vert[2] = 
	{	{pRect->left,pRect->top,static_cast<COLOR16>((GetRValue(clrTopLeft) << 8)),static_cast<COLOR16>((GetGValue(clrTopLeft) << 8)),static_cast<COLOR16>((GetBValue(clrTopLeft) << 8)),0},
		{pRect->right,pRect->bottom,static_cast<COLOR16>((GetRValue(clrBottomRight) << 8)),static_cast<COLOR16>((GetGValue(clrBottomRight) << 8)),static_cast<COLOR16>((GetBValue(clrBottomRight) << 8)),0}
	};
	::GradientFill(pDC->m_hDC,vert,2,&gRect,1,(horz ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V));
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_base::DrawImageNormal(CDC *dc, Gdiplus::Bitmap *bmp, CPoint const &ptDst, int image, CSize const &szSrc, COLORREF clrTransp) const 
{	Gdiplus::Graphics gr(dc->m_hDC);
	if(clrTransp==CLR_NONE)
		gr.DrawImage(bmp, ptDst.x,ptDst.y, image*szSrc.cx,0,szSrc.cx,szSrc.cy, Gdiplus::UnitPixel);
	else   // draw with color key.
	{	Gdiplus::ImageAttributes att;
		const Gdiplus::Color clrTr(GetRValue(clrTransp),GetGValue(clrTransp),GetBValue(clrTransp));
		att.SetColorKey(clrTr,clrTr,Gdiplus::ColorAdjustTypeBitmap);
		const Gdiplus::Rect rcDest(ptDst.x,ptDst.y,szSrc.cx,szSrc.cy);
		gr.DrawImage(bmp, rcDest, image*szSrc.cx,0,szSrc.cx,szSrc.cy, Gdiplus::UnitPixel, &att);
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_base::DrawImageDisable(CDC *dc, Gdiplus::Bitmap *bmp, CPoint const &ptDst, int image, CSize const &szSrc, 
	COLORREF clrTransp, ToolBoxCtrl::DisabledItemImage type, unsigned char alpha) const
{
	Gdiplus::ImageAttributes att;
	if(type==ToolBoxCtrl::DisabledItemImageTranslucent)
	{	static Gdiplus::ColorMatrix matrix = 
		{	1.0f,0.0f,0.0f,0.0f,0.0f,
			0.0f,1.0f,0.0f,0.0f,0.0f,
			0.0f,0.0f,1.0f,0.0f,0.0f,
			0.0f,0.0f,0.0f,1.0f,0.0f,
			0.0f,0.0f,0.0f,0.0f,1.0f
		};
		matrix.m[3][3] = static_cast<float>(alpha) / 255.0f;
	   att.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
	}
	else   // ToolBoxCtrl::DisabledItemImageGrayscale.
	{	static Gdiplus::ColorMatrix matrix =
		{	0.3f,0.3f,0.3f,0.0f,0.0f,
			0.6f,0.6f,0.6f,0.0f,0.0f,
			0.1f,0.1f,0.1f,0.0f,0.0f,
			0.0f,0.0f,0.0f,1.0f,0.0f,
			0.0f,0.0f,0.0f,0.0f,1.0f
		};
		matrix.m[3][3] = static_cast<float>(alpha) / 255.0f;
	   att.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
	}
		// 
	if(clrTransp!=CLR_NONE)
	{	const Gdiplus::Color clrTr(GetRValue(clrTransp),GetGValue(clrTransp),GetBValue(clrTransp));
		att.SetColorKey(clrTr,clrTr,Gdiplus::ColorAdjustTypeBitmap);
	}
		// 
	Gdiplus::Graphics gr(dc->m_hDC);
	const Gdiplus::Rect rcDest(ptDst.x,ptDst.y,szSrc.cx,szSrc.cy);
	gr.DrawImage(bmp,rcDest,image*szSrc.cx,0,szSrc.cx,szSrc.cy,Gdiplus::UnitPixel,&att,nullptr,nullptr);
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_base::DrawText(CDC *dc, CString const &text, CRect const &rect) const
{	CString textWithEllipsis;
	(ExpandStringEllipsis(dc,text,rect.Width(),&textWithEllipsis/*out*/) ? 
		dc->DrawText(textWithEllipsis,const_cast<CRect *>(&rect),DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX) : 
		dc->DrawText(text,const_cast<CRect *>(&rect),DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX));
}
/////////////////////////////////////////////////////////////////////////////
// 
bool ToolBoxCtrlStyle_base::ExpandStringEllipsis(CDC *dc, CString const &strSrc, int maxWidth, CString *strDst/*out*/) const
{	const int strSrcLength = strSrc.GetLength();
		// 
	int number;
	SIZE sz;
	if( ::GetTextExtentExPoint(dc->m_hDC,strSrc,strSrcLength,maxWidth,&number,nullptr,&sz) )
		if(number < strSrcLength)
		{	if(number>0)
			{	const int pointsWidth = dc->GetTextExtent(_T("Е")).cx;
				if( !::GetTextExtentExPoint(dc->m_hDC,strSrc,strSrcLength,std::max(0,maxWidth-pointsWidth),&number,nullptr,&sz) )
					return false;
				*strDst = strSrc.Left(number) + _T("Е");
			}
			else
				*strDst = _T("Е");
			return true;
		}
		// 
	return false;
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF ToolBoxCtrlStyle_base::MixingColors(COLORREF src, COLORREF dst, int percent) const
{	const int ipercent = 100 - percent;
	return RGB(
		(GetRValue(src) * percent + GetRValue(dst) * ipercent) / 100,
		(GetGValue(src) * percent + GetGValue(dst) * ipercent) / 100,
		(GetBValue(src) * percent + GetBValue(dst) * ipercent) / 100);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// ToolBoxCtrlStyle_classic.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_classic::Install(ToolBoxCtrl *ctrl)
{	ctrl->SetDrawManager(this);
	ctrl->SetToolTipManager(this);
	ctrl->SetRecalcManager(this);
}
/////////////////////////////////////////////////////////////////////////////
// 
int ToolBoxCtrlStyle_classic::GetBorderWidth(ToolBoxCtrl const *ctrl, IRecalc *base) { return base->GetBorderWidth(ctrl,base); }
CRect ToolBoxCtrlStyle_classic::GetGroupPadding(ToolBoxCtrl const * /*ctrl*/, IRecalc * /*base*/) { return CRect(3,3,2,3); }
int ToolBoxCtrlStyle_classic::GetGroupImageTextSpace(ToolBoxCtrl const *ctrl, IRecalc *base) { return base->GetGroupImageTextSpace(ctrl,base); }
CRect ToolBoxCtrlStyle_classic::GetItemPadding(ToolBoxCtrl const * /*ctrl*/, IRecalc * /*base*/) { return CRect(10,3,3,2); }
int ToolBoxCtrlStyle_classic::GetItemImageTextSpace(ToolBoxCtrl const *ctrl, IRecalc *base) { return base->GetItemImageTextSpace(ctrl,base); }
// 
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_classic::DrawGroupBack(ToolBoxCtrl const * /*ctrl*/, CDC *dc, CRect rect)
{	rect.DeflateRect(1,1,1,0);
	dc->FillSolidRect(&rect, GetGroupGradientTopColor() );
}
/////////////////////////////////////////////////////////////////////////////
// 
void ToolBoxCtrlStyle_classic::DrawItemSelection(ToolBoxCtrl const *ctrl, CDC *dc, int group, int item, CRect rect, 
	bool enable, bool checked, COLORREF clrBack)
{
	rect.DeflateRect(1,1,1,0);
		// 
	if(enable)   // item is available.
	{	struct Item { int group, item; } itemUnderCursor, itemPushed;
		ctrl->GetItemUnderCursor(&itemUnderCursor.group/*out*/,&itemUnderCursor.item/*out*/);
		ctrl->GetItemPushed(&itemPushed.group/*out*/,&itemPushed.item/*out*/);
			// 
		const bool over = (itemUnderCursor.group==group && itemUnderCursor.item==item);
		const bool pushed = (itemPushed.group==group && itemPushed.item==item);
			// 
		if((itemPushed.group==-1 && checked!=over) || (itemPushed.group!=-1 && !pushed && checked))   // light highlight.
			dc->FillSolidRect(&rect, GetEnableSelectLightBackColor() );
		else if(pushed || (itemPushed.group==-1 && checked && over))   // dark highlight.
			dc->FillSolidRect(&rect, GetEnableSelectDarkBackColor() );
		else if(clrBack!=CLR_NONE)   // the item is available and unchecked.
			dc->FillSolidRect(&rect,clrBack);
	}
	else if(checked)   // the item is not available and checked.
	{	dc->FillSolidRect(&rect, GetDisableBackColor() );
		DrawRect(dc,&rect, GetDisableBorderColor()  );
	}
	else if(clrBack!=CLR_NONE)   // the item is not available or unchecked.
		dc->FillSolidRect(&rect,clrBack);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

















