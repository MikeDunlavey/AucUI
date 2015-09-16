// AucUIDlg.cpp : implementation file
/*****
Copyright (C) 2007 Michael R. Dunlavey
For license terms, see the copyright notice in file AucUI.h
*****/

#include "stdafx.h"
#include "AucUI.h"
#include "AucUIDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAucUIDlg dialog

CAucUIDlg::CAucUIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAucUIDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAucUIDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// --- DynDlg stuff
	pbaOld = new CByteArray();
	pbaNew = new CByteArray();
	iOld = 0;
	mode = DD_SHOW;
	bInContents = FALSE;
	hwCommandCtl = 0;
	wCommandMsg = 0;
	bUpdateAfterCommand = FALSE;
	idNext = 101;
	idCommand = 0;
	iTimerCount = 0;
	// --- end DynDlg stuff
}

// --- DynDlg stuff

// Utility for IF, FOR, etc. statements
BOOL CAucUIDlg::deIfUtil(BOOL bTest){
	BOOL rval = FALSE;
	BOOL bOldTest;
	bTest = !!bTest;
	deGetPut(&bOldTest, &bTest);
	if (mode==DD_ERASE) rval = bOldTest;
	else if (mode==DD_SHOW) rval = bTest;
	else {
		if (bTest==bOldTest) rval = bTest;
		else {
			rval = TRUE;
			mode = (bTest ? DD_SHOW : DD_ERASE);
		}
	}
	return rval;
}

// Utility for SWITCH statement
void CAucUIDlg::deSwitchUtil(int iValue, int modes[2], int vals[5]){
	int iOldValue = 0;
	modes[0] = modes[1] = mode;
	deGetPut(&iOldValue, &iValue);
	vals[0] = vals[1] = vals[2] = vals[3] = vals[4] = iValue;
	if (mode==DD_UPDATE && iValue != iOldValue){
		modes[0] = DD_ERASE;
		modes[1] = DD_SHOW;
		vals[DD_ERASE] = iOldValue;
	}
}

// Routine to run a pass.
// Note that your code doesn't need to call it.
// It protects against recursive invocation.
void CAucUIDlg::Run(int md, int fg){
	if (bInContents) return;
	bCommandMode = ((fg & FLAG_COMMAND_MODE) != 0);
	bInContents = TRUE;
	if ((fg & FLAG_ENABLE_WINDOW_WHILE_UPDATE)==0) this->EnableWindow(FALSE);
	mode = md;
	deReset();
	DD_TRY {
		deContents();
	}
	DD_CATCH {
	}
	DD_END_CATCH;
	if (bCommandMode){
		ASSERT(pbaNew->GetSize() == 0);
		deReset();
	} else {
		deSwap();
	}

	if ((fg & FLAG_ENABLE_WINDOW_WHILE_UPDATE)==0) this->EnableWindow(TRUE);
	bInContents = FALSE;
	mode = DD_UPDATE;
}

// Control routines follow.

// Note that the controls I'm actually creating here are WIN32 controls.
// You can make any kind of controls you want.

// "static" text control.  The user cannot edit it,
// but the caption can change, so "static" is a misnomer.
void CAucUIDlg::deStatic(int w, int h, const char* szCaption){
	int oldx, oldy, oldw, oldh;
	WORD id;
	char szCaptionOld[512];
	HWND hw = 0;
	if (mode==DD_SHOW){
		id = idNext++;
		if (!bCommandMode){
			hw = ::CreateWindowEx(0, _T("static")
				, NULL, WS_CHILD, gx, gy, w, h
				, this->m_hWnd, (HMENU)id, AfxGetApp()->m_hInstance, NULL
				);
			::SendMessage(hw, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0L);
			::SetWindowText(hw, szCaption);
			::ShowWindow(hw, SW_SHOW);
		}
	}
	deGetPut(&hw, &hw);
	deGetPut(&id, &id);
	deGetPut(&oldx, &gx);
	deGetPut(&oldy, &gy);
	deGetPut(&oldw, &w);
	deGetPut(&oldh, &h);
	deGetPutString(szCaptionOld, szCaption);
	if (mode==DD_ERASE){
		if (!bCommandMode){
			::ShowWindow(hw, SW_HIDE);
			::DestroyWindow(hw);
		}
	}
	if (mode==DD_UPDATE){
		if (!bCommandMode){
			if (strcmp(szCaption, szCaptionOld) != 0)
				::SetWindowText(hw, szCaption);
			if (gx != oldx || gy != oldy || w != oldw || h != oldh){
				::MoveWindow(hw, gx, gy, w, h, TRUE);
			}
		}
	}
	if (mode != DD_ERASE){
		if (bHorizontal)
			gx += w;
		else
			gy += h;
	}
}

