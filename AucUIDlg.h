// AucUIDlg.h : header file
/*****
Copyright (C) 2007 Michael R. Dunlavey
For license terms, see the copyright notice in file AucUI.h
*****/

#if !defined(AFX_AucUIDLG_H__79AB0494_CAAD_437C_81D7_BE6EEB5BA4DD__INCLUDED_)
#define AFX_AucUIDLG_H__79AB0494_CAAD_437C_81D7_BE6EEB5BA4DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CAucUIDlg dialog

#include "dynexc.h"

class CAucUIDlg : public CDialog
{
// Construction
public:
	CAucUIDlg(CWnd* pParent = NULL);	// standard constructor

	// --- DynDlg stuff

	// To the reader:

	// The reason for putting the comment "// --- DynDlg stuff" above and below the
	// following code is so if you want to put it in a class of your own, you can just
	// copy that block of code, which in some cases might be easier than trying to
	// inherit it.

	// However, the "deContents" routine is declared virtual, so if you want to derive
	// a class from this one, it should be easy.

	// If you want to add controls, or routines to manage any sort of object like another
	// window of your own choosing, hopefully you can just use the existing ones, like
	// deEdit, as examples to copy and modify.  For example, list boxes, combo boxes,
	// tab controls, scroll bars, multi-line edits, and even menus are not hard to add.
	// As the ambition strikes, I will add some of these.  I've done them many times
	// in other contexts.

	// If you want to adapt this class to your own use, feel free, obviously, to rename
	// the variables so they don't conflict with your app, like putting underscores in
	// front of them, or whatever.
	// I gave them simple obvious names just to help make them readable.

	HWND hwCommandCtl;
	WORD wCommandMsg;
	WORD idCommand;
	WORD idNext;
	BOOL bUpdateAfterCommand;
	void RequestUpdateAfterCommand(){bUpdateAfterCommand = TRUE;}
	BOOL bInContents;
	// This is the mode variable and its possible values
	// The reason for numbering them this way is to simplify the SWITCH statement
	// and the Get and Put routines.
	int mode;
	#define DD_ERASE 1
	#define DD_SHOW 2
	#define DD_UPDATE 3
	// When the bCommandMode flag is set, that means a user input event is being processed
	//	That means control flow is the same as it is in ERASE, SHOW, or UPDATE mode, except that
	//	no controls are created or destroyed, and nothing is written to the new byte array.
	//	At the end of such a pass, it is as if nothing had happened, except that the event has been handled.
	BOOL bCommandMode;
	#define FLAG_COMMAND_MODE 1
	#define FLAG_ENABLE_WINDOW_WHILE_UPDATE 2
	// These two byte arrays are the old and new streams.
	// Note that they are pointed to, so that they are easy to swap at the end of a pass.
	int iOld;			// read index in old bytes
	CByteArray* pbaOld;	// FIFO consists of old bytes followed by new
	CByteArray* pbaNew;
	// Routines for reading/writing data
	void deGetPut(BYTE* pbOld, BYTE* pbNew){
		if (mode & DD_ERASE){
			*pbOld = (*pbaOld)[iOld++];
		}
		if (mode & DD_SHOW){
			if (!bCommandMode){
				(*pbaNew).Add(*pbNew);
			}
		}
	}
	void deGetPut(int* pOld, int* pNew){
		int i; BYTE *pbOld = (BYTE*)pOld, *pbNew = (BYTE*)pNew;
		for (i = 0; i < sizeof(*pOld); i++) deGetPut(pbOld + i, pbNew + i);
	}
	BOOL deChanged(int iNew){
		int iOld = 0;
		deGetPut(&iOld, &iNew);
		return ((mode == DD_UPDATE) && (iNew != iOld));
	}
	void deGetPut(WORD* pOld, WORD* pNew){
		int i; BYTE *pbOld = (BYTE*)pOld, *pbNew = (BYTE*)pNew;
		for (i = 0; i < sizeof(*pOld); i++) deGetPut(pbOld + i, pbNew + i);
	}
	void deGetPut(HWND* pOld, HWND* pNew){
		int i; BYTE *pbOld = (BYTE*)pOld, *pbNew = (BYTE*)pNew;
		for (i = 0; i < sizeof(*pOld); i++) deGetPut(pbOld + i, pbNew + i);
	}
	void deGetPutString(char* sOld, const char* sNew){
		char c;
		if (mode & DD_ERASE){
			do {
				c = (char)((*pbaOld)[iOld++]);
				*sOld++ = c;
			}
			while (c);
		}
		if (mode & DD_SHOW){
			if (!bCommandMode){
				do {
					c = *sNew++;
					(*pbaNew).Add((BYTE)c);
				}
				while(c);
			}
		}
	}
	// Routines to be called at the end of a pass.
	void deSwap(){
		CByteArray* temp = pbaOld; pbaOld = pbaNew; pbaNew = temp;
		deReset();
	}
	void deReset(){
		iOld = 0;
		gx = gy = 0;
		bHorizontal = FALSE;
		pbaNew->SetSize(0);
	}

