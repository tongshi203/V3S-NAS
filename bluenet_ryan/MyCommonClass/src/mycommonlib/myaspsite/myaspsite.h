// MyAspSite.h: interface for the CMyAspSite class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYASPSITE_H__6741C4FD_A136_4CDB_8FF6_77DE523DAE9A__INCLUDED_)
#define AFX_MYASPSITE_H__6741C4FD_A136_4CDB_8FF6_77DE523DAE9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMyResponse;
class CBasicEngine;
class CMyString;
class CMyRequest;
class cobject;

class CASPInOutObject
{
public:
	virtual void Write( void * pBuffer, int nSize ) = 0;
	virtual DWORD Read( void * pBuffer, int nByteToRead ) = 0;
	virtual void ResetBuffer()= 0;
};

class CMyAspSite
{
public:
	CMyAspSite( CASPInOutObject * pOutObject );
	virtual ~CMyAspSite();

	bool IsValid();
	bool Run( LPCSTR pszContent, LPCSTR lpszURL=NULL );

	CBasicEngine*	GetBasicEngine();	
	bool RegisterObject( cobject* pObject );
	LPCSTR GetErrorDescription();

private:
	CMyResponse *	m_pResponseObj;
	CBasicEngine*	m_pEngine;
	CASPInOutObject * m_pOutputObject;
	CMyString	*	m_pstrLastError;
	CMyString	*	m_pSplitedAspPage;
	CMyRequest  *	m_pRequest;

private:
	CMyString * SplitASPPage(LPCSTR pszContent);
};

#endif // !defined(AFX_MYASPSITE_H__6741C4FD_A136_4CDB_8FF6_77DE523DAE9A__INCLUDED_)
