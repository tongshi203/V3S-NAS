// TSIPREC2.h : main header file for the TSIPREC2 DLL
//

#if !defined(AFX_TSIPREC2_H__C38A5F2E_4186_4852_8B86_E032371A62CE__INCLUDED_)
#define AFX_TSIPREC2_H__C38A5F2E_4186_4852_8B86_E032371A62CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTSIPREC2App
// See TSIPREC2.cpp for the implementation of this class
//

class CTSIPREC2App : public CWinApp
{
public:
	CTSIPREC2App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTSIPREC2App)
	public:
	virtual int ExitInstance();
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTSIPREC2App)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TSIPREC2_H__C38A5F2E_4186_4852_8B86_E032371A62CE__INCLUDED_)
