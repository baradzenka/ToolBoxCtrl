#pragma once

#include "ToolBoxCtrl/ToolBoxCtrl.h"


// 
struct Dialog : CDialog,
	ToolBoxCtrl::Notify
{
	Dialog(CWnd *parent);

private:   // ToolBoxCtrl::Notify.
	void OnShowItemDescription(ToolBoxCtrl *ctrl, TCHAR const *desc/*or null*/) override;
	void OnUpdateItemState(ToolBoxCtrl *ctrl, int group, int item, int id, CCmdUI *cmd) override;
	void OnClickItem(ToolBoxCtrl *ctrl, int group, int item, int id) override;
	bool OnDragItemStart(ToolBoxCtrl *ctrl, int group, int item, int id) override;
	void OnContextMenu(ToolBoxCtrl *ctrl, CPoint point) override;

private:
	ToolBoxCtrl m_ToolBox;
	ToolBoxCtrlStyle_base_system m_Style1;
	ToolBoxCtrlStyle_base_orange m_Style2;
	ToolBoxCtrlStyle_classic_system m_Style3;
	ToolBoxCtrlStyle_classic_orange m_Style4;
	CListCtrl m_ListCtrl;
	CImageList m_ListImageList;
	CStatusBarCtrl m_StatusBar;
	CImageList m_imglistArrow;
	CPoint m_ptArrow;
		// 
	struct Drag
	{	bool active;
		int group, item;
		HCURSOR curEnable, curDisable;
	} m_Drag;

private:
	LOGFONT const *GetTahomaFont() const;
	LOGFONT const *GetTahomaBoldFont() const;
	LOGFONT const *GetSegoeScriptFont() const;
	void SetChildContolsPos();
	void SetButtonCheck(int id, bool check) const;
	bool GetButtonCheck(int id) const;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	BOOL OnInitDialog() override;
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg void OnPaint();
		// 
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
		// 
	afx_msg void OnContextMenuCollapseAll();
	afx_msg void OnContextMenuExpandAll();
		// 
	afx_msg void OnBnClickedRadio11();
	afx_msg void OnBnClickedRadio12();
	afx_msg void OnBnClickedRadio13();
	afx_msg void OnBnClickedRadio14();
	afx_msg void OnBnClickedCheck21();
	afx_msg void OnBnClickedCheck22();
	afx_msg void OnBnClickedCheck23();
	afx_msg void OnBnClickedCheck24();
	afx_msg void OnBnClickedCheck31();
	afx_msg void OnBnClickedCheck32();
	afx_msg void OnBnClickedCheck41();
};
