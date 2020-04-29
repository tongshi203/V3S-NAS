// MyRegKey.h: interface for the CMyRegKey class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYREGKEY_H__16486756_AACA_11D2_B3E2_00C04FCCA334__INCLUDED_)
#define AFX_MYREGKEY_H__16486756_AACA_11D2_B3E2_00C04FCCA334__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlbase.h>

class CMyRegKey : public CRegKey  
{
public:
	CMyRegKey(HKEY hKeyParent,UINT regPath);
	CMyRegKey(HKEY hKeyParent,LPCSTR  pszRegPath);
	BOOL WriteProfileString( UINT Section, UINT Entry, LPCTSTR lpszValue );
	BOOL WriteProfileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue );
	CString GetProfileString( UINT Section, UINT Entry, LPCTSTR lpszDefault = NULL );
	CString GetProfileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL );
	BOOL WriteProfileInt( UINT Section, UINT Entry, int nValue );
	BOOL WriteProfileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue );
	UINT GetProfileInt(UINT Section,UINT Entry,int nDefault);
	UINT GetProfileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault );
	CMyRegKey();
	virtual ~CMyRegKey();
private:
	HKEY		m_hKeyParent;
	CString		m_RegPath;
};

#endif // !defined(AFX_MYREGKEY_H__16486756_AACA_11D2_B3E2_00C04FCCA334__INCLUDED_)
