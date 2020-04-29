///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2004-12-7
///
///		用途：
///			libHTTP，可以实现http文件下载
///=======================================================

// MyHTTPFile.cpp: implementation of the CMyHTTPFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "myhttpfile.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
	#include <Winsock.h>
	#pragma comment(lib, "Ws2_32.lib")
#else
	#include <MyString.h>
	#include <sys/types.h>
	#include <sys/param.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
#endif	//_WIN32


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyHTTPFile::CMyHTTPFile()
{
	m_hSocket = 0;	
	m_nHttpResponseCode = 0;
	m_dwBytesRead = 0;

	PresetRequest();
}

CMyHTTPFile::~CMyHTTPFile()
{
	Close();
}

///-------------------------------------------------------
/// CYJ,2004-12-7
/// Function:
///		parse URL
/// Input parameter:
///		url					[IN]		full URL
///		strScheme			[OUT]		http or ftp, protocol
///		strHost				[OUT]		host name
///		port				[OUT]		port, default value for http = 80, ftp = 21
///		strPath				[OUT]		releative path
/// Output parameter:
///		None
static void parse_url(const char *url, CMyString & strScheme, CMyString & strHost, int *port, CMyString & strPath )
{
	char *slash, *colon;
	char *delim;
	char turl[1024];
	char *t;
	int nDefaultPort = 80;

	/* All operations on turl so as not to mess contents of url */
  
	strncpy(turl, url, sizeof(turl)-1);
	turl[sizeof(turl)-1] = 0;

	delim = "://";

	if ((colon = strstr(turl, delim)) == NULL) 
	{
		fprintf(stderr, "Warning: URL is not in format <scheme>://<host>/<path>.\nAssuming scheme = http.\n");
		strScheme = "http";
		t = turl;
		nDefaultPort = 80;
	} else 
	{
		*colon = '\0';
		strScheme = turl;
		t = colon + strlen(delim);
		if( 0 == strScheme.CompareNoCase( "http" ) )
			nDefaultPort = 80;
		else
			nDefaultPort = 21;			// assume ftp
	}

	/* Now t points to the beginning of host name */

	if ((slash = strchr(t, '/')) == NULL) 
	{
		/* If there isn't even one slash, the path must be empty */
	#ifdef _DEBUG
		TRACE( "Warning: no slash character after the host name.  Empty path.  Adding slash.\n");
	#endif // _DEBUG		
		strHost = t;
		strPath = "/";
	} 
	else 
	{
		strPath = slash;
		*slash = '\0';	/* Terminate host name */
		strHost = t;
	}

	/* Check if the hostname includes ":portnumber" at the end */
	int nPos = strHost.Find( ':' );
	if( nPos < 0 )
		*port = nDefaultPort;	/* HTTP standard */
	else 
	{
		*port = atoi( (char*)(strHost)+nPos+ 1);
		strHost.ReleaseBuffer( nPos );
	}
}

