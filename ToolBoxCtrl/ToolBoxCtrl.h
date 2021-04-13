//==========================================================
// Author: Baradzenka Aleh (baradzenka@gmail.com)
//==========================================================
// 
#pragma once
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable : 4458)   // declaration of 'nativeCap' hides class member.
#include <gdiplus.h>
#pragma warning(pop)
// 
#if (!defined(_MSC_VER) && __cplusplus < 201103L) || (defined(_MSC_VER) && _MSC_VER < 1900)   // C++11 is not supported.
	#define nullptr  NULL
	#define override
#endif
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
class ToolBoxCtrl : public CWnd
{	DECLARE_DYNCREATE(ToolBoxCtrl)

///////////////////////////////////////
// PUBLIC
///////////////////////////////////////
public:
	struct Draw
	{	virtual void DrawBegin(ToolBoxCtrl const * /*ctrl*/, CDC * /*dc*/) {}
		virtual void DrawBorder(ToolBoxCtrl const * /*ctrl*/, CDC * /*dc*/, CRect const * /*rect*/) {}
		virtual void DrawWorkAreaBack(ToolBoxCtrl const * /*ctrl*/, CDC * /*dc*/, CRect const * /*rect*/) {}
		virtual void DrawGroup(ToolBoxCtrl const * /*ctrl*/, CDC * /*dc*/, int /*group*/, CRect const * /*rect*/) {}
		virtual void DrawItem(ToolBoxCtrl const * /*ctrl*/, CDC * /*dc*/, int /*group*/, int /*item*/, CRect const * /*rect*/) {}
		virtual void DrawEnd(ToolBoxCtrl const * /*ctrl*/, CDC * /*dc*/) {}
	};
	interface IRecalc
	{	virtual int GetBorderWidth(ToolBoxCtrl const *ctrl, IRecalc *base) = 0;
		virtual CRect GetGroupPadding(ToolBoxCtrl const *ctrl, IRecalc *base) = 0;
		virtual int GetGroupImageTextSpace(ToolBoxCtrl const *ctrl, IRecalc *base) = 0;   // space between image and text in a group.
		virtual CRect GetItemPadding(ToolBoxCtrl const *ctrl, IRecalc *base) = 0;
		virtual int GetItemImageTextSpace(ToolBoxCtrl const *ctrl, IRecalc *base) = 0;   // space between image and text in an intem.
	};
	struct ToolTip
	{	virtual CToolTipCtrl *CreateToolTip(ToolBoxCtrl * /*ctrl*/) { return nullptr; }
		virtual void DestroyToolTip(CToolTipCtrl * /*tooltip*/) {}
	};
	struct Notify
	{	virtual void OnShowItemDescription(ToolBoxCtrl * /*ctrl*/, TCHAR const * /*desc*//*or null*/) {}   // it is necessary to display the item description in the StatusBar.
		virtual void OnUpdateItemState(ToolBoxCtrl * /*ctrl*/, int /*group*/, int /*item*/, int /*id*/, CCmdUI * /*cmd*/) {}   // notification about the need to update the state of the item; {group==-1, item==-1, id==0} for UpdateModeCommon mode and one call for all visible items.
		virtual void OnClickItem(ToolBoxCtrl * /*ctrl*/, int /*group*/, int /*item*/, int /*id*/) {}   // clicking on an element in ActivateModeClick mode.
		virtual bool OnDragItemStart(ToolBoxCtrl * /*ctrl*/, int /*group*/, int /*item*/, int /*id*/) { return false; }   // notification to start moving the pressed item in ActivateModeDrag mode.
		virtual void OnContextMenu(ToolBoxCtrl * /*ctrl*/, CPoint /*point*/) {}
	};

public:
	ToolBoxCtrl();
	~ToolBoxCtrl();

public:
	bool Create(CWnd *parent, DWORD style, RECT const &rect, UINT id);
	void Update(bool recalc);
		// 
	void SetDrawManager(Draw *p/*or null*/);
	Draw *GetDrawManager() const;
	void SetRecalcManager(IRecalc *p/*or null*/);   // or null for default manager.
	IRecalc *GetRecalcManager() const;
	void SetToolTipManager(ToolTip *p/*or null*/);
	ToolTip *GetToolTipManager() const;
	void SetNotifyManager(Notify *p/*or null*/);
	Notify *GetNotifyManager() const;
		// 
	enum Image { ImageGroup, ImageItem };
	bool CreateImages(Image type, HMODULE moduleRes/*or null*/, UINT resID/*or 0*/, bool pngImage, int imageWidth, COLORREF clrTransp=CLR_NONE);
	Gdiplus::Bitmap *GetImages(Image type) const;
	bool GetImageList(Image type, COLORREF clrDstBack/*or CLR_NONE*/, CImageList *imageList/*out*/) const;
	CSize GetImageSize(Image type) const;
	COLORREF GetImageTranspColor(Image type) const;
		//
	bool SetGroupCursor(HMODULE module/*or null*/, UINT resID);
	bool SetGroupCursor(HCURSOR cursor);
	HCURSOR GetGroupCursor() const;
		// 
	enum Font { FontGroup, FontItemBasic, FontItemExtra };
	bool SetFont(Font type, CFont *font);
	void SetFontRef(Font type, CFont *font);
	bool SetFont(Font type, LOGFONT const *lf);
	CFont *GetFont(Font type) const;
		// 
	void AddGroup(TCHAR const *text, COLORREF clrFore=CLR_NONE);
	void AddItem(int group, int id, TCHAR const *text, TCHAR const *desc=nullptr, int image=-1, Font font=FontItemBasic, COLORREF clrBack=CLR_NONE, COLORREF clrFore=CLR_NONE);
	void DeleteGroup(int group);
	void DeleteItem(int group, int item);
	void DeleteAll();
		// 
	int GetNumberGroups() const;
	int GetNumberItems(int group) const;
		// 
	enum ActivateMode
	{	ActivateModeClick,   // when the clicked group item is released, a Notify::OnClickItem notification is sent.
		ActivateModeDrag   // when holding down the pressed element of the group and moving the mouse, a Notify::OnDragItemStart notification is sent.
	};
	void SetActivateMode(ActivateMode mode);
	ActivateMode GetActivateMode() const;
	void CancelDragMode();   // resetting the state of the control after finishing moving the element in the ActivateMode::ActivateModeDrag mode.
		// 
	void EnableDynamicItemsUpdate(bool enable);   // enables dynamic updating of the items' state, leads to a call to Notify::OnUpdateItemState.
	bool IsDynamicItemsUpdateEnable() const;
	enum UpdateMode
	{	UpdateModeIndividual,   // the state of the items in the control is requested separately for each visible item.
		UpdateModeCommon   // the state of all visible items in the control is requested once. 
	};
	void SetUpdateMode(UpdateMode mode);
	UpdateMode GetUpdateMode() const;
		//
	void SetGroupText(int group, TCHAR const *text);
	CString GetGroupText(int group) const;
	void SetGroupForeColor(int group, COLORREF color);   // 'color'=CLR_NONE to not use the foreground color.
	COLORREF GetGroupForeColor(int group) const;
		// 
	void CollapseGroup(int group);
	void ExpandGroup(int group);
	bool IsGroupExpanded(int group) const;
	void CollapseAllGroups();
	void ExpandAllGroups();
		// 
	void SetItemId(int group, int item, int id);
	int GetItemId(int group, int item) const;
	void SetItemText(int group, int item, TCHAR const *text);
	CString GetItemText(int group, int item) const;
	void SetItemDescription(int group, int item, TCHAR const *text);
	CString GetItemDescription(int group, int item) const;
	void SetItemImage(int group, int item, int image);   // 'image'=-1 for item without image.
	int GetItemImage(int group, int item) const;
	void SetItemFont(int group, int item, Font font);   // only Font::FontItemBasic or Font::FontItemExtra.
	Font GetItemFont(int group, int item) const;
	void SetItemBackColor(int group, int item, COLORREF color);   // 'color'=CLR_NONE to not use the background color.
	COLORREF GetItemBackColor(int group, int item) const;
	void SetItemForeColor(int group, int item, COLORREF color);   // 'color'=CLR_NONE to not use the foreground color.
	COLORREF GetItemForeColor(int group, int item) const;
	void EnableItem(int group, int item, bool enable);
	bool IsItemEnabled(int group, int item) const;
	void CheckItem(int group, int item, bool check);
	bool IsItemChecked(int group, int item) const;
	void SetItemData(int group, int item, __int64 data);   // set any user data for the item.
	__int64 GetItemData(int group, int item) const;
		// 
	void ShowBorder(bool show);   // border is visible if IsBorderVisible()==true and IRecalc::GetBorderWidth(...) returns >0.
	bool IsBorderVisible() const;
	void ShowScroll(bool show);
	bool IsScrollVisible() const;
	void EnableOneGroupMode(bool enable);
	bool IsOneGroupModeEnable() const;
	void EnableToolTip(bool enable);
	bool IsToolTipEnable() const;
		// 
	enum DisabledItemImage { DisabledItemImageTranslucent, DisabledItemImageGrayscale };
	void SetDisabledItemImageType(DisabledItemImage type);
	DisabledItemImage GetDisabledItemImageType() const;
		//
	void GetItemUnderCursor(int *group/*out,or null*/, int *item/*out,or null*/) const;
	void GetItemPushed(int *group/*out,or null*/, int *item/*out,or null*/) const;
		// 
	bool LoadState(CWinApp *app, TCHAR const *section, TCHAR const *entry);   // load state from registry.
	bool SaveState(CWinApp *app, TCHAR const *section, TCHAR const *entry) const;   // save state in registry.
	bool LoadState(CArchive *ar);
	bool SaveState(CArchive *ar) const;

public:   // functions of IRecalc interface, return information from current recalc manager.
	int GetBorderWidth() const;
	CRect GetGroupPadding() const;
	int GetGroupImageTextSpace() const;   // space between image and text in a group.
	CRect GetItemPadding() const;
	int GetItemImageTextSpace() const;   // space between image and text in an intem.

public:
	CToolTipCtrl *GetToolTip() const;   // get used tooltip object (null if tooltip wasn't created).

///////////////////////////////////////
// PRIVATE
///////////////////////////////////////
private:
	struct Private;
	Private &p;

protected:
	DECLARE_MESSAGE_MAP()
	BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD style, const RECT &rect, CWnd *parentWnd, UINT id, CCreateContext *context) override;
	afx_msg void OnDestroy();
	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	LRESULT OnMouseLeave(WPARAM wp, LPARAM lp);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