	// utility routines for conditionals
	BOOL deIfUtil(BOOL bTest);
	void deSwitchUtil(int iValue, int modes[2], int vals[5]);

	// protection macro for expressions
	#define P(expr) (mode != DD_ERASE ? (expr) : 0)

	// IF macro.  bDidIt supports the ELSEIF and ELSE macros
	#define IF(test){int svmode = mode; BOOL bDidIt = FALSE; \
		if(deIfUtil( P(bDidIt = !!(test)) )){

	#define ELSEIF(test) } mode = svmode; \
		if(deIfUtil( P(!bDidIt && (bDidIt = !!(test))) )){

	#define ELSE } mode = svmode; \
		if(deIfUtil( P(!bDidIt) )){

	#define FOR(init, test, incr){int svmode = mode; \
		for(P(init); deIfUtil(P(test)); P(incr)){

	#define WHILE(test){int svmode = mode; \
		while(deIfUtil(P(test))){

	// the way SWITCH works is that, if the switch value changes,
	// it executes the body twice, once with the old value in ERASE mode,
	// then with the new value in SHOW mode.
	// The possible advantage of this is that it cleans up the old controls before making the new ones.
	#define SWITCH(value){int svmode = mode; int modes[2]; int vals[5]; \
		deSwitchUtil(P(value), modes, vals); \
		for (mode=modes[0]; mode<=modes[1]; mode++) \
		switch(vals[mode]){

	// END macro closes any of the other statements
	#define END } mode = svmode;}

	// Following is a useful function for debugging
	// Basically every bug you can have in DE comes down to
	// 1) allowing anything to happen in ERASE mode other than just letting the code retrace its steps.
	//    ex. incrementing counters, following pointers, calling non-DE functions, calling DE functions
	//        from "if" or "for" statements rather than "IF" or "FOR" functions.
	// 2) in DE functions, using any control statements other than the ones provided
	//    such as "if", "for", "while", "goto", "continue", "break", "return"
	//        What these typically do is cause an END statement to be missed, or some such thing,
	//        and the FIFO gets out of sync, causing the UI to act strange.
	// It is best to think of DE as a Separate Language with its own statements.
	// You can "escape" into normal C++ code by the P macro, or "if(mode != DD_ERASE){.....}".
	// You cannot "escape from C++ into DE code.
	// You indicate which functions are in the DE language by prefixing their names with "de",
	// just to help you follow the rules.
	void deAssertSync(int w){
		int oldw = w;
		deGetPut(&oldw, &w);
		ASSERT(oldw==w);
	}

	// function to execute a pass
	// it takes care of setting the mode and managing the byte streams
	void Run(int md, int fg = 0);

	// counter of timer events
	int iTimerCount;

	// graphic attribute stuff
	int gx, gy;
	BOOL bHorizontal;
	int gxSave, gySave;
	CRect rc;
	int Width(){return rc.Width();}
	int Height(){return rc.Height();}
	void deStartHorizontal(){if (mode != DD_ERASE){bHorizontal = TRUE; gxSave = gx; gySave = gy;}}
	void deEndHorizontal(int h){if (mode != DD_ERASE){gx = gxSave; gy = gySave + h; bHorizontal = FALSE;}}

	// These are the routines for painting controls.
	// More are easily added, using these as examples.
	// For example, a listbox, combo box, or tab control could take as arguments
	//   1) an array of strings, and
	//   2) the address of an integer variable representing the user's current choice.
	void deStatic(int w, int h, const char* szCaption);
	BOOL deButton(int w, int h, const char* szCaption);
	BOOL deEdit(int w, int h, CString* ps);
	BOOL deEdit(int w, int h, double* pd);
	BOOL deEdit(int w, int h, long* pd);
	BOOL deCheckBox(int w, int h, const char* szCaption, BOOL* pb);
	BOOL deTabControl(int w, int h, CStringArray *psa, int* pw);

	// the main deContents routine, declared virtual so you can override it.
	virtual void deContents();

	// The painting routines for each different demo.
	// You don't need these if you're doing your own UI.
	void deContentsAdmin();
	void deContentsStatus();
	void deContentsItemFind();
	void deContentsItem();
	void deContentsCustomerFind();
	BOOL GetCustomerNumbers(int iCust, int& iBid, int& iExtra, int& iPaid, int& iOwe);
	void deContentsCustomer();
	void deContentsAddExtra();
	void deContentsAddExtraOneRow(long& iCustomerId);
	void deContentsItemDetail();
	void deContentsCustDetail();

	// --- end DynDlg stuff

// Dialog Data
	//{{AFX_DATA(CAucUIDlg)
	enum { IDD = IDD_AUCUI_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAucUIDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAucUIDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	afx_msg BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AucUIDLG_H__79AB0494_CAAD_437C_81D7_BE6EEB5BA4DD__INCLUDED_)
