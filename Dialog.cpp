#include "stdafx.h"
#include "Dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#ifndef IDC_HAND
	#define IDC_HAND      MAKEINTRESOURCE(32649)
#endif


// 
Dialog::Dialog(CWnd *parent)
	: CDialog(IDD_DIALOG,parent)
{
}

BEGIN_MESSAGE_MAP(Dialog, CDialog)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CAPTURECHANGED()
		// 
	ON_COMMAND(ID_CONTEXT_MENU_COLLAPSE_ALL, OnContextMenuCollapseAll)
	ON_COMMAND(ID_CONTEXT_MENU_EXPAND_ALL, OnContextMenuExpandAll)
		// 
	ON_BN_CLICKED(IDC_RADIO11,OnBnClickedRadio11)
	ON_BN_CLICKED(IDC_RADIO12,OnBnClickedRadio12)
	ON_BN_CLICKED(IDC_RADIO13,OnBnClickedRadio13)
	ON_BN_CLICKED(IDC_RADIO14,OnBnClickedRadio14)
	ON_BN_CLICKED(IDC_CHECK21,OnBnClickedCheck21)
	ON_BN_CLICKED(IDC_CHECK22,OnBnClickedCheck22)
	ON_BN_CLICKED(IDC_CHECK23,OnBnClickedCheck23)
	ON_BN_CLICKED(IDC_CHECK24,OnBnClickedCheck24)
	ON_BN_CLICKED(IDC_CHECK31,OnBnClickedCheck31)
	ON_BN_CLICKED(IDC_CHECK32,OnBnClickedCheck32)
	ON_BN_CLICKED(IDC_CHECK41,OnBnClickedCheck41)
END_MESSAGE_MAP()


int Dialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{	if(CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
		// 
		// 
	if( !m_ToolBox.Create(this,WS_VISIBLE,CRect(0,0,0,0),202) )
		return -1;
	m_Style1.Install(&m_ToolBox);
	m_ToolBox.SetNotifyManager(this);
	m_ToolBox.EnableDynamicItemsUpdate(true);
//	m_ToolBox.SetUpdateMode(ToolBoxCtrl::UpdateModeCommon);
		// 
	m_ToolBox.CreateImages(ToolBoxCtrl::ImageGroup,nullptr,ID_PNG_GROUP_IMAGES1,true,11);
	m_ToolBox.CreateImages(ToolBoxCtrl::ImageItem,nullptr,ID_PNG_ITEM_IMAGES,true,16);
	m_ToolBox.SetGroupCursor( ::LoadCursor(nullptr,IDC_HAND) );
	m_ToolBox.SetFont(ToolBoxCtrl::FontGroup,  GetTahomaBoldFont() );
	m_ToolBox.SetFont(ToolBoxCtrl::FontItemBasic, GetTahomaFont() );
	m_ToolBox.SetFont(ToolBoxCtrl::FontItemExtra, GetSegoeScriptFont() );
		// 
	m_ToolBox.AddGroup(_T("Common Controls"));
		m_ToolBox.AddItem(0, 1, _T("ComboBox"),_T("Description 'ComboBox'"),0,ToolBoxCtrl::FontItemBasic,RGB(200,250,200));
		m_ToolBox.AddItem(0, 2, _T("LinkLabel"),_T("Description 'LinkLabel'"));
		m_ToolBox.AddItem(0, 3, _T("PictureBox"),_T("Description 'PictureBox'"),1);
		m_ToolBox.AddItem(0, 4, _T("ToolTip"),_T("Description 'ToolTip'"),2);
		m_ToolBox.AddItem(0, 5, _T("WebBrowser"),_T("Description 'WebBrowser'"),3);
	m_ToolBox.AddGroup(_T("Menus & Toolbars"), RGB(160,0,0));
		m_ToolBox.AddItem(1, 6, _T("ContextMenuStrip"),_T("Description 'ContextMenuStrip'"),-1,ToolBoxCtrl::FontItemExtra);
		m_ToolBox.AddItem(1, 7, _T("MenuItem"),_T("Description 'MenuItem'"),4,ToolBoxCtrl::FontItemExtra);
		m_ToolBox.AddItem(1, 8, _T("ToolStripContainer"),_T("Description 'ToolStripContainer'"),5,ToolBoxCtrl::FontItemExtra);
	m_ToolBox.AddGroup(_T("Features"));
		m_ToolBox.AddItem(2, 9, _T("Checked"),_T("Description 'Checked'"),6);
		m_ToolBox.AddItem(2, 10, _T("Fore color + Checked + Disabled"),_T("Description 'Fore color + Checked + Disabled'"),6,ToolBoxCtrl::FontItemBasic,CLR_NONE,RGB(150,0,200));
		m_ToolBox.AddItem(2, 11, _T("Back color"),_T("Description 'Back color'"),7,ToolBoxCtrl::FontItemBasic,RGB(200,250,200));
		m_ToolBox.AddItem(2, 12, _T("Back color + Fore color + Disabled"),_T("Description 'Back color + Fore color + Disabled'"),7,ToolBoxCtrl::FontItemBasic,RGB(200,200,250),RGB(0,0,200));
	m_ToolBox.AddGroup(_T("Ribbon Editor"));
		m_ToolBox.AddItem(3, 13, _T("Undo Button"),_T("Description 'Undo Button'"),8);
		m_ToolBox.AddItem(3, 14, _T("OneLevelUp"),_T("Description 'OneLevelUp'"),9);
		m_ToolBox.AddItem(3, 15, _T("CheckboxList"),_T("Description 'CheckboxList'"),10);
		m_ToolBox.AddItem(3, 16, _T("CheckBox"),_T("Description 'CheckBox'"),11);
		// 
	if( !m_ToolBox.LoadState(AfxGetApp(),_T("ToolBoxCtrl"),_T("State")) )
		m_ToolBox.CollapseGroup(2);   // some default state.
	m_ToolBox.Update(true);
		// 
		// 
	if( !m_ListCtrl.CreateEx(WS_EX_CLIENTEDGE,WS_VISIBLE | LVS_LIST | LVS_SMALLICON,CRect(0,0,0,0),this,200) ||
		!m_ToolBox.GetImageList(ToolBoxCtrl::ImageItem,::GetSysColor(COLOR_BTNFACE),&m_ListImageList/*out*/) )
		return -1;
	m_ListCtrl.SetImageList(&m_ListImageList,LVSIL_SMALL);
	m_ListCtrl.SetBkColor( ::GetSysColor(COLOR_BTNFACE) );
	m_ListCtrl.SetTextBkColor( ::GetSysColor(COLOR_BTNFACE) );
		// 
		// 
	if( !m_StatusBar.Create(WS_VISIBLE | CCS_NOPARENTALIGN,CRect(0,0,0,0),this,201) )
		return -1;
	m_StatusBar.SetSimple(TRUE);
	m_StatusBar.SetText(_T("..."),255,0);
		//
		//
	m_imglistArrow.Create(ID_BMP_ARROW,14,0,RGB(255,0,255));
		// 
	m_Drag.active = false;
	m_Drag.curEnable = ::LoadCursor( AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_CUR_DRAG_ENABLE));
	m_Drag.curDisable = ::LoadCursor( AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_CUR_DRAG_DISABLE));
		// 
		// 
	return 0;
}
// 
void Dialog::OnDestroy()
{	m_ToolBox.SaveState(AfxGetApp(),_T("ToolBoxCtrl"),_T("State"));
		//
	if(m_Drag.curEnable)
		::DestroyCursor(m_Drag.curEnable);
	if(m_Drag.curDisable)
		::DestroyCursor(m_Drag.curDisable);
		// 
	CDialog::OnDestroy();
}
/////////////////////////////////////////////////////////////////////////////
// 
BOOL Dialog::OnInitDialog()
{	CDialog::OnInitDialog();
		// 
	SetWindowText(_T("ToolBoxCtrl"));
	ModifyStyle(0,WS_CLIPCHILDREN);
		// 
	SetChildContolsPos();
	GetDlgItem(IDC_TOOLBOXCTRL_BASE)->ShowWindow(SW_HIDE);
		//
	SetButtonCheck(IDC_RADIO11,true);
	SetButtonCheck(IDC_CHECK21,true);
	SetButtonCheck(IDC_CHECK22,true);
	SetButtonCheck(IDC_CHECK23,true);
	SetButtonCheck(IDC_CHECK24,true);
		// 
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// 
void Dialog::OnSize(UINT nType,int cx,int cy)
{	CDialog::OnSize(nType,cx,cy);
		// 
	SetChildContolsPos();
	Invalidate(TRUE);
}
/////////////////////////////////////////////////////////////////////////////
// 
void Dialog::OnPaint()
{	CPaintDC dc(this);
	m_imglistArrow.Draw(&dc,0,m_ptArrow,ILD_TRANSPARENT);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void Dialog::SetChildContolsPos()
{	CWnd *baseWnd = GetDlgItem(IDC_TOOLBOXCTRL_BASE);
		// 
	if(baseWnd)
	{	CRect rcClient, rcStatusBar;
		GetClientRect(&rcClient/*out*/);
		m_StatusBar.GetClientRect(&rcStatusBar/*out*/);
			// 
		CRect rcBase;
		baseWnd->GetWindowRect(&rcBase/*out*/);
		ScreenToClient(&rcBase);
			//
		IMAGEINFO imglistArrow;
		m_imglistArrow.GetImageInfo(0,&imglistArrow/*out*/);
		const int gapWidth = imglistArrow.rcImage.right + 6;
			// 
		rcStatusBar.SetRect(rcBase.left,rcClient.bottom-rcStatusBar.Height(),rcClient.right,rcClient.bottom);
		m_StatusBar.MoveWindow(&rcStatusBar);
			// 
		CRect rcToolBox = rcClient;
		rcToolBox.DeflateRect(rcBase.left,rcBase.top,rcBase.top,rcBase.top+rcStatusBar.Height());
		CRect rcList = rcToolBox;
		const int listWidth = (rcToolBox.Width() - gapWidth)/3 - gapWidth/2;
		rcList.left = rcList.right - listWidth;
		rcToolBox.right = rcList.left - gapWidth;
		m_ToolBox.MoveWindow(&rcToolBox);
		m_ListCtrl.MoveWindow(&rcList);
			// 
		m_ptArrow.x = (rcToolBox.right + rcList.left - imglistArrow.rcImage.right) / 2;
		m_ptArrow.y = (rcToolBox.top + rcToolBox.bottom - imglistArrow.rcImage.bottom) / 2;
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void Dialog::OnShowItemDescription(ToolBoxCtrl * /*ctrl*/, TCHAR const *desc/*or null*/)
{
#if 1   // for this example.
	m_StatusBar.SetText((desc ? desc : _T("...")),255,0);
#else   // but for you it may be this way.
	CFrameWnd *frame = GetParentFrame();
	assert(frame!=nullptr);   // you need to send WM_SETMESSAGESTRING to the frame that contains CStatusBar.
	(!desc ? frame->SendMessage(WM_SETMESSAGESTRING,AFX_IDS_IDLEMESSAGE,0) : frame->SendMessage(WM_SETMESSAGESTRING,0,reinterpret_cast<LPARAM>(desc)));
#endif
}
/////////////////////////////////////////////////////////////////////////////
// 
void Dialog::OnUpdateItemState(ToolBoxCtrl * /*ctrl*/, int /*group*/, int /*item*/, int id, CCmdUI *cmd)
{	if(m_ToolBox.GetUpdateMode()==ToolBoxCtrl::UpdateModeIndividual)
		switch(id)
		{	case 1: cmd->Enable(FALSE); break;
			case 3: cmd->SetCheck(TRUE); break;
			case 9: cmd->SetCheck(TRUE); break;
			case 10: cmd->Enable(FALSE); cmd->SetCheck(TRUE); break;
			case 12: cmd->Enable(FALSE); break;
			case 15: cmd->Enable(FALSE); cmd->SetCheck(TRUE); break;
		}
	else   // ToolBoxCtrlCommon - one call for all items.
	{	cmd->Enable(FALSE);
		cmd->SetCheck(TRUE);
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void Dialog::OnClickItem(ToolBoxCtrl * /*ctrl*/, int group, int item, int /*id*/)
{	const CString text = m_ToolBox.GetItemText(group,item);
	const int image = m_ToolBox.GetItemImage(group,item);
	m_ListCtrl.InsertItem( m_ListCtrl.GetItemCount(), text,image);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool Dialog::OnDragItemStart(ToolBoxCtrl * /*ctrl*/, int group, int item, int /*id*/)
{	m_Drag.active = true;
	m_Drag.group = group;
	m_Drag.item = item;
	SetCapture();
	return true;
}
// 
void Dialog::OnMouseMove(UINT nFlags, CPoint point)
{	CDialog::OnMouseMove(nFlags, point);
		// 
	if(m_Drag.active)
	{	CRect rcList;
		m_ListCtrl.GetWindowRect(&rcList/*out*/);
		ClientToScreen(&point);
		::SetCursor(rcList.PtInRect(point) ? m_Drag.curEnable : m_Drag.curDisable);
	}
}
// 
void Dialog::OnLButtonUp(UINT nFlags, CPoint point)
{	CDialog::OnLButtonUp(nFlags,point);
		// 
	if(m_Drag.active)
	{	::ReleaseCapture();
		m_ToolBox.CancelDragMode();   // !!!.
		m_ToolBox.Update(false);
		m_Drag.active = false;
			// 
		CRect rcList;
		m_ListCtrl.GetWindowRect(&rcList/*out*/);
		ClientToScreen(&point);
		if( rcList.PtInRect(point) )
		{	const CString text = m_ToolBox.GetItemText(m_Drag.group,m_Drag.item);
			const int image = m_ToolBox.GetItemImage(m_Drag.group,m_Drag.item);
			m_ListCtrl.InsertItem( m_ListCtrl.GetItemCount(), text,image);
		}
	}
}
// 
void Dialog::OnCaptureChanged(CWnd *pWnd)
{	if(pWnd!=this)
		if(m_Drag.active)
		{	m_ToolBox.CancelDragMode();   // !!!.
			m_ToolBox.Update(false);
			m_Drag.active = false;
		}
		// 
	CDialog::OnCaptureChanged(pWnd);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void Dialog::OnContextMenu(ToolBoxCtrl *ctrl, CPoint point)
{	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT_MENU);
	CMenu *popup = menu.GetSubMenu(0);
		// 
	if( ctrl->IsOneGroupModeEnable() )
	{	popup->EnableMenuItem(ID_CONTEXT_MENU_COLLAPSE_ALL,MF_BYCOMMAND | MF_GRAYED);
		popup->EnableMenuItem(ID_CONTEXT_MENU_EXPAND_ALL,MF_BYCOMMAND | MF_GRAYED);
	}
		// 
	ctrl->ClientToScreen(&point);
	popup->TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,this);
}
//
void Dialog::OnContextMenuCollapseAll()
{	m_ToolBox.CollapseAllGroups();
	m_ToolBox.Update(true);
}
// 
void Dialog::OnContextMenuExpandAll()
{	m_ToolBox.ExpandAllGroups();
	m_ToolBox.Update(true);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Style: style1.
// 
void Dialog::OnBnClickedRadio11()
{	m_Style1.Install(&m_ToolBox);
	m_ToolBox.CreateImages(ToolBoxCtrl::ImageGroup,nullptr,ID_PNG_GROUP_IMAGES1,true,11);
	m_ToolBox.Update(true);
}
/////////////////////////////////////////////////////////////////////////////
// Style: style2.
// 
void Dialog::OnBnClickedRadio12()
{	m_Style2.Install(&m_ToolBox);
	m_ToolBox.CreateImages(ToolBoxCtrl::ImageGroup,nullptr,ID_PNG_GROUP_IMAGES1,true,11);
	m_ToolBox.Update(true);
}
/////////////////////////////////////////////////////////////////////////////
// Style: style3.
// 
void Dialog::OnBnClickedRadio13()
{	m_Style3.Install(&m_ToolBox);
	m_ToolBox.CreateImages(ToolBoxCtrl::ImageGroup,nullptr,ID_PNG_GROUP_IMAGES2,true,11);
	m_ToolBox.Update(true);
}
/////////////////////////////////////////////////////////////////////////////
// Style: style4.
// 
void Dialog::OnBnClickedRadio14()
{	m_Style4.Install(&m_ToolBox);
	m_ToolBox.CreateImages(ToolBoxCtrl::ImageGroup,nullptr,ID_PNG_GROUP_IMAGES2,true,11);
	m_ToolBox.Update(true);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Show border.
// 
void Dialog::OnBnClickedCheck21()
{	m_ToolBox.ShowBorder( GetButtonCheck(IDC_CHECK21) );
	m_ToolBox.Update(true);
}
/////////////////////////////////////////////////////////////////////////////
// Show scroll.
// 
void Dialog::OnBnClickedCheck22()
{	m_ToolBox.ShowScroll( GetButtonCheck(IDC_CHECK22) );
	m_ToolBox.Update(true);
}
/////////////////////////////////////////////////////////////////////////////
// Show tooltips.
// 
void Dialog::OnBnClickedCheck23()
{	m_ToolBox.EnableToolTip( GetButtonCheck(IDC_CHECK23) );
	m_ToolBox.Update(true);
}
/////////////////////////////////////////////////////////////////////////////
// Custom cursor for groups.
// 
void Dialog::OnBnClickedCheck24()
{	m_ToolBox.SetGroupCursor(GetButtonCheck(IDC_CHECK24) ? ::LoadCursor(nullptr,IDC_HAND) : nullptr);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Drag items.
// 
void Dialog::OnBnClickedCheck31()
{	m_ToolBox.SetActivateMode(GetButtonCheck(IDC_CHECK31) ? ToolBoxCtrl::ActivateModeDrag : ToolBoxCtrl::ActivateModeClick);
	m_ToolBox.Update(true);
}
/////////////////////////////////////////////////////////////////////////////
// Show one group.
// 
void Dialog::OnBnClickedCheck32()
{	m_ToolBox.EnableOneGroupMode( GetButtonCheck(IDC_CHECK32) );
	m_ToolBox.Update(true);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Grayscale images for disabled items.
// 
void Dialog::OnBnClickedCheck41()
{	m_ToolBox.SetDisabledItemImageType(!GetButtonCheck(IDC_CHECK41) ? ToolBoxCtrl::DisabledItemImageTranslucent : ToolBoxCtrl::DisabledItemImageGrayscale);
	m_ToolBox.Update(true);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
LOGFONT const *Dialog::GetTahomaFont() const
{	static const LOGFONT lf = {-11/*8pt*/,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_TT_PRECIS,
		CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,VARIABLE_PITCH | FF_DONTCARE,_T("Tahoma")};
	return &lf;
}
LOGFONT const *Dialog::GetTahomaBoldFont() const
{	static const LOGFONT lf = {-11/*8pt*/,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_TT_PRECIS,
		CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,VARIABLE_PITCH | FF_DONTCARE,_T("Tahoma")};
	return &lf;
}
LOGFONT const *Dialog::GetSegoeScriptFont() const
{	static const LOGFONT lf = {-10,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_TT_PRECIS,
		CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,VARIABLE_PITCH | FF_DONTCARE,_T("Segoe Script")};
	return &lf;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void Dialog::SetButtonCheck(int id, bool check) const 
{	reinterpret_cast<CButton*>( GetDlgItem(id) )->SetCheck(check ? BST_CHECKED : BST_UNCHECKED);
}
bool Dialog::GetButtonCheck(int id) const
{	return reinterpret_cast<CButton*>( GetDlgItem(id) )->GetCheck() == BST_CHECKED;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////