// button control
BOOL CAucUIDlg::deButton(int w, int h, const char* szCaption){
	int oldx, oldy, oldw, oldh;
	WORD id;
	char szCaptionOld[512];
	BOOL rval = FALSE;
	HWND hw = 0;
	if (mode==DD_SHOW){
		id = idNext++;
		if (!bCommandMode){
			hw = ::CreateWindowEx(0, _T("button")
				, NULL, WS_CHILD|WS_TABSTOP, gx, gy, w, h
				, this->m_hWnd, (HMENU)id, AfxGetApp()->m_hInstance, NULL
				);
			::SendMessage(hw, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0L);
			::SetWindowText(hw, szCaption);
			::ShowWindow(hw, SW_SHOW);
		}
	}
	deGetPut(&hw, &hw);
	deGetPut(&id, &id);
	deGetPut(&oldx, &gx);
	deGetPut(&oldy, &gy);
	deGetPut(&oldw, &w);
	deGetPut(&oldh, &h);
	deGetPutString(szCaptionOld, szCaption);
	if (mode==DD_ERASE){
		if (!bCommandMode){
			::ShowWindow(hw, SW_HIDE);
			::DestroyWindow(hw);
		}
	}
	if (mode==DD_UPDATE){
		if (!bCommandMode){
			if (strcmp(szCaption, szCaptionOld) != 0)
				::SetWindowText(hw, szCaption);
			if (gx != oldx || gy != oldy || w != oldw || h != oldh){
				::MoveWindow(hw, gx, gy, w, h, TRUE);
			}
		}
	}
	if (mode != DD_ERASE && bCommandMode){
		if (idCommand == id)
		{
			if (wCommandMsg==BN_CLICKED){
				RequestUpdateAfterCommand();
				rval = TRUE;
			}
		}
	}
	if (mode != DD_ERASE){
		if (bHorizontal)
			gx += w;
		else
			gy += h;
	}
	return rval;
}

// text edit control
BOOL CAucUIDlg::deEdit(int w, int h, CString* ps){
	BOOL rval = FALSE;
	TCHAR oldStr[32767];
	const TCHAR* newStr = P((const TCHAR*)(*ps));
	if (newStr==NULL) newStr = _T("");
//	deGetPutString(oldStr, newStr);
	WORD oldid = (mode==DD_SHOW ? idNext++ : 0);
	deGetPut(&oldid, &oldid);
	WORD id = oldid;
	int oldx=0, oldy=0, oldw=0, oldh=0;
	deGetPut(&oldx, &gx);
	deGetPut(&oldy, &gy);
	deGetPut(&oldw, &w);
	deGetPut(&oldh, &h);
	HWND hw = 0;
	HWND hwFocus = ::GetFocus();
	if (0);
	else if (mode==DD_SHOW){
		if (!bCommandMode){
			hw = ::CreateWindowEx(WS_EX_CLIENTEDGE, _T("edit")
				, NULL
				, WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL
				, gx, gy, w, h
				, this->m_hWnd, (HMENU)id, AfxGetApp()->m_hInstance, NULL
				);
			::SendMessage(hw, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0L);
			::SetWindowText(hw, (LPCTSTR)(*ps));
			::ShowWindow(hw, SW_SHOW);
		}
	}
	else if (mode==DD_UPDATE){
		if (!bCommandMode){
			hw = ::GetDlgItem(this->m_hWnd, oldid);
			::GetWindowText(hw, oldStr, sizeof(oldStr)-1);
			if (bUpdateAfterCommand && hwFocus == hw){
				if (_tcscmp(newStr, oldStr)){
					int iFirstLine = -1, iSel0 = 0, iSel1 = -1;
					iFirstLine = ::SendMessage(hw, EM_GETFIRSTVISIBLELINE, 0, 0);
					::SetWindowText(hw, newStr);
					::SendMessage(hw, EM_SETSEL, (WPARAM)iSel0, (LPARAM)iSel1);
					::SendMessage(hw, EM_LINESCROLL, 0, iFirstLine);
				}
			}
			else if (hwFocus != hw){
				if (_tcscmp(newStr, oldStr)){
					int iFirstLine = -1, iSel0 = -1, iSel1 = -1;
					::SendMessage(hw, EM_GETSEL, (WPARAM)&iSel0, (LPARAM)&iSel1);
					iFirstLine = ::SendMessage(hw, EM_GETFIRSTVISIBLELINE, 0, 0);
					::SetWindowText(hw, newStr);
					::SendMessage(hw, EM_SETSEL, (WPARAM)iSel0, (LPARAM)iSel1);
					//::SendMessage(hw, EM_SCROLLCARET, 0, 0);
					::SendMessage(hw, EM_LINESCROLL, 0, iFirstLine);
				}
			}
			if (gx != oldx || gy != oldy || w != oldw || h != oldh){
				::MoveWindow(hw, gx, gy, w, h, TRUE);
			}
		}
	}
	else if (mode==DD_ERASE){
		if (!bCommandMode){
			hw = ::GetDlgItem(this->m_hWnd, oldid);
			::DestroyWindow(hw);
	//		::ShowWindow(hw, SW_HIDE);
	//		::DestroyWindow(hw);
		}
	}
	if (mode != DD_ERASE && bCommandMode){
		hw = ::GetDlgItem(this->m_hWnd, oldid);
		if (hwCommandCtl == hw){
			// only allow update if it doesn't have focus
			if (
//				wCommandMsg == EN_CHANGE
				wCommandMsg == EN_KILLFOCUS
				)
			{
				int len = ::GetWindowTextLength(hw);
				::GetWindowText(hw, oldStr, len+10);
				if ((*ps) != oldStr){
					(*ps) = oldStr;
					RequestUpdateAfterCommand();
					rval = TRUE;
				}
			}
		}
	}

	if (mode != DD_ERASE){
		if (bHorizontal)
			gx += w;
		else
			gy += h;
	}
	return rval;
}

