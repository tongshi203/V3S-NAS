// MyRegKey.cpp: implementation of the CMyRegKey class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyRegKey.h"

#include <atlbase.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyRegKey::CMyRegKey()
{

}

CMyRegKey::CMyRegKey(HKEY hKeyParent, LPCSTR pszRegPath)
{
	m_hKeyParent = hKeyParent;
	m_RegPath = pszRegPath;
	m_RegPath += _T("\\");
}

CMyRegKey::CMyRegKey(HKEY hKeyParent, UINT regPath)
{
	m_hKeyParent = hKeyParent;
	m_RegPath .LoadString(regPath);
	m_RegPath += _T("\\");
}

CMyRegKey::~CMyRegKey()
{
}

UINT CMyRegKey::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
{
CString	keyname;
DWORD	dwValue;
	keyname = m_RegPath + lpszSection;
	if( Create(m_hKeyParent,keyname) == ERROR_SUCCESS )
	{
		if( QueryValue( dwValue, lpszEntry ) != ERROR_SUCCESS )
			dwValue = nDefault;
		Close();
	}
	else
		dwValue = nDefault;
	return dwValue;
}

UINT CMyRegKey::GetProfileInt(UINT Section, UINT Entry, int nDefault)
{
CString	SectionName;
CString	EntryName;
	SectionName.LoadString(Section);
	EntryName.LoadString(Entry);
	return( GetProfileInt( SectionName, EntryName,nDefault) );
}

BOOL CMyRegKey::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
CString		keyname;
	keyname = m_RegPath + lpszSection;
	if( Create( m_hKeyParent, keyname ) != ERROR_SUCCESS ) return FALSE;
	SetValue(nValue,lpszEntry);
	Close();
	return TRUE;
}

BOOL CMyRegKey::WriteProfileInt(UINT Section, UINT Entry, int nValue)
{
CString	SectionName;
CString	EntryName;
	SectionName.LoadString(Section);
	EntryName.LoadString(Entry);
	return( WriteProfileInt(SectionName,EntryName,nValue) );
}

CString CMyRegKey::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
CString	keyname;
CString strValue;
	keyname = m_RegPath + lpszSection;
	strValue = lpszDefault;
	if( Create(m_hKeyParent, keyname) == ERROR_SUCCESS )
	{
		DWORD dwType, dwCount;
		LONG lResult = ::RegQueryValueEx( m_hKey, (LPTSTR)lpszEntry, NULL, &dwType,
			NULL, &dwCount);
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_SZ);
			lResult = ::RegQueryValueEx( m_hKey, (LPTSTR)lpszEntry, NULL, &dwType,
				(LPBYTE)strValue.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
			strValue.ReleaseBuffer();
		}
		Close();
	}
	return( strValue );
}

CString CMyRegKey::GetProfileString( UINT Section, UINT Entry, LPCTSTR lpszDefault )
{
CString	SectionName;
CString	EntryName;
	SectionName.LoadString(Section);
	EntryName.LoadString(Entry);
	return( GetProfileString(SectionName,EntryName,lpszDefault) );
}

BOOL CMyRegKey::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
CString	keyname;
CString strValue;
	keyname = m_RegPath + lpszSection;
	strValue = lpszValue;
	if( Create(m_hKeyParent, keyname) != ERROR_SUCCESS ) return FALSE;
	SetValue( lpszValue,lpszEntry );
	Close();
	return TRUE;
}

BOOL CMyRegKey::WriteProfileString(UINT Section, UINT Entry, LPCTSTR lpszValue)
{
CString	SectionName;
CString	EntryName;
	SectionName.LoadString(Section);
	EntryName.LoadString(Entry);
	return( WriteProfileString( SectionName,EntryName,lpszValue) );
}

