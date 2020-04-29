///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2004-12-7
///
///		用途：
///			libHTTP，可以实现http文件下载
///=======================================================

#if !defined(AFX_MYHTTPFILE_H__3E1C7071_8F4B_4CFD_A0EE_625267FE4FB5__INCLUDED_)
#define AFX_MYHTTPFILE_H__3E1C7071_8F4B_4CFD_A0EE_625267FE4FB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(_WIN32) && !defined(_LIB)
#ifdef _AFXDLL
	#ifdef _DEBUG
		#pragma comment(lib, "MYHTTPLIBD.lib")		
	#else
		#pragma comment(lib, "MYHTTPLIB.lib")
	#endif //_DEBUG
#else
	#ifdef _DEBUG
		#pragma comment(lib, "MYHTTPLIBSD.lib")
	#else
		#pragma comment(lib, "MYHTTPLIBS.lib")
	#endif //_DEBUG
#endif //_AFXDLL
#endif //_WIN32

#include <MyString.h>
#include <MyMap.h>

class CMyHTTPFile  
{
public:
	void AppendRequstString( LPCSTR lpszName, LPCSTR lpszValue );
	long GetContentLenght();
	CMyString GetContentType() const;
	CMyString QueryResponseValue( LPCSTR lpszKey ) const;
	void PresetRequest();
	void SetProxy( const char * pszProxySetting = NULL );
	void Close();
	int Read( PBYTE pBuf, int nLen );
	BOOL Open( LPCSTR lpszURL );
	CMyHTTPFile();
	virtual ~CMyHTTPFile();
	DWORD	GetBytesRead() const{ return m_dwBytesRead; }

private:
	CMyString EncodeURL( LPCSTR lpszURL );
	bool ParseResponseStrings( char * lpszResponseString );
	bool ReadResponses();
	CMyString GetFullRequestString();
	
	DWORD		m_dwBytesRead;	//	目前只支持 4GB 大小
	int m_nHttpResponseCode;
	CMyString		m_strURL;
	int				m_hSocket;
	CMyString		m_strProxySetting;
	CMyMap<CMyString,LPCSTR,CMyString,LPCSTR>	m_aRequest;
	CMyMap<CMyString,LPCSTR,CMyString,LPCSTR>	m_aResponse;
};

#endif // !defined(AFX_MYHTTPFILE_H__3E1C7071_8F4B_4CFD_A0EE_625267FE4FB5__INCLUDED_)