/////////////////////////////////////////////////////////////////////////////
// 
struct ToolBoxCtrlStyle_base : 
	ToolBoxCtrl::Draw,
	ToolBoxCtrl::ToolTip
{
	void Install(ToolBoxCtrl *ctrl);

		// ToolBoxCtrl::Draw.
	void DrawBorder(ToolBoxCtrl const *ctrl, CDC *dc, CRect const *rect) override;
	void DrawWorkAreaBack(ToolBoxCtrl const *ctrl, CDC *dc, CRect const *rect) override;
	void DrawGroup(ToolBoxCtrl const *ctrl, CDC *dc, int group, CRect const *rect) override;
	void DrawItem(ToolBoxCtrl const *ctrl, CDC *dc, int group, int item, CRect const *rect) override;

		// ToolBoxCtrl::ToolTip.
	CToolTipCtrl *CreateToolTip(ToolBoxCtrl *ctrl) override;
	void DestroyToolTip(CToolTipCtrl *tooltip) override;

	virtual void DrawGroupBack(ToolBoxCtrl const *ctrl, CDC *dc, CRect rect);
	virtual void DrawItemSelection(ToolBoxCtrl const *ctrl, CDC *dc, int group, int item, CRect rect, bool enable, bool checked, COLORREF clrBack);
	virtual void DrawItemContent(ToolBoxCtrl const *ctrl, CDC *dc, int group, int item, CRect rect, int image, ToolBoxCtrl::Font font, bool enable, bool checked);
		// 
	virtual COLORREF GetBorderColor() = 0;
	virtual COLORREF GetBackgroundColor() = 0;
	virtual COLORREF GetGroupTextColor() = 0;
	virtual COLORREF GetItemTextColor() = 0;
		// 
	virtual COLORREF GetGroupGradientTopColor() = 0;
	virtual COLORREF GetGroupGradientBottomColor() = 0;
		// 
	virtual COLORREF GetEnableSelectLightBackColor() = 0;
	virtual COLORREF GetEnableSelectLightBorderColor() = 0;
	virtual COLORREF GetEnableSelectDarkBackColor() = 0;
	virtual COLORREF GetEnableSelectDarkBorderColor() = 0;
	virtual COLORREF GetDisableBackColor() = 0;
	virtual COLORREF GetDisableBorderColor() = 0;

	void DrawRect(CDC *dc, CRect const *rect, COLORREF clr) const;
	void DrawGradient(CDC *pDC, CRect const *pRect, bool horz, COLORREF clrTopLeft, COLORREF clrBottomRight) const;
	void DrawImageNormal(CDC *dc, Gdiplus::Bitmap *bmp, CPoint const &ptDst, int image, CSize const &szSrc, COLORREF clrTransp) const;
	void DrawImageDisable(CDC *dc, Gdiplus::Bitmap *bmp, CPoint const &ptDst, int image, CSize const &szSrc, COLORREF clrTransp, 
		ToolBoxCtrl::DisabledItemImage type, unsigned char alpha) const;
	void DrawText(CDC *dc, CString const &text, CRect const &rect) const;
	bool ExpandStringEllipsis(CDC *dc, CString const &strSrc, int maxWidth, CString *strDst/*out*/) const;
	COLORREF MixingColors(COLORREF src, COLORREF dst, int percent) const;
};
/////////////////////////////////////////////////////////////////////////////
// 
struct ToolBoxCtrlStyle_classic : ToolBoxCtrlStyle_base, 
	ToolBoxCtrl::IRecalc
{
	void Install(ToolBoxCtrl *ctrl);

		// ToolBoxCtrlStyle_base.
	void DrawGroupBack(ToolBoxCtrl const *ctrl, CDC *dc, CRect rect) override;
	void DrawItemSelection(ToolBoxCtrl const *ctrl, CDC *dc, int group, int item, CRect rect, bool enable, bool checked, COLORREF clrBack) override;

		// IRecalc.
	int GetBorderWidth(ToolBoxCtrl const *ctrl, IRecalc *base) override;
	CRect GetGroupPadding(ToolBoxCtrl const *ctrl, IRecalc *base) override;
	int GetGroupImageTextSpace(ToolBoxCtrl const *ctrl, IRecalc *base) override;
	CRect GetItemPadding(ToolBoxCtrl const *ctrl, IRecalc *base) override;
	int GetItemImageTextSpace(ToolBoxCtrl const *ctrl, IRecalc *base) override;
};
/////////////////////////////////////////////////////////////////////////////
// 
template<typename BASE> struct ToolBoxCtrlStyle_system : BASE
{		// ToolBoxCtrlStyle_base.
	COLORREF GetBorderColor() override { return ::GetSysColor(COLOR_BTNSHADOW); }
	COLORREF GetBackgroundColor() override { return ::GetSysColor(COLOR_BTNFACE); }
	COLORREF GetGroupTextColor() override { return ::GetSysColor(COLOR_WINDOWTEXT); }
	COLORREF GetItemTextColor() override { return ::GetSysColor(COLOR_WINDOWTEXT); }
		// 
	COLORREF GetGroupGradientTopColor() override { return MixingColors(::GetSysColor(COLOR_BTNSHADOW),::GetSysColor(COLOR_BTNFACE),20); }
	COLORREF GetGroupGradientBottomColor() override { return MixingColors(::GetSysColor(COLOR_BTNSHADOW),::GetSysColor(COLOR_BTNFACE),50); }
		// 
	COLORREF GetEnableSelectLightBackColor() override { return MixingColors(::GetSysColor(COLOR_BTNSHADOW), MixingColors(::GetSysColor(COLOR_HIGHLIGHT),::GetSysColor(COLOR_WINDOW),22), 10); }
	COLORREF GetEnableSelectLightBorderColor() override { return ::GetSysColor(COLOR_HIGHLIGHT); }
	COLORREF GetEnableSelectDarkBackColor() override { return MixingColors(::GetSysColor(COLOR_BTNSHADOW), MixingColors(::GetSysColor(COLOR_HIGHLIGHT),::GetSysColor(COLOR_WINDOW),55), 10); }
	COLORREF GetEnableSelectDarkBorderColor() override { return ::GetSysColor(COLOR_HIGHLIGHT); }
	COLORREF GetDisableBackColor() override { return MixingColors(::GetSysColor(COLOR_BTNFACE),::GetSysColor(COLOR_WINDOW),50); }
	COLORREF GetDisableBorderColor() override { return MixingColors(::GetSysColor(COLOR_BTNSHADOW), MixingColors(::GetSysColor(COLOR_HIGHLIGHT),::GetSysColor(COLOR_WINDOW),50), 30); }
};
typedef ToolBoxCtrlStyle_system<ToolBoxCtrlStyle_base> ToolBoxCtrlStyle_base_system;
typedef ToolBoxCtrlStyle_system<ToolBoxCtrlStyle_classic> ToolBoxCtrlStyle_classic_system;
// 
/////////////////////////////////////////////////////////////////////////////
// 
template<typename BASE> struct ToolBoxCtrlStyle_orange : BASE
{		// ToolBoxCtrlStyle_base.
	COLORREF GetBorderColor() override { return RGB(83,83,83); }
	COLORREF GetBackgroundColor() override { return RGB(240,240,240); }
	COLORREF GetGroupTextColor() override { return RGB(0,0,0); }
	COLORREF GetItemTextColor() override { return RGB(0,0,0); }
		// 
	COLORREF GetGroupGradientTopColor() override { return RGB(224,224,224); }
	COLORREF GetGroupGradientBottomColor() override { return RGB(200,200,200); }
		// 
	COLORREF GetEnableSelectLightBackColor() override { return RGB(255,226,155); }
	COLORREF GetEnableSelectLightBorderColor() override { return RGB(220,206,152); }
	COLORREF GetEnableSelectDarkBackColor() override { return RGB(255,207,123); }
	COLORREF GetEnableSelectDarkBorderColor() override { return RGB(206,150,101); }
	COLORREF GetDisableBackColor() override { return RGB(247,247,247); }
	COLORREF GetDisableBorderColor() override { return RGB(206,150,101); }
};
typedef ToolBoxCtrlStyle_orange<ToolBoxCtrlStyle_base> ToolBoxCtrlStyle_base_orange;
typedef ToolBoxCtrlStyle_orange<ToolBoxCtrlStyle_classic> ToolBoxCtrlStyle_classic_orange;
// 
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
template<typename CLASS_STYLE>
struct ToolBoxCtrlEx : ToolBoxCtrl
{	ToolBoxCtrlEx()
	{	style.Install(this);
	}
	CLASS_STYLE style;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
