#if !defined(AFX_GETBINDIP_H__61A18D2E_6611_48C2_8AA9_8C46B2EB092F__INCLUDED_)
#define AFX_GETBINDIP_H__61A18D2E_6611_48C2_8AA9_8C46B2EB092F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GetBindIP.h : header file
//
#include "resource.h"
#include "MyWS2_32.h"	// Added by ClassView

/////////////////////////////////////////////////////////////////////////////
// CGetBindIP dialog

class CGetBindIP : public CDialog
{
// Construction
public:
	int GetIPAddressList( PDWORD pdwIPList, int nBufCount );
	CString m_strAdapterName;
	int m_nIPAddressCount;
	CGetBindIP(CWnd* pParent = NULL);   // standard constructor
	~CGetBindIP();

// Dialog Data
	//{{AFX_DATA(CGetBindIP)
	enum { IDD = IDD_DLG_BINDIP };
	CListBox	m_list_Adapter;
	CComboBox	m_cb_IP;
	CString	m_strIP;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetBindIP)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGetBindIP)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListAdapter();
	afx_msg void OnSelchangeCbIp();
	afx_msg void OnDblclkListAdapter();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bIsWSAStartupSucc;
	CMyWS2_32 m_drv;
	int EnumIPAndFillIPCB();
	BOOL LoadAndFillAdapter();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GETBINDIP_H__61A18D2E_6611_48C2_8AA9_8C46B2EB092F__INCLUDED_)