// floating-point number edit control, adapted from text edit control
BOOL CAucUIDlg::deEdit(int w, int h, double* pd){
	BOOL rval = FALSE;
	CString sTemp = "";
	if (mode != DD_ERASE) sTemp.Format("%g", *pd);
	if (deEdit(w, 20, P(&sTemp))){
		float f = -999;
		sscanf(sTemp, "%g", &f);
		if (f != -999) *pd = f;
		rval = TRUE;
	}
	return rval;
}

BOOL CAucUIDlg::deEdit(int w, int h, long* pd){
	BOOL rval = FALSE;
	CString sTemp = "";
	if (mode != DD_ERASE) sTemp.Format("%d", *pd);
	if (deEdit(w, 20, P(&sTemp))){
		long f = -999;
		sscanf(sTemp, "%d", &f);
		if (f != -999) *pd = f;
		rval = TRUE;
	}
	return rval;
}
// tab control
BOOL CAucUIDlg::deTabControl(int w, int h, CStringArray *psa, int* pw){
	BOOL rval = FALSE;
	int style = 0;
	RECT rClient;
	if (mode != DD_ERASE){
		if (!bCommandMode){
			rClient.left = gx;
			rClient.top = gy;
			rClient.right = rClient.left + w;
			rClient.bottom = rClient.top + h;
		}
	}
	WORD oldid = (mode==DD_SHOW ? idNext++ : 0);
	deGetPut(&oldid, &oldid);
	WORD id = oldid;
	int oldx=0, oldy=0, oldw=0, oldh=0;
	deGetPut(&oldx, &gx);
	deGetPut(&oldy, &gy);
	deGetPut(&oldw, &w);
	deGetPut(&oldh, &h);
//	short bOldEnable; deGetPut(&bOldEnable, &bEnable);
	int wNewVal = P(pw ? *pw : 0);
	int wOldVal = 0;
	deGetPut(&wOldVal, &wNewVal);
	HWND hw = 0;
	int i, j;
	if (0);
	else if (mode==DD_SHOW){
		if (!bCommandMode){
			style &= ~WS_GROUP;
			style |= WS_EX_TRANSPARENT;
			hw = ::CreateWindowEx(0, WC_TABCONTROL
				, NULL, WS_CHILD|style, gx, gy, w, h
				, this->m_hWnd, (HMENU)id, AfxGetApp()->m_hInstance, NULL
				);
			::SendMessage(hw, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0L);
			for (i=0; i<psa->GetSize(); i++){
				TC_ITEM tie;
				tie.mask = TCIF_TEXT;
				tie.iImage = -1;
				tie.pszText = (TCHAR*)(const TCHAR*)((*psa)[i]);
				::SendMessage(hw, TCM_INSERTITEM, (WPARAM)i, (LPARAM)&tie);
			}
			::SendMessage(hw, TCM_SETCURSEL, (WPARAM)wNewVal, (LPARAM)NULL);
			::ShowWindow(hw, SW_SHOW);
	//		::EnableWindow(hw, bEnable);
			::SendMessage(hw, TCM_ADJUSTRECT, 0, (LPARAM)&rClient);
	//		int dy = rClient.top - y;
	//		gy += dy;
		}
	}
	else if (mode==DD_UPDATE){
		if (!bCommandMode){
			hw = ::GetDlgItem(this->m_hWnd, oldid);
			int nOld = ::SendMessage(hw, TCM_GETITEMCOUNT, (WPARAM)0, (LPARAM)NULL);
			int nNew = psa->GetSize();
			BOOL bChanged = FALSE;
			for (i=0; i<nOld && i<nNew; i++){
				TCHAR buf[10240];
				TC_ITEM tie;
				tie.mask = TCIF_TEXT;
				tie.iImage = -1;
				tie.pszText = buf;
				tie.cchTextMax = sizeof(buf);
				::SendMessage(hw, TCM_GETITEM, (WPARAM)i, (LPARAM)&tie);
				if ((*psa)[i] != buf){
					::SendMessage(hw, TCM_DELETEITEM, (WPARAM)i, (LPARAM)NULL);
					tie.pszText = (TCHAR*)(const TCHAR*)((*psa)[i]);
					::SendMessage(hw, TCM_INSERTITEM, (WPARAM)i, (LPARAM)&tie);
					bChanged = TRUE;
				}
			}
			while(i < nOld){
				::SendMessage(hw, TCM_DELETEITEM, (WPARAM)i, (LPARAM)NULL);
				nOld--;
				bChanged = TRUE;
			}
			while(i < nNew){
				TC_ITEM tie;
				tie.mask = TCIF_TEXT;
				tie.iImage = -1;
				tie.pszText = (TCHAR*)(const TCHAR*)((*psa)[i]);
				::SendMessage(hw, TCM_INSERTITEM, (WPARAM)i, (LPARAM)&tie);
				i++;
				bChanged = TRUE;
			}
			if (wNewVal != wOldVal || bChanged){
				j = ::SendMessage(hw, TCM_SETCURSEL, (WPARAM)wNewVal, (LPARAM)NULL);
			}
	//		if (bEnable != bOldEnable) ::EnableWindow(hw, bEnable);
			if (gx==oldx && gy==oldy && w==oldw && h==oldh);
			else {
				::MoveWindow(hw, gx, gy, w, h, TRUE);
			}
			::SendMessage(hw, TCM_ADJUSTRECT, 0, (LPARAM)&rClient);
	//		int dy = rClient.top - gy;
	//		gy += dy;
		}
	}
	else if (mode==DD_ERASE){
		if (!bCommandMode){
			hw = ::GetDlgItem(this->m_hWnd, oldid);
			::DestroyWindow(hw);
		}
	}
	if (mode != DD_ERASE && bCommandMode){
		hw = ::GetDlgItem(this->m_hWnd, oldid);
		::SendMessage(hw, TCM_ADJUSTRECT, 0, (LPARAM)&rClient);
//		int dy = rClient.top - gy;
//		gy += dy;
		if (hwCommandCtl == hw){
			if (0);
			else if (wCommandMsg==(WORD)TCN_SELCHANGE
				|| wCommandMsg == 0
				)
			{
				*pw = (short)::SendMessage(hw, TCM_GETCURSEL, (WPARAM)0, (LPARAM)NULL);
				RequestUpdateAfterCommand();
				rval = TRUE;
//				DD_THROW;
			}
		}
	}
	if (mode != DD_ERASE){
		if (bHorizontal)
			gx += w;
		else
			gy += h;
	}
	return rval;
}
// checkbox control
BOOL CAucUIDlg::deCheckBox(int w, int h, const TCHAR* newStr, int* pw){
	if (newStr==NULL) newStr = _T("");
	BOOL rval = FALSE;
	TCHAR oldStr[10240];
	deGetPutString(oldStr, newStr);
	WORD oldid = (mode==DD_SHOW ? idNext++ : 0);
	deGetPut(&oldid, &oldid);
	WORD id = oldid;
	int oldx=0, oldy=0, oldw=0, oldh=0;
	deGetPut(&oldx, &gx);
	deGetPut(&oldy, &gy);
	deGetPut(&oldw, &w);
	deGetPut(&oldh, &h);
	int wNewVal = P(pw ? *pw : 0);
	int wOldVal = 0;
	deGetPut(&wOldVal, &wNewVal);
	HWND hw = 0;
	if (0);
	else if (mode==DD_SHOW){
		if (!bCommandMode){
			hw = ::CreateWindowEx(0, _T("button"), NULL
				, WS_CHILD |WS_TABSTOP | BS_CHECKBOX
				, gx, gy, w, h
				, this->m_hWnd, (HMENU)id, AfxGetApp()->m_hInstance, NULL
				);
			::SendMessage(hw, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0L);
			::SetWindowText(hw, newStr);
			::SendMessage(hw, BM_SETCHECK, !!wNewVal, 0);
			::ShowWindow(hw, SW_SHOW);
		}
	}
	else if (mode==DD_UPDATE){
		if (!bCommandMode){
			hw = ::GetDlgItem(this->m_hWnd, oldid);
			if (_tcscmp(newStr, oldStr)) ::SetWindowText(hw, newStr);
			if (!!wNewVal != !!wOldVal){
				::SendMessage(hw, BM_SETCHECK, !!wNewVal, 0);
			}
			if (gx==oldx && gy==oldy && w==oldw && h==oldh);
			else {
				::MoveWindow(hw, gx, gy, w, h, TRUE);
			}
		}
	}
	else if (mode==DD_ERASE){
		if (!bCommandMode){
			hw = ::GetDlgItem(this->m_hWnd, oldid);
			::DestroyWindow(hw);
		}
	}
	if (mode != DD_ERASE && bCommandMode){
		hw = ::GetDlgItem(this->m_hWnd, oldid);
		if (hwCommandCtl == hw){
			if (wCommandMsg==BN_CLICKED){
				if (pw) *pw = !(*pw);
				RequestUpdateAfterCommand();
				rval = TRUE;
			}
		}
	}

	if (mode != DD_ERASE){
		if (bHorizontal)
			gx += w;
		else
			gy += h;
	}
	return rval;
}

