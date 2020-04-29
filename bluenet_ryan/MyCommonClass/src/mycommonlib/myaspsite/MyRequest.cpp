// MyRequest.cpp: implementation of the CMyRequest class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyRequest.h"
#include <MyFunctionToObjectWrapper.h>
#include "UnEscapeUrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#if defined(_DEBUG) && defined(_WIN32)
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

CMyRequest::CMyRequest()
{
	m_pWrapper = new CMyFunctionToObjectWrapper( 3, DWORD(this), (void*)OnBasicFunctionCallLnk, TRUE );
}

CMyRequest::~CMyRequest()
{
	if( m_pWrapper )
		delete m_pWrapper;
}

///-------------------------------------------------------
/// CYJ,2004-7-11
/// Function:
///		Get get request
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyRequest::OnBasicFunctionCall(int nargs, variable* pargs, variable& result)
{
	result = "";
	if( 0 == m_astrValues.GetSize() )
		return;
	if( nargs != 1 )
	{
		result = (LPCSTR)m_astrValues[0];
		return;
	}

	if( pargs[0].type != _string )
	{
		int nIndex = (int)pargs[0];
		if( nIndex < 0 || nIndex >= m_astrValues.GetSize() )
			nIndex = 0;
		result = (LPCSTR)m_astrValues[nIndex];
		return;
	}

	int nCount = m_astrValues.GetSize();
	LPCSTR lpszName = (LPCSTR)pargs[0];
	for(int i=0; i<nCount; i++)
	{
		if( 0 == m_astrNames[i].CompareNoCase( lpszName ) )
		{
			result = (LPCSTR)m_astrValues[i];
			return;
		}
	}
}

void CMyRequest::OnBasicFunctionCallLnk(CMyRequest * pThis, int nargs, variable* pargs, variable& result)
{
	if( NULL == pThis )
		return;
	pThis->OnBasicFunctionCall( nargs, pargs, result );
}

///-------------------------------------------------------
/// CYJ,2004-7-11
/// Function:
///
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyRequest::SetURL(LPCSTR lpszURL)
{
	CMyString strTmp = lpszURL;
	m_astrValues.RemoveAll();
	m_astrNames.RemoveAll();

	int nIndex = strTmp.Find( '?' );
	if( nIndex < 0 )
	{						// No parameter
		m_strURL = strTmp;
		return;
	}
	m_strURL = strTmp.Left( nIndex );
	strTmp.Delete( 0, nIndex + 1 );

	CMyString strName;
	CMyString strValue;

	strTmp.TrimRight();

	while( false == strTmp.IsEmpty()  )
	{
		nIndex = strTmp.Find( '&' );
		if( 0 == nIndex )
		{
			strTmp.Delete( 0 );
			strTmp.TrimLeft();
			continue;
		}
		else if( nIndex >= 0 )
		{
			strValue = strTmp.Left( nIndex );
			strTmp.Delete( 0, nIndex + 1 );
			strTmp.TrimLeft();
		}
		else
		{
			strValue = strTmp;
			strTmp = "";
		}
		nIndex = strValue.Find( '=' );
		if( nIndex < 0 )
			continue;
		strValue.TrimLeft();
		strValue.TrimRight();
		strName = strValue.Left( nIndex );
		strValue.Delete( 0, nIndex + 1 );

		strValue = CUnEscapeUrl::Decode( strValue, TRUE );
		strName = CUnEscapeUrl::Decode( strName, TRUE );

		m_astrNames.Add( strName );
		m_astrValues.Add( strValue );
	}
}

///-------------------------------------------------------
/// CYJ,2004-7-11
/// Function:
///		Get request export function pointer
/// Input parameter:
///		None
/// Output parameter:
///		NULL					failed
///		else					succ
PEXTENSION_FUNCTION_FUNCTION CMyRequest::GetRequestExportFunction()
{
	if( NULL == m_pWrapper )
		return NULL;
	return (PEXTENSION_FUNCTION_FUNCTION)m_pWrapper->GetFucntionEntry();
}
