///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2004-11-26
///
///		用途：
///			我的HTML显示组合类
///=======================================================

// MyHTMLRender.cpp: implementation of the CMyHTMLRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyHTMLRender.h"
#include "HTMLParser.h"
#include "HTMLTokenizer.h"
#include "MyCompoundFileObj.h"
#include "HTMLRender.h"

#ifdef _WIN32
	#include <MyCommonToolsLib.h>
#else
	#include <mygdiobjs.h>
#endif //_WIN32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyHTMLRender::CMyHTMLRender(CDC * pDC)
{
	m_pDC = pDC;
	m_pFileObj = new CMyCompoundFileObj;
	m_pParser = new CHTMLParser( pDC );
	m_pRender = new CHTMLRender( pDC ); 
	m_pActiveLinkURL = new CMyString;
}

CMyHTMLRender::~CMyHTMLRender()
{
	if( m_pFileObj )
		delete m_pFileObj;
	if( m_pParser )
		delete m_pParser;
	if( m_pRender )
		delete m_pRender;
	if( m_pActiveLinkURL )
		delete m_pActiveLinkURL;
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		显示
/// Input parameter:
///		nWidth			width
///		nHeight			heigh
///		pDC				overload pDC object
/// Output parameter:
///		None
void CMyHTMLRender::Render(int nWidth, int nHeight, CDC * pDC )
{	
	if( false == IsValid() )
		return;	
	if( pDC )
	{
		m_pDC = pDC;
		m_pRender->SetDC( pDC );
	}
	m_pRender->Render( m_pParser, nWidth, nHeight );
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		解析网页
/// Input parameter:
///		pBuf					缓冲区，允许为NULL，此时使用上次设置的内容
///		nLen					大小
///		lpszStartFileName		主页文件名，缺省为 INDEX.HTM
/// Output parameter:
///		None
///	Note:
///		在显示过程中，不能释放 pBuf
bool CMyHTMLRender::Parse(PBYTE pBuf, int nLen, LPCSTR lpszStartFileName)
{
	if( false == IsValid() )
		return false;
	if( NULL == pBuf || 0 == nLen )
	{								//	缺省为原来的文件
		if( 0 == m_pFileObj->GetCount() )
			return Parse( lpszStartFileName );		// 非复合文档方式
	}	
	else if( false == m_pFileObj->Attach( pBuf, nLen ) )
		return false;

	return m_pParser->Parse( m_pFileObj, lpszStartFileName );
}

///-------------------------------------------------------
/// CYJ,2004-12-6
/// Function:
///		解析一个缓冲区，并设置复合文档
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CMyHTMLRender::Parse( PBYTE pBuf, int nLen, LPCSTR lpszContent, int nContentBufSize, LPCSTR lpszRequest )
{
	if( false == IsValid() )
		return false;
	if( NULL == lpszContent || 0 == nLen )
		return false;
	if( NULL == pBuf || 0 == nLen )
	{								//	缺省为原来的文件
		if( 0 == m_pFileObj->GetCount() )
			return false;
	}	
	else if( false == m_pFileObj->Attach( pBuf, nLen ) )
		return false;

	return m_pParser->Parse( m_pFileObj, lpszContent, nContentBufSize, lpszRequest );
}

///-------------------------------------------------------
/// CYJ,2004-12-6
/// Function:
///		解析一个文档
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CMyHTMLRender::Parse( LPCSTR lpszStartFileName )
{
	if( false == IsValid() )
		return false;
	if( NULL == lpszStartFileName )
		return false;
	return m_pParser->Parse( lpszStartFileName );
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		是否有效
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CMyHTMLRender::IsValid()
{
	if( NULL == m_pFileObj )
		return false;
	if( NULL == m_pParser )
		return false;
	if( NULL == m_pRender )
		return false;
	return true;
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		获取有效的连接
/// Input parameter:
///		None
/// Output parameter:
///		NULL				failed
LPCSTR CMyHTMLRender::GetActiveLinkURL( bool bHaseBaseURL )
{
	if( m_pParser )
	{
		*m_pActiveLinkURL = m_pParser->GetActiveLinkURL( bHaseBaseURL );
		return (char*)(*m_pActiveLinkURL);
	}
	return "";
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		下一个连接
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHTMLRender::MoveBackActiveLink()
{
	if( m_pParser )
		m_pParser->MoveBackActiveLink();
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		上一个连接
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHTMLRender::MoveToNextActiveLink()
{
	if( m_pParser )
		m_pParser->MoveToNextActiveLink();
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// 函数功能:
///		获取指定序号的链接与ID
/// 输入参数:
///		nIndex		数组下标
///		pstrID		输出ID，缺省为 NULL
/// 返回参数:
///		指定序号的链接
CMyString CMyHTMLRender::GetLinkURL( int nIndex, CMyString * pstrID )
{
	if( m_pParser )
		return m_pParser->GetLinkURL( nIndex, pstrID );
	return CMyString("");
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// 函数功能:
///		设置新的有效链接
/// 输入参数:
///		nNewIndex			新的链接数组下标
/// 返回参数:
///		原来的序号
int CMyHTMLRender::SetActiveLinkIndex( int nNewIndex )
{
	if( m_pParser )
		return m_pParser->SetActiveLinkIndex( nNewIndex );
	return 0;
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// 函数功能:
///		获取当前有效链接到序号
/// 输入参数:
///		无
/// 返回参数:
///		无
int CMyHTMLRender::GetActiveLinkIndex()
{
	if( m_pParser )
		return m_pParser->GetActiveLinkIndex();
	return 0;
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// 函数功能:
///		获取所有链接个数
/// 输入参数:
///		无
/// 返回参数:
///		无
int CMyHTMLRender::GetLinkUrlCount()
{
	if( m_pParser )
		return m_pParser->GetLinkUrlCount();
	return 0;
}