//FILE* fff = NULL;
//int nIndent = 0;

// This is the routine that captures user input events
// and causes them to be handled.
BOOL CAucUIDlg::OnCommand(WPARAM wParam, LPARAM lParam){
	idCommand  = LOWORD(wParam);
	hwCommandCtl = ::GetDlgItem(this->m_hWnd, idCommand);
	wCommandMsg = HIWORD(wParam);
//	if (fff == NULL){
//		fff = fopen("c:\\temp2.txt", "wb");
//		ASSERT(fff != NULL);
//	}
//	fprintf(fff, "%*s ----> OnCommand(%d, %d)\r\n", nIndent++, "", LOWORD(wParam), HIWORD(wParam)); fflush(fff);
	if (0);
	else if (idCommand == IDOK){
	}
	else if (idCommand == IDCANCEL){
		return CDialog::OnCommand(wParam, lParam);
	}
	else if (bInContents){
	}
	else if (idCommand >= 101){
		bUpdateAfterCommand = FALSE;
		Run(DD_UPDATE, FLAG_COMMAND_MODE);
		if (bUpdateAfterCommand){
			Run(DD_UPDATE);
			bUpdateAfterCommand = FALSE;
		}
	}
//	fprintf(fff, "%*s <---- OnCommand(%d, %d)\r\n", --nIndent, "", LOWORD(wParam), HIWORD(wParam)); fflush(fff);
	return TRUE;
}
// --- end DynDlg stuff


void CAucUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAucUIDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAucUIDlg, CDialog)
	//{{AFX_MSG_MAP(CAucUIDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAucUIDlg message handlers

BOOL CAucUIDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	Run(DD_SHOW);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAucUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAucUIDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAucUIDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CAucUIDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	// don't try to run it too early, before it's been painted
	GetClientRect(&rc);
	if (mode==DD_UPDATE){
		// don't disable window while updating
		Run(DD_UPDATE, FLAG_ENABLE_WINDOW_WHILE_UPDATE);
		Invalidate(FALSE);
	}
	
}

void CAucUIDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	iTimerCount++;
	if (mode==DD_UPDATE){
		// don't disable window while updating
		Run(DD_UPDATE, FLAG_ENABLE_WINDOW_WHILE_UPDATE);
		Invalidate(FALSE);
	}
	
//	CDialog::OnTimer(nIDEvent);
}


BOOL CAucUIDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (LOWORD(wParam) >= 101){
		OnCommand(wParam, lParam);
	}
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}
