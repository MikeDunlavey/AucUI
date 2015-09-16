// DYNEXC.H : header file
/*****
Copyright (C) 2007 Michael R. Dunlavey
For license terms, see the copyright notice in file AucUI.h
*****/

#ifndef _DYNEXC_H
#define _DYNEXC_H

/////////////////////////////////////////////////////////////////////////////
// CDynDialogException dialog

class CDynDialogException : public CException
{
	DECLARE_DYNAMIC(CDynDialogException)
public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const {}
#endif
};

#define DD_TRY		TRY
#define DD_THROW	THROW(new CDynDialogException)
#define DD_PASS		
#define DD_CATCH	CATCH(CDynDialogException, _e)
#define DD_END_CATCH	END_CATCH

#endif