///-------------------------------------------------------
/// CYJ,2004-12-7
/// Function:
///		打开一个http文件
/// Input parameter:
///		lpszURL					http://形式的URL
/// Output parameter:
///		None
BOOL CMyHTTPFile::Open(LPCSTR lpszURL)
{
	Close();

	if( NULL == lpszURL )
		return FALSE;
	m_strURL = lpszURL;

	CMyString strScheme;
	CMyString strHost;
	int nPort = 80;
	CMyString strPath;

	if( FALSE == m_strProxySetting.IsEmpty() )			//	使用代理
		parse_url( (char*)m_strProxySetting, strScheme, strHost, &nPort, strPath );
	else
		parse_url( lpszURL, strScheme, strHost, &nPort, strPath );

	// Find out the IP address
	struct hostent *pNameInfo;
	struct sockaddr_in addr;
	if( (pNameInfo = gethostbyname((char*)strHost)) == NULL )
	{								//	使用的是IP地址
		addr.sin_addr.s_addr = inet_addr((char*)strHost);
		if( (int)addr.sin_addr.s_addr == -1 ) 
		{
			#ifdef _DEBUG
				TRACE( "Unknown host %s\n", (char*)strHost);
			#endif //_DEBUG
			return FALSE;
		}
	}
	else 
		memcpy( (char *)&addr.sin_addr.s_addr, pNameInfo->h_addr, pNameInfo->h_length );

	// Create socket and connect  
	if( ( m_hSocket = socket(PF_INET, SOCK_STREAM, 0)) == -1) 
	{
	#ifdef _DEBUG
		TRACE("httpget: socket()");
	#endif //_DEBUG
		return FALSE;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);  
	if( connect(m_hSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1) 
	{
	#ifdef _DEBUG
		TRACE("httpget: connect()");
	#endif //_DEBUG		
		return FALSE;
	}
#ifdef _DEBUG
	TRACE( "Connected to %s:%d\n", (LPCSTR)strHost, nPort );	
#endif //_DEBUG
	CMyString strRequest;
	if( FALSE == m_strProxySetting.IsEmpty() )
	{					// use proxy
	#ifdef _DEBUG	
		TRACE( "Sending URL %s to proxy...\n",  lpszURL );
	#endif //_DEBUG		
		strRequest.Format( "GET %s HTTP/1.0\r\n", (char*)EncodeURL(lpszURL) );
	} 
	else 
	{
	#ifdef _DEBUG
		TRACE( "Sending request...\n" );
	#endif //_DEBUG
		strRequest.Format( "GET %s HTTP/1.0\r\nHost: %s\r\n", (char*)EncodeURL((char*)strPath), (char*)strHost );
	}
	strRequest += GetFullRequestString();

	send( m_hSocket, (char*)strRequest, strRequest.GetLength(), 0 );

	ReadResponses();
	
	m_dwBytesRead = 0;				// Response 头不能算
	
	return TRUE;
}

///-------------------------------------------------------
/// CYJ,2004-12-7
/// Function:
///		Read data
/// Input parameter:
///		None
/// Output parameter:
///		>0				succ
///		0				none is read
///		<0				failed
int CMyHTTPFile::Read( PBYTE pBuf, int nLen )
{
	if( 0 == m_hSocket )
		return -1;
	int nRetVal = recv( m_hSocket, (char*)pBuf, nLen, 0 );
	if( nRetVal > 0 )
		m_dwBytesRead += nRetVal;
	else
		nRetVal = 0;
	return nRetVal;
}

///-------------------------------------------------------
/// CYJ,2004-12-7
/// Function:
///		Close the socket
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHTTPFile::Close()
{
	if( m_hSocket )
	{
#ifdef _WIN32
		closesocket( m_hSocket );
#else
		close( m_hSocket );
#endif // _WIN32
	}
	m_hSocket = 0;

	m_strURL = "";

	m_aResponse.RemoveAll();
	m_nHttpResponseCode = 0;
	m_dwBytesRead = 0;
}

///-------------------------------------------------------
/// CYJ,2004-12-8
/// Function:
///		Set proxy
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHTTPFile::SetProxy(const char *pszProxySetting)
{	
	if( pszProxySetting )
		m_strProxySetting = pszProxySetting;
	else
		m_strProxySetting = "";
}

///-------------------------------------------------------
/// CYJ,2004-12-8
/// Function:
///		预制Request
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHTTPFile::PresetRequest()
{
	m_aRequest.RemoveAll();
	
	m_aRequest["Accept"] = "*/*";
	m_aRequest["User-Agent"] = "Mozilla/4.0 (Windows;)";
}

///-------------------------------------------------------
/// CYJ,2004-12-8
/// Function:
///		Get full request string, include the end "\r\n\r\n"
/// Input parameter:
///		None
/// Output parameter:
///		None
CMyString CMyHTTPFile::GetFullRequestString()
{
	CMyString strRetVal;
	POSITION pos = m_aRequest.GetStartPosition();

	CMyString strRequest;
	CMyString strValue;
	while( pos )
	{
		m_aRequest.GetNextAssoc( pos, strRequest, strValue );
		strRequest += ": ";
		strRequest += strValue;
		strRequest += "\r\n";
		strRetVal += strRequest;
	}
	strRetVal += "\r\n";

	return strRetVal;
}

///-------------------------------------------------------
/// CYJ,2004-12-8
/// Function:
///		receive the response
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CMyHTTPFile::ReadResponses()
{
	m_aResponse.RemoveAll();

	bool bSucc = FALSE;

	char szResponeString[1024];
	unsigned int nBytesRead = 0;
	while( nBytesRead < sizeof(szResponeString)-2 && 1 == Read( PBYTE(szResponeString+nBytesRead), 1) )
	{
		if( nBytesRead < 4 )
		{
			nBytesRead ++;
			continue;
		}
		if( szResponeString[nBytesRead] == '\n' )
		{
			if( szResponeString[nBytesRead-1] == '\n' )
			{
				bSucc = true;
				break;			//	发现两个回车
			}
			if( szResponeString[nBytesRead-1] == '\r' && szResponeString[nBytesRead-2] == '\n' )
			{
				bSucc = true;
				break;
			}
		}
		nBytesRead ++;
	}

	if( false == bSucc )
		return false;	

	szResponeString[nBytesRead] = 0;
	
#ifdef _DEBUG	
	TRACE( szResponeString );
#endif //_DEBUG

	return ParseResponseStrings( szResponeString );
}

///-------------------------------------------------------
/// CYJ,2004-12-8
/// Function:
///		Response string
/// Input parameter:
///		None
/// Output parameter:
///		None
///	Note
///		When return, the input buffer is demaged
bool CMyHTTPFile::ParseResponseStrings(char *lpszResponseString)
{
	char * pszHead = lpszResponseString;
	char * pszTail = strchr( pszHead, '\n' );

	CMyString strName;
	CMyString strValue;
	if( NULL == pszTail )
		return false;
	*pszTail ++ = 0;			// to next line
	strName = pszHead;
	strName.TrimLeft();
	strName.TrimRight();
	int nPos = strName.Find( ' ' );
	if( nPos < 0 )
		return false;
	m_nHttpResponseCode = atoi( (char*)(strName)+nPos+1 );
	if( (m_nHttpResponseCode/100) != 2 )
		return false;
	while( (pszHead = pszTail) )
	{
		pszTail = strchr( pszHead, '\n' );
		if( pszTail )
			*pszTail ++ = 0;
		strName = pszHead;
		strName.TrimLeft();
		strName.TrimRight();
		if( strName.IsEmpty() )
			break;
		nPos = strName.Find( ':' );
		if( nPos < 0 )
			continue;			//	错误的格式
		strValue = strName.Mid( nPos+1 );
		strName.ReleaseBuffer( nPos );
		strName.TrimRight();
		strName.MakeUpper();
		strValue.TrimLeft();		
		m_aResponse[(char*)strName] = (char*)strValue;
	}
	return true;
}

///-------------------------------------------------------
/// CYJ,2004-12-8
/// Function:
///		Query response value
/// Input parameter:
///		lpszKey				key value
/// Output parameter:
///		NULL				failed. not exist
///		else				value
CMyString CMyHTTPFile::QueryResponseValue(LPCSTR lpszKey) const
{
	CMyString strValue;
	if( NULL == lpszKey || 0 == *lpszKey )
		return strValue;

	CMyString strName = lpszKey;
	strName.MakeUpper();
	
	if( FALSE == m_aResponse.Lookup( (char*)strName, strValue ) )
		strValue = "";
	return strValue;
}

///-------------------------------------------------------
/// CYJ,2004-12-8
/// Function:
///
/// Input parameter:
///		None
/// Output parameter:
///		None
CMyString CMyHTTPFile::GetContentType() const
{
	return QueryResponseValue("Content-Type");
}

///-------------------------------------------------------
/// CYJ,2004-12-8
/// Function:
///		Get content length
/// Input parameter:
///		None
/// Output parameter:
///		0					not define
///		else				content lenght
long CMyHTTPFile::GetContentLenght()
{
	return atoi( (char*)QueryResponseValue("Content-Length") );
}

///-------------------------------------------------------
/// CYJ,2004-12-8
/// Function:
///		追加请求
/// Input parameter:
///		None
/// Output parameter:
///		None
///	注：
///		该方式必须在调用 Open 前进行
void CMyHTTPFile::AppendRequstString(LPCSTR lpszName, LPCSTR lpszValue)
{
	if( NULL == lpszName || 0 == *lpszName )
		return;
	if( NULL == lpszValue || 0 == *lpszValue )
		return;
	m_aRequest[lpszName] = lpszValue;
}

///-------------------------------------------------------
/// CYJ,2004-12-8
/// Function:
///		Encode the URL
/// Input parameter:
///		None
/// Output parameter:
///		None
CMyString CMyHTTPFile::EncodeURL(LPCSTR lpszURL)
{
	CMyString strRetVal;
	PBYTE pbyTmp = (PBYTE)lpszURL;
	BYTE byTmp;
	while( (byTmp = *pbyTmp) )
	{
		if( byTmp < 0x80 )
			strRetVal += (char)byTmp;
		else
		{
			char szTmp[10];
			sprintf( szTmp, "%%%02X", byTmp );
			strRetVal += szTmp;
		}
		pbyTmp ++;
	}
	return strRetVal;
}
