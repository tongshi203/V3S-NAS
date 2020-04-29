// MyRequest.h: interface for the CMyRequest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYREQUEST_H__BCF320B7_9DA3_4D1A_8E2B_4C514362C0D6__INCLUDED_)
#define AFX_MYREQUEST_H__BCF320B7_9DA3_4D1A_8E2B_4C514362C0D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MyArray.h>
#include <MyString.h>
#include <BasicEngine/basic_interface.h>

class CMyFunctionToObjectWrapper;

class CMyRequest
{
public:
	CMyRequest();
	~CMyRequest();

	void SetURL( LPCSTR lpszURL );
	PEXTENSION_FUNCTION_FUNCTION	GetRequestExportFunction();

	virtual void OnBasicFunctionCall(int nargs, variable* pargs, variable& result);

private:
	CMyArray< CMyString >	m_astrValues;
	CMyArray< CMyString >	m_astrNames;
	CMyFunctionToObjectWrapper *	m_pWrapper;
	CMyString		m_strURL;

private:
	static void OnBasicFunctionCallLnk(CMyRequest * pThis, int nargs, variable* pargs, variable& result);
};

#endif // !defined(AFX_MYREQUEST_H__BCF320B7_9DA3_4D1A_8E2B_4C514362C0D6__INCLUDED_)
