// UnEscapeUrl.h: interface for the CUnEscapeUrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNESCAPEURL_H__EB58F326_B79D_11D3_BD17_005004868EAA__INCLUDED_)
#define AFX_UNESCAPEURL_H__EB58F326_B79D_11D3_BD17_005004868EAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MyString.h>

class CUnEscapeUrl  
{
public:
	static CMyString Decode(LPCSTR lpszUrl, BOOL bQuery=FALSE);
	static void UpdateWideChar( CMyString & strUrl );
	static WORD WideToMultiByte( WORD dwWideChar );
	static BYTE DecodeOneByte( LPCSTR pszEsc );
	static CMyString Decode( const CMyString & strUrl, BOOL bQuery = FALSE );
};

#endif // !defined(AFX_UNESCAPEURL_H__EB58F326_B79D_11D3_BD17_005004868EAA__INCLUDED_)
