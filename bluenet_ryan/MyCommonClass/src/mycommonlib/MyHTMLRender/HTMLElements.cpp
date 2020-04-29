///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2004-11-22
///
///		用途：
///			HTML 语言元素
///=======================================================

// HTMLElements.cpp: implementation of the CHTMLElements class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include "HTMLElements.h"
#include "HTMLRender.h"
#include <MyMap.h>
#include <MyArray.h>

#ifdef _WIN32
	#include <CxImage/ximage.h>
#endif //_WIN32
#ifdef __FOR_MICROWIN_EMBED__
	#include <mwin_image.h>
#endif //__FOR_MICROWIN_EMBED__


#if defined(_DEBUG) && defined(_WIN32)
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

#if defined(_WIN32)
	class CMyImageObject : public CxImage
#elif defined( __FOR_MICROWIN_EMBED__)
	class CMyImageObject : public CMWin_Image
#elif defined( __FOR_MY_OSD_EMBED__ )
	class CMyImageObject : public CBitmap
#else
	!!!!!!!!!
#endif //__FOR_MY_OSD_EMBED__
{
public:
	CMyImageObject(LPCSTR lpszFileName);
	~CMyImageObject();
	long AddRef();
	long Release();
	bool SetImage( PBYTE pBuf, int nLen, CDC * pDC, LPCSTR lpszFileName );

#ifdef _WIN32
	CMyImageObject(PBYTE pBuf, int nLen, DWORD nMode, LPCSTR lpszFileName);
#endif //_WIN32

private:
	void RegisterToCache( LPCSTR lpszFileName );

private:
	long m_nRef;
	CMyString m_strFileName;
};

static CMyMap<CMyString,LPCSTR, CMyImageObject*,CMyImageObject*> s_ImagesCache;
static bool	s_GlobalCacheImage = false;

///-------------------------------------------------------
/// CYJ,2005-7-20
/// 函数功能:
///		启用全部图象优化
/// 输入参数:
///		无
/// 返回参数:
///		无
void EnableGlobalImageCache( bool bValue )
{
	if( s_GlobalCacheImage == bValue )
		return;				// 相同的设置，不用作任何处理

	POSITION pos = s_ImagesCache.GetStartPosition();
	CMyArray<CMyImageObject *> aImage;
	while( pos )
	{	
		CMyImageObject * pImage = NULL;
		CMyString strFileName;
		s_ImagesCache.GetNextAssoc( pos, strFileName, pImage );
#ifdef _DEBUG
		ASSERT( pImage );
		TRACE(" AddRef or Release Image %s, pImage=%p\n", (char*)strFileName, pImage );
#endif //_DEBUG
		if( NULL == pImage )
			continue;
		aImage.Add( pImage );		
	}

	int nCount = aImage.GetSize();
	for(int i=0; i<nCount; i++ )
	{
		if( s_GlobalCacheImage )
		{			// 原来已经启用全局缓存，现在取消全局缓存
			ASSERT( false == bValue );
			aImage[i]->Release();
		}
		else
			aImage[i]->AddRef();
	}
	
	s_GlobalCacheImage = bValue;
}

CMyImageObject::CMyImageObject( LPCSTR lpszFileName )
{
	m_nRef = 0;
	RegisterToCache( lpszFileName );
}

#ifdef _WIN32
CMyImageObject::CMyImageObject(PBYTE pBuf, int nLen, DWORD nMode, LPCSTR lpszFileName)
	:CxImage( pBuf, nLen, nMode )
{
	m_nRef = 0;
	RegisterToCache( lpszFileName );
}
#endif //_WIN32

void CMyImageObject::RegisterToCache( LPCSTR lpszFileName )
{
	if( lpszFileName )
	{
		m_strFileName = lpszFileName;
		m_strFileName.TrimLeft();
		m_strFileName.TrimRight();
		if( false == m_strFileName.IsEmpty() )
		{	
			m_strFileName.MakeUpper();
			s_ImagesCache[m_strFileName] = this;
		}
	}
}

CMyImageObject::~CMyImageObject()
{
	if( false == m_strFileName.IsEmpty() )
		s_ImagesCache.RemoveKey( m_strFileName );
}

long CMyImageObject::AddRef()
{
	m_nRef ++;
	return m_nRef;
}

long CMyImageObject::Release()
{
	m_nRef --;
	if( m_nRef )
		return m_nRef;

	delete this;

	return 0;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHTMLElementsBase::CHTMLElementsBase( CHTMLElementsBase * pParent )
{
	m_pParent = pParent;
	if( pParent )
		m_pDC = pParent->m_pDC;
	else
		m_pDC = NULL;
	Preset();
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		预制所有参数
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLElementsBase::Preset()
{
	m_pChild = NULL;
	m_pNext = NULL;
	m_x = 0;
	m_y = 0;
	m_cx = DEFAULT_WINDOW_WIDTH;
	m_cy = DEFAULT_WINDOW_HEIGHT;
	m_z_index = 0;
}

CHTMLElementsBase::~CHTMLElementsBase()
{

}

void CHTMLElementsBase::OnRender(CHTMLRender * pRender, int & x, int & y)
{
}

///-------------------------------------------------------
/// CYJ,2004-11-23
/// Function:
///		设置坐标
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLElementsBase::SetCoordinate(int x, int y, int cx, int cy)
{
	m_x = x;
	m_y = y;
	m_cx = cx;
	m_cy = cy;
}

void CHTMLElementsBase::SetNext( CHTMLElementsBase * pNext )
{
	m_pNext = pNext;
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		追加一个子元素
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLElementsBase::AppendChild( CHTMLElementsBase * pChild )
{
	CHTMLElementsBase * pOlder = m_pChild;
	if( NULL == pOlder )
	{
		m_pChild = pChild;							//	还没有子元素
		return;
	}

	int nZ_Index = pChild->Get_Z_Index();

	if( nZ_Index < m_pChild->m_z_index )		//	先判断特例
	{											//	比第一个Child的z_index还要小
		m_pChild = pChild;
		pChild->SetNext( pOlder );
		return;
	}

	while( CHTMLElementsBase * pNext = pOlder->GetNext() )
	{											//	找到最后一个元素，然后添加
		if( nZ_Index < pNext->Get_Z_Index() )
			break;
		pOlder = pNext;
	}

	CHTMLElementsBase * pOlderNext = pOlder->GetNext();
	pOlder->SetNext( pChild );
	pChild->SetNext( pOlderNext );
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		set parameter
/// Input parameter:
///		strParam			associated parameter
/// Output parameter:
///		None
bool CHTMLElementsBase::SetParameter(CMyString &strParam)
{
#ifdef _DEBUG
	m_strParam = strParam;
#endif //_DEBUG


	return true;
}


///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		结束一个元素的动作，恢复现场。
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLElementsBase::OnEndRender(CHTMLRender * pRender, int & x, int & y)
{

}

#ifdef _DEBUG
void CHTMLElementsBase::Dump()
{
	CHTMLElementsBase * pParent = this;
	while( pParent = pParent->GetParent() )
	{
		TRACE("    ");
	}
}

CMyString CHTMLElementsBase::FormatColor( COLORREF colorValue )
{
	CMyString strRetVal;
	PBYTE pTmpBuf = (PBYTE)&colorValue;
	strRetVal.Format("%02X%02X%02X", pTmpBuf[0], pTmpBuf[1], pTmpBuf[2] );
	return strRetVal;
}

#endif //_DEBUG


///-------------------------------------------------------
/// CYJ,2004-11-23
/// Function:
///		获取一个参数
/// Input parameter:
///		strParam			参数
///		lpszItemName		待获取的参数名称
///		cSeparator			隔离符号
/// Output parameter:
///		参数
CMyString CHTMLElementsBase::GetOneParameterItem( CMyString & strParam, LPCSTR lpszItemName, char cSeparator )
{
	CMyString strTmpParam = strParam;
	strTmpParam.MakeUpper();

	CMyString strRetVal;
	int nPos = strTmpParam.Find( lpszItemName );
	if( nPos < 0 )
		return strRetVal;
	LPCSTR lpszTmpStr = (char*)(strParam) + nPos + strlen(lpszItemName);
	while( *lpszTmpStr && *lpszTmpStr != cSeparator )
	{
		lpszTmpStr ++;
	}
	lpszTmpStr ++;

	strRetVal = lpszTmpStr;
	nPos = strRetVal.FindOneOf( " ,;:" );
	if( nPos > 0 )
		strRetVal.ReleaseBuffer( nPos );

	return strRetVal;
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		获取当前连接
/// Input parameter:
///		None
/// Output parameter:
///		None
CMyString CHTMLElementsBase::GetLinkURL()
{		
	CHTMLElementsBase * pParent = GetParent();
	if( pParent )
		return pParent->GetLinkURL();	
	CMyString strRetVal;
	return strRetVal;	
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// 函数功能:
///		获取链接的元素对象
/// 输入参数:
///		无
/// 返回参数:
///		无
CHTMLElementsBase * CHTMLElementsBase::GetElementOfLinkURL()
{
	CHTMLElementsBase * pParent = GetParent();
	if( pParent )
		return pParent->GetElementOfLinkURL();
	return this;	
}


///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		转换为颜色
/// Input parameter:
///		None
/// Output parameter:
///		None
COLORREF CHTMLElementsBase::GetValueAsColor( CMyString & strValue )
{
	COLORREF RetVal = 0;
	strValue.Replace('#', 0x20 );
	DWORD dwColor = strtoul( (char*)strValue, NULL, 16 );

	for(int i=0; i<3; i++)
	{
		RetVal <<= 8;
		RetVal |= BYTE( dwColor & 0xFF );
		dwColor >>= 8;
	}

	return RetVal;
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		设置当前行的高度
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLElementsBase::SetCurrentLineHeight( int nNewValue )
{
	if( m_pParent )
		m_pParent->SetCurrentLineHeight( nNewValue );
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		开始新的一行
/// Input parameter:
///		None
/// Output parameter:
///		返回当前行的高度
int CHTMLElementsBase::OnNewLine()
{
	if( m_pParent )
		return m_pParent->OnNewLine();
	return DEFAULT_ONE_LINE_HEIGHT;
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		获取行高度
/// Input parameter:
///		None
/// Output parameter:
///		None
int CHTMLElementsBase::GetLineHeight()
{
	if( m_pParent )
		return m_pParent->GetLineHeight();
	else
		return DEFAULT_ONE_LINE_HEIGHT;	
}

//////////////////////////////////////////////////////////
CHTMLContainerBase::CHTMLContainerBase( CHTMLElementsBase * pParent )
	:CHTMLElementsBase( pParent )
{
	m_nCurrentLineHeight = DEFAULT_ONE_LINE_HEIGHT;
	m_nDefLineHeight = DEFAULT_ONE_LINE_HEIGHT;
}

CHTMLContainerBase::~CHTMLContainerBase()
{
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		需要改变一行的高度
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLContainerBase::OnRender(CHTMLRender * pRender, int & x, int & y)
{
	m_nCurrentLineHeight = m_nDefLineHeight;
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		设置当前行高度，取最大值
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLContainerBase::SetCurrentLineHeight( int nNewValue )
{
	if( m_nCurrentLineHeight < nNewValue )
		m_nCurrentLineHeight = nNewValue;			//	保留最大的行高度
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		开始新的一行，需要重新赋值
/// Input parameter:
///		None
/// Output parameter:
///		返回当前行的高度
int CHTMLContainerBase::OnNewLine()
{
	int nRetVal = m_nCurrentLineHeight;
	m_nCurrentLineHeight = m_nDefLineHeight;
	return nRetVal;
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		获取当前行高度
/// Input parameter:
///		None
/// Output parameter:
///		None
int CHTMLContainerBase::GetLineHeight()
{
	return m_nCurrentLineHeight; 
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		设置缺省的行高度
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLContainerBase::SetDefLineHeight( int nNewValue )
{
	m_nDefLineHeight = nNewValue;
}

//////////////////////////////////////////////////////////
CHTML_Dummy_Element::CHTML_Dummy_Element(CHTMLElementsBase * pParent, LPCSTR lpszTagName )
 :CHTMLContainerBase( pParent )
{
	m_strTagName = lpszTagName;
}

CHTML_Dummy_Element::~CHTML_Dummy_Element()
{
}


//////////////////////////////////////////////////////////
CHTMLTextElement::CHTMLTextElement(CHTMLElementsBase * pParent)
	:CHTMLLinkedElementBase( pParent )
{
	m_bIsPreparedFormat = false;
}

CHTMLTextElement::~CHTMLTextElement()
{

}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		设置
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CHTMLTextElement::SetParameter( CMyString & strParam )
{ 
	m_bIsPreparedFormat = false;	
	CHTMLElementsBase * pParent = GetParent();
	while( pParent )
	{
		if( 0 == strcmp( pParent->GetTagName(), "PRE" ) )
		{
			m_bIsPreparedFormat = true;			
			break;
		}
		pParent = pParent->GetParent();
	}

	if( false == m_bIsPreparedFormat )
	{		
		strParam.Remove( '\n' );
		strParam.Remove( 9 );			//	删除 Tab
		strParam.Remove( 0x20 );		//	删除空格		
	}
	else
		strParam.Replace("\t", "    ");

	strParam.Remove( '\r' );
	strParam.Replace("&nbsp;", " ");
	m_strText = strParam; 
	return true; 
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		显示文字
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLTextElement::OnRender(CHTMLRender * pRender, int & x, int & y)
{
	CDC * pDC = pRender->GetDC();

	int x1 = GetX() + GetWidth();
	int y1 = GetY() + GetHeight();
	y = y + GetLineHeight() - DEFAULT_ONE_LINE_HEIGHT;

	COLORREF OldTextColor;
	if( m_bIsActiveLink )
	{
		OldTextColor = pRender->GetTextColor();		//	当前有效连接
		pRender->SetTextColor( pRender->GetActiveLinkTextColor() );
	}

	LPCSTR lpszTmp = (char*)m_strText;
	int nCountLeft = m_strText.GetLength();
	SIZE	sizeNeed;
	int nCharCountFix = 0;	
	while( nCountLeft > 0 )
	{
		if( m_bIsPreparedFormat )
		{									//	预排格式，需要手动换行
			char * pszNewLine = strchr( lpszTmp, '\n' );			
			if( pszNewLine )
				nCharCountFix = pszNewLine - lpszTmp + 1;
			else
				nCharCountFix = strlen( lpszTmp );
			GetTextExtentPoint( pDC->m_hDC, lpszTmp, nCharCountFix, &sizeNeed );
		}
		else
		{
			nCharCountFix = nCountLeft;
			if( FALSE == GetTextExtentExPoint( pDC->m_hDC, lpszTmp, nCountLeft, x1-x, &nCharCountFix, NULL, &sizeNeed ) )
				return;			//	error occur
		}
		if( 0 == nCharCountFix )
			return;				//	error occur
		//	可能需要进行汉字个数的统计				
		if( nCharCountFix > 1 || *lpszTmp != '\n' )
		{						//	若本行只保护一个 '\n' 字符，则不做任何显示。
			int nCharToDraw = nCharCountFix;
			if( nCharCountFix && lpszTmp[nCharCountFix-1] == '\n' )
				nCharToDraw --;
			pDC->TextOut( x, y, lpszTmp, nCharToDraw );
			if( pRender->IsBold() )
				pDC->TextOut( x+1, y, lpszTmp, nCharToDraw );
			if( pRender->IsUnderLine() )
			{
				SIZE	sizeDrawn;
				GetTextExtentPoint( pDC->m_hDC, lpszTmp, nCharToDraw, &sizeDrawn );
				if( pRender->IsBold() )
					sizeDrawn.cx ++;			
				sizeDrawn.cy ++;
				pDC->MoveTo( x, y+sizeDrawn.cy );
				pDC->LineTo( x+sizeDrawn.cx, y+sizeDrawn.cy );
			}
		}
		nCountLeft -= nCharCountFix;
		lpszTmp += nCharCountFix;

		x += sizeNeed.cx;
		if( x >= x1 || m_bIsPreparedFormat )
		{						//	换行
			OnNewLine();		//	新的一样，调整 y 的高度
			y += GetLineHeight();
			x = GetX();			//	回到第一列
			if( y > y1 )
				break;			//	超过高度
		}		
	}	
	if( m_bIsActiveLink )			//	恢复颜色
		pRender->SetTextColor( OldTextColor );
}

//////////////////////////////////////////////////////////
CHTML_A_Element::CHTML_A_Element(CHTMLElementsBase * pParent)
	: CHTMLElementsBase( pParent )
{	
	m_bShowUnderLine = true;
}

CHTML_A_Element::~CHTML_A_Element()
{
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		需要设置字体颜色
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_A_Element::OnRender(CHTMLRender * pRender, int & x, int & y)
{
	m_OldTextColor = pRender->GetTextColor();
	m_bOldUnderLine = pRender->IsUnderLine();

	pRender->SetTextColor( pRender->GetLinkTextColor() );
	pRender->SetUnderLine( m_bShowUnderLine );
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		恢复字体颜色
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_A_Element::OnEndRender(CHTMLRender * pRender, int & x, int & y)
{
	pRender->SetTextColor( m_OldTextColor );
	pRender->SetUnderLine( m_bOldUnderLine );
}

///-------------------------------------------------------
/// CYJ,2004-11-23
/// Function:
///		设置参数，需要读取的参数只有一个：href
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CHTML_A_Element::SetParameter( CMyString & strParam )
{
#ifdef _DEBUG
	CHTMLElementsBase::SetParameter( strParam );
#endif //_DEBUG

	m_strID = GetOneParameterItem( strParam, "ID", '=' );
	m_strHRef = GetOneParameterItem( strParam, "HREF", '=' );
	
	CMyString strShowUnderLine = GetOneParameterItem( strParam, "UNDERLINE", '=' );
	if( strShowUnderLine.GetLength() )
		m_bShowUnderLine = ( atoi( (char*)strShowUnderLine ) != 0 );
	else
		m_bShowUnderLine = true;

	return m_strHRef.GetLength() > 0;
}

//////////////////////////////////////////////////////////
CHTML_DIV_Element::CHTML_DIV_Element(CHTMLElementsBase * pParent)
	: CHTMLContainerBase( pParent )
{
}

CHTML_DIV_Element::~CHTML_DIV_Element()
{
}

///-------------------------------------------------------
/// CYJ,2004-11-23
/// Function:
///		设置参数
/// Input parameter:
///		None
/// Output parameter:
///		None
///	Note:
///		需要分析的参数有：left，top, width, right, z-index
bool CHTML_DIV_Element::SetParameter( CMyString & strParam )
{
#ifdef _DEBUG
	CHTMLElementsBase::SetParameter( strParam );
#endif //_DEBUG

	strParam.MakeUpper();
	int anPosition[5];		// Left, top, width, height, z-index
	static const char * apszPosName[6] = { "LEFT","TOP", "WIDTH", "HEIGHT","Z-INDEX", "POSITION" };
	ASSERT( sizeof(anPosition)/sizeof(int)+1 == sizeof(apszPosName)/sizeof(const char*) );
	LPCSTR pszParam = (char*)(strParam);
	bool bIsPositionAbsolute = false;
	for(int i=0; i<sizeof(anPosition)/sizeof(int); i++)
	{
		anPosition[i] = 0;
		int nPos = strParam.Find( apszPosName[i] );
		if( nPos < 0 )
			continue;
		LPCSTR lpszTmpStr = pszParam + nPos + strlen(apszPosName[i]);
		while( *lpszTmpStr && *lpszTmpStr != ':' )
		{
			lpszTmpStr ++;
		}
		lpszTmpStr ++;		
		if( i == 5 )
		{
			ASSERT( strcmp(apszPosName[i], "POSITION") == 0 );
			bIsPositionAbsolute = ( strcmp(lpszTmpStr, "ABSOLUTE") == 0 );
		}
		else
			anPosition[i] = strtoul( lpszTmpStr, NULL, 10 );						
	}

	m_x = anPosition[0];
	m_y = anPosition[1];
	if( false == bIsPositionAbsolute )
	{
		CHTMLElementsBase * pParent = GetParent();
		if( pParent )
		{		
			m_x += pParent->GetX();
			m_y += pParent->GetY();
		}
	}
	m_cx = anPosition[2];
	m_cy = anPosition[3];
	if( 0 == m_cx )
		m_cx = 800;
	if( 0 == m_cy )
		m_cy = 600;

	m_z_index = anPosition[4];

	return true;
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		改变坐标
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_DIV_Element::OnRender(CHTMLRender * pRender, int & x, int & y)
{
	CHTMLContainerBase::OnRender( pRender, x, y );

	m_nOld_x = x;
	m_nOld_y = y;
	x = m_x;
	y = m_y;
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		恢复坐标
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_DIV_Element::OnEndRender(CHTMLRender * pRender, int & x, int & y)
{
	x = m_nOld_x;
	y = m_nOld_y;
}

//////////////////////////////////////////////////////////
CHTML_BR_Element::CHTML_BR_Element(CHTMLElementsBase * pParent)
	: CHTMLElementsBase( pParent )
{
}

CHTML_BR_Element::~CHTML_BR_Element()
{
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		换行
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_BR_Element::OnRender(CHTMLRender * pRender, int & x, int & y)
{
	y += OnNewLine();		//	新的一样，调整 y 的高度
	x = GetX();				//	回到第一行
}

//////////////////////////////////////////////////////////
CHTML_IMAGE_Element::CHTML_IMAGE_Element(CHTMLElementsBase * pParent)
	:CHTMLLinkedElementBase( pParent )
{
#if defined(_WIN32) || defined(__FOR_MICROWIN_EMBED__) || defined(__FOR_MY_OSD_EMBED__)
	m_pImageObj = NULL;
	m_pImageObjActive = NULL;
#endif //_WIN32
}

CHTML_IMAGE_Element::~CHTML_IMAGE_Element()
{
#if defined(_WIN32) || defined(__FOR_MICROWIN_EMBED__) || defined(__FOR_MY_OSD_EMBED__)
	if( m_pImageObj )
		m_pImageObj->Release();
	if( m_pImageObjActive )
		m_pImageObjActive->Release();
#endif //_WIN32

}

///-------------------------------------------------------
/// CYJ,2004-11-23
/// Function:
///		get image parameter, only one parameter, src
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CHTML_IMAGE_Element::SetParameter( CMyString & strParam )
{	
#if defined(_WIN32) || defined(__FOR_MICROWIN_EMBED__) || defined(__FOR_MY_OSD_EMBED__)
	if( m_pImageObj )
		m_pImageObj->Release();
	m_pImageObj = NULL;
	if( m_pImageObjActive )
		m_pImageObjActive->Release();
	m_pImageObjActive = NULL;
#endif //_WIN32

	m_strFileName = GetOneParameterItem( strParam, "SRC", '=' );
	m_strFileName.TrimLeft();
	int nPos = m_strFileName.FindOneOf( " ;," );
	if( nPos > 0 )
		m_strFileName.ReleaseBuffer( nPos );	
	m_strFileName.TrimRight();

	m_strActiveURLFileName = GetOneParameterItem( strParam, "ACTIVESRC", '=' );
	m_strActiveURLFileName.TrimLeft();
	nPos = m_strActiveURLFileName.FindOneOf( " ;," );
	if( nPos > 0 )
		m_strActiveURLFileName.ReleaseBuffer( nPos );	
	m_strActiveURLFileName.TrimRight();

	return true;
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		输出图形
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_IMAGE_Element::OnRender(CHTMLRender * pRender, int & x, int & y)
{
#if defined(_WIN32) || defined(__FOR_MICROWIN_EMBED__) || defined(__FOR_MY_OSD_EMBED__)
	CMyImageObject * pImgObj = NULL;
	if( m_bIsActiveLink && m_pImageObjActive )
		pImgObj = m_pImageObjActive;			//  CYJ,2005-4-11 active link and has ActiveLinkImage
	else
		pImgObj = m_pImageObj;

	if( NULL == pImgObj )
		return;

#if defined(__FOR_MY_OSD_EMBED__)
	pRender->GetDC()->DrawImage( x, y, 0, 0, pImgObj );
#else
  	pImgObj->Draw( pRender->GetDC()->m_hDC, x, y );
#endif // defined(__FOR_MY_OSD_EMBED__)

	SetCurrentLineHeight( m_cy );
	x += m_cx;	
#else
	!!!!!...
#endif //__FOR_MICROWIN_EMBED__
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		获取图象文件名
/// Input parameter:
///		None
/// Output parameter:
///		None
CMyString CHTML_IMAGE_Element::GetFileName()
{
	return m_strFileName;
}

///-------------------------------------------------------
/// CYJ,2005-4-11
/// 函数功能:
///		获取有效激活的图象
/// 输入参数:
///		无
/// 返回参数:
///		图象名称
CMyString CHTML_IMAGE_Element::GetActiveImgFileName()
{
	return m_strActiveURLFileName;
}

///-------------------------------------------------------
/// CYJ,2005-4-11
/// 函数功能:
///		载入图象
/// 输入参数:
///		pBuf			image data buffer
///		nLen			buffer size
///		pDC				DC
///		lpszFileName	image file name
/// 返回参数:
///		无
static CMyImageObject * MyLoadImage( PBYTE pBuf, int nLen, CDC * pDC, LPCSTR lpszFileName )
{
	CMyString strFileName = lpszFileName;
	strFileName.TrimLeft();
	strFileName.TrimRight();
	strFileName.MakeUpper();
	CMyImageObject * pRetVal = NULL;
	if( s_ImagesCache.Lookup(strFileName, pRetVal) && pRetVal )
	{			// find one cached image object
		pRetVal->AddRef();
		return pRetVal;
	}

#if defined(_WIN32)
	pRetVal = new CMyImageObject( pBuf, nLen, CXIMAGE_FORMAT_UNKNOWN, lpszFileName );
#else
  	pRetVal = new CMyImageObject( lpszFileName );
#endif //_WIN32
	
	if( NULL == pRetVal )
		return NULL;

#ifdef __FOR_MICROWIN_EMBED__
  	if( false == pRetVal->LoadFromBuffer( pDC->m_hDC, pBuf, nLen ) )
	{
		delete pRetVal;
		return NULL;
	}
#endif //__FOR_MICROWIN_EMBED__				
#if defined(__FOR_MY_OSD_EMBED__)
	if( false == pRetVal->LoadBitmap( pDC, pBuf, nLen ) )
	{
		delete pRetVal;		
		return NULL;
	}
#endif // defined(__FOR_MY_OSD_EMBED__)

	pRetVal->AddRef();

	if( s_GlobalCacheImage )		//  CYJ,2005-7-20 启用全局缓存
		pRetVal->AddRef();

	return pRetVal;
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		设置对应文件的映象数据
/// Input parameter:
///		pBuf			buffer
///		nLen			buffer size
///		bNormal			for normal
/// Output parameter:
///		None
void CHTML_IMAGE_Element::SetImageData( PBYTE pBuf, int nLen, bool bNormal )
{
#if defined(_WIN32) || defined(__FOR_MICROWIN_EMBED__)||defined(__FOR_MY_OSD_EMBED__)
	if( bNormal )
	{	
		if( m_pImageObj )
			m_pImageObj->Release();		
		m_pImageObj = MyLoadImage( pBuf, nLen, m_pDC, m_strFileName );
		if( NULL == m_pImageObj )
			return;
	  
		m_cx = m_pImageObj->GetWidth();
		m_cy = m_pImageObj->GetHeight();
	}
	else if( m_pImageObj )
	{							//	Active Link image
		if( m_pImageObjActive )
			m_pImageObjActive->Release();
		m_pImageObjActive = MyLoadImage( pBuf, nLen, m_pDC, m_strActiveURLFileName );
	}

#else

	!!!!!!!!............

#endif //_WIN32
}

//////////////////////////////////////////////////////////
CHTML_FONT_Element::CHTML_FONT_Element(CHTMLElementsBase * pParent)
	:CHTMLElementsBase( pParent )
{
	m_Color = INVALID_COLOR_VALUE;
	m_nSize = -1;
}

CHTML_FONT_Element::~CHTML_FONT_Element()
{
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		恢复字体大小和文本颜色
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_FONT_Element::OnEndRender(CHTMLRender * pRender, int & x, int & y)
{
	pRender->SetTextColor( m_OldColor );
	pRender->SetFontSize( m_nOldFontSize );
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		设置字体大小和颜色
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_FONT_Element::OnRender(CHTMLRender * pRender, int & x, int & y)
{
	m_nOldFontSize = pRender->GetFontSize();
	m_OldColor = pRender->GetTextColor();

	if( m_Color != INVALID_COLOR_VALUE )
		pRender->SetTextColor( m_Color );
	if( m_nSize > 0 )
		pRender->SetFontSize( m_nSize );
}

///-------------------------------------------------------
/// CYJ,2004-11-23
/// Function:
///		设置参数，有两个参数，font，size
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CHTML_FONT_Element::SetParameter( CMyString & strParam )
{
	strParam.MakeUpper();

	CMyString strTmp = GetOneParameterItem( strParam, "SIZE", '=' );
	m_nSize = strtoul( strTmp, NULL, 10 );		//	若为0，则不改变字体大小

	strTmp = GetOneParameterItem( strParam, "COLOR", '=' );
	if( strTmp.IsEmpty() )
		m_Color = INVALID_COLOR_VALUE;
	else
		m_Color = GetValueAsColor( strTmp );	

	return true;
}

//////////////////////////////////////////////////////////
CHTML_U_Element::CHTML_U_Element(CHTMLElementsBase * pParent)
	:CHTMLElementsBase( pParent )
{
}

CHTML_U_Element::~CHTML_U_Element()
{
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		恢复下划线
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_U_Element::OnEndRender(CHTMLRender * pRender, int & x, int & y)
{
	pRender->SetUnderLine( m_bOldValue );
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		设置为下划线
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_U_Element::OnRender(CHTMLRender * pRender, int & x, int & y)
{
	m_bOldValue = pRender->IsUnderLine();
	pRender->SetUnderLine( true );
}

//////////////////////////////////////////////////////////
CHTML_B_Element::CHTML_B_Element(CHTMLElementsBase * pParent)
	:CHTMLElementsBase( pParent )
{
}

CHTML_B_Element::~CHTML_B_Element()
{
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_B_Element::OnEndRender(CHTMLRender * pRender, int & x, int & y)
{
	pRender->SetBold( m_bOldValue );
}

void CHTML_B_Element::OnRender(CHTMLRender * pRender, int & x, int & y)
{
	m_bOldValue = pRender->IsBold();
	pRender->SetBold( true );
}

///////////////////////////////////////////////////////////
CHTML_BODY_Element::CHTML_BODY_Element( CHTMLElementsBase * pParent )
	:CHTMLContainerBase( pParent )
{
	m_colorBg = 0;
	m_colorText = 0xFFFFFF;
	m_colorALink = 0xFF6060;				//	需要反过来看
	m_colorVLink = 0xFF00FF;

#if defined(_WIN32) || defined(__FOR_MICROWIN_EMBED__)||defined(__FOR_MY_OSD_EMBED__)
	m_pImageObj = NULL;
#endif //_WIN32
}

CHTML_BODY_Element::~CHTML_BODY_Element()
{
#if defined(_WIN32) || defined(__FOR_MICROWIN_EMBED__)||defined(__FOR_MY_OSD_EMBED__)
	if( m_pImageObj )
		m_pImageObj->Release();
#endif //_WIN32
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		设置3种颜色，作背景图
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_BODY_Element::OnRender(CHTMLRender * pRender, int & x, int & y)
{
	CHTMLContainerBase::OnRender( pRender, x, y );

	CDC * pDC = pRender->GetDC();

	pDC->FillSolidRect( 0, 0, pRender->GetWidth(), pRender->GetHeight(), m_colorBg );

	pDC->SetBkColor( m_colorBg );

	pRender->SetTextColor( m_colorText );
	pRender->SetLinkTextColor( m_colorALink );	
	pRender->SetActiveLinkTextColor( m_colorVLink );

#if defined(_WIN32) || defined(__FOR_MICROWIN_EMBED__)
	if( m_pImageObj )
		m_pImageObj->Draw( pRender->GetDC()->m_hDC, x, y );	
#elif defined(__FOR_MY_OSD_EMBED__)
	if( m_pImageObj )
		pRender->GetDC()->DrawImage( x, y, 0, 0, m_pImageObj );
#else
	!!!!!...
#endif //_WIN32
}

void CHTML_BODY_Element::OnEndRender(CHTMLRender * pRender, int & x, int & y)
{
	x = 0;
	y = 0;
}

bool CHTML_BODY_Element::SetParameter( CMyString & strParam )
{
	strParam.MakeUpper();

	CMyString strTmp = GetOneParameterItem( strParam, "BGCOLOR", '=' );
	if( strTmp.IsEmpty() )
		strTmp = "#000000";
	m_colorBg = GetValueAsColor( strTmp );	

	strTmp = GetOneParameterItem( strParam, "TEXT", '=' );
	if( strTmp.IsEmpty() )
		strTmp = "#FFFFFF";
	m_colorText = GetValueAsColor( strTmp );	

	strTmp = GetOneParameterItem( strParam, "ALINK", '=' );
	if( strTmp.IsEmpty() )
		strTmp = "0xFF6060";
	m_colorALink = GetValueAsColor( strTmp );	

	strTmp = GetOneParameterItem( strParam, "VLINK", '=' );
	if( strTmp.IsEmpty() )
		strTmp = "#FF00FF";
	m_colorVLink = GetValueAsColor( strTmp );

	m_strBgImgFileName = GetOneParameterItem( strParam, "BACKGROUND", '=' );

	return true;
}

#ifdef _DEBUG
void CHTML_BODY_Element::Dump()
{ 
	CHTMLElementsBase::Dump(); 
	CMyString strColor[4];
	strColor[0] = FormatColor(m_colorBg);
	strColor[1] = FormatColor(m_colorText);
	strColor[2] = FormatColor(m_colorALink);
	strColor[3] = FormatColor(m_colorVLink);
	TRACE("<BODY bgcolor=%s text=%s alink=%s vlink=%s background=%s>\n", 
		(LPCSTR)strColor[0], (LPCSTR)strColor[1], 
		(LPCSTR)strColor[2], (LPCSTR)strColor[3], (char*)m_strBgImgFileName ); 
}
#endif //_DEBUG
	
///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		get file name
/// Input parameter:
///		None
/// Output parameter:
///		None
CMyString CHTML_BODY_Element::GetFileName()
{
	return m_strBgImgFileName;
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		put image data
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTML_BODY_Element::SetImageData( PBYTE pBuf, int nLen )
{
#if defined(_WIN32) || defined(__FOR_MICROWIN_EMBED__)||defined(__FOR_MY_OSD_EMBED__)
	if( m_pImageObj )
		m_pImageObj->Release();
	m_pImageObj = MyLoadImage( pBuf, nLen, m_pDC, m_strBgImgFileName );
	if( NULL == m_pImageObj )
		return;
	m_cx = m_pImageObj->GetWidth();
	m_cy = m_pImageObj->GetHeight();

#else

	!!!!!!!!............

#endif //_WIN32
}


///////////////////////////////////////////////////////////////////////
// LINE element
CHTML_LINE_Element::CHTML_LINE_Element(CHTMLElementsBase * pParent)
: CHTMLElementsBase( pParent )
{
	m_x1 = m_y1 = m_x2 = m_y2 = m_nWidth = 0;
	m_Color = 0;
}

CHTML_LINE_Element::~CHTML_LINE_Element()
{

}

void CHTML_LINE_Element::OnRender(CHTMLRender * pRender, int & x, int & y)
{
	CDC * pDC = pRender->GetDC();
	
	CPen penTmp;
	penTmp.CreatePen( PS_SOLID, m_nWidth, m_Color );
#ifdef _WIN32	
	CPen * penOld = pDC->SelectObject( &penTmp );
#else
	HPEN hOldPen = pDC->SelectObject( &penTmp );
#endif //_WIN32	

	pDC->MoveTo( m_x1, m_y1 );
	pDC->LineTo( m_x2, m_y2 );
	
#ifdef _WIN32
	pDC->SelectObject( penOld );
#else
	pDC->SelectObject( hOldPen );
#endif //_WIN32	
}


bool CHTML_LINE_Element::SetParameter( CMyString & strParam )
{
	CMyString strTmp = GetOneParameterItem( strParam, "COLOR", '=' );
	if( strTmp.IsEmpty() )
		strTmp = "#000000";
	m_Color = GetValueAsColor( strTmp );	

	strTmp = GetOneParameterItem( strParam, "X1", '=' );
	if( strTmp.IsEmpty() )
		m_x1 = 0;
	else
		m_x1 = strtoul( (char*)strTmp, NULL, 10 );

	strTmp = GetOneParameterItem( strParam, "Y1", '=' );
	if( strTmp.IsEmpty() )
		m_y1 = 0;
	else
		m_y1 = strtoul( (char*)strTmp, NULL, 10 );

	strTmp = GetOneParameterItem( strParam, "X2", '=' );
	if( strTmp.IsEmpty() )
		m_x2 = 600;
	else
		m_x2 = strtoul( (char*)strTmp, NULL, 10 );

	strTmp = GetOneParameterItem( strParam, "Y2", '=' );
	if( strTmp.IsEmpty() )
		m_y2 = m_y1;
	else
		m_y2 = strtoul( (char*)strTmp, NULL, 10 );

	strTmp = GetOneParameterItem( strParam, "WIDTH", '=' );
	if( strTmp.IsEmpty() )
		m_nWidth = 1;
	else
	{
		m_nWidth = strtoul( (char*)strTmp, NULL, 10 );	
		if( m_nWidth < 1 )
			m_nWidth = 1;
	}

	if( m_x2 < m_x1 )
	{
		int x = m_x1;
		m_x1 = m_x2;
		m_x2 = x;
	}
	if( m_y2 < m_y1 )
	{
		int y = m_y1;
		m_y1 = m_y2;
		m_y2 = y;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
// RECT element
CHTML_RECT_Element::CHTML_RECT_Element(CHTMLElementsBase * pParent)
	: CHTMLElementsBase( pParent )
{
	m_x1 = m_y1 = m_x2 = m_y2 = 0;
	m_Color = 0;
}

CHTML_RECT_Element::~CHTML_RECT_Element()
{
}

void CHTML_RECT_Element::OnRender(CHTMLRender * pRender, int & x, int & y)
{
	CDC * pDC = pRender->GetDC();
	pDC->FillSolidRect( m_x1, m_y1, m_x2-m_x1, m_y2-m_y1, m_Color );
}

bool CHTML_RECT_Element::SetParameter( CMyString & strParam )
{
	CMyString strTmp = GetOneParameterItem( strParam, "COLOR", '=' );
	if( strTmp.IsEmpty() )
		strTmp = "#000000";
	m_Color = GetValueAsColor( strTmp );	

	strTmp = GetOneParameterItem( strParam, "X1", '=' );
	if( strTmp.IsEmpty() )
		m_x1 = 0;
	else
		m_x1 = strtoul( (char*)strTmp, NULL, 10 );

	strTmp = GetOneParameterItem( strParam, "Y1", '=' );
	if( strTmp.IsEmpty() )
		m_y1 = 0;
	else
		m_y1 = strtoul( (char*)strTmp, NULL, 10 );

	strTmp = GetOneParameterItem( strParam, "X2", '=' );
	if( strTmp.IsEmpty() )
		m_x2 = 600;
	else
		m_x2 = strtoul( (char*)strTmp, NULL, 10 );

	strTmp = GetOneParameterItem( strParam, "Y2", '=' );
	if( strTmp.IsEmpty() )
		m_y2 = m_y1;
	else
		m_y2 = strtoul( (char*)strTmp, NULL, 10 );

	if( m_x2 < m_x1 )
	{
		int x = m_x1;
		m_x1 = m_x2;
		m_x2 = x;
	}
	if( m_y2 < m_y1 )
	{
		int y = m_y1;
		m_y1 = m_y2;
		m_y2 = y;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////
// class CHTML_P_Element
///////////////////////////////////////////////////////////////////////
CHTML_P_Element::CHTML_P_Element(CHTMLElementsBase * pParent)
	: CHTMLElementsBase( pParent )
{
	m_Color = INVALID_COLOR_VALUE;
	m_BkColor = INVALID_COLOR_VALUE;
	m_nOldOutputBkMode = TRANSPARENT;
}

CHTML_P_Element::~CHTML_P_Element()
{
}

///-------------------------------------------------------
/// CYJ,2005-8-2
/// 函数功能:
///		显示
/// 输入参数:
///		无
/// 返回参数:
///		无
void CHTML_P_Element::OnRender(CHTMLRender * pRender, int & x, int & y)
{
	CDC * pDC = pRender->GetDC();
	if( NULL == pDC )
		return;
	if( m_BkColor != INVALID_COLOR_VALUE )
	{
		m_nOldOutputBkMode = pDC->SetBkMode( OPAQUE );		
		m_OldBkColor = pDC->SetBkColor( m_BkColor );
	}
	if( m_Color != INVALID_COLOR_VALUE )
	{
		m_OldColor = pRender->GetTextColor();
		pRender->SetTextColor( m_Color );
	}
}

///-------------------------------------------------------
/// CYJ,2005-8-2
/// 函数功能:
///		结束显示，恢复现状
/// 输入参数:
///		无
/// 返回参数:
///		无
void CHTML_P_Element::OnEndRender(CHTMLRender * pRender, int & x, int & y)
{
	CDC * pDC = pRender->GetDC();
	if( NULL == pDC )
		return;
	if( m_BkColor != INVALID_COLOR_VALUE )
	{
		pDC->SetBkMode( m_nOldOutputBkMode );
		pDC->SetBkColor( m_OldBkColor );
	}
	if( m_Color != INVALID_COLOR_VALUE )
		pRender->SetTextColor( m_OldColor );
}

///-------------------------------------------------------
/// CYJ,2005-8-2
/// 函数功能:
///		设置参数
/// 输入参数:
///		无
/// 返回参数:
///		无
///	参数有：
///		Style
bool CHTML_P_Element::SetParameter( CMyString & strParam )
{
	CMyString strTmp = GetOneParameterItem( strParam, "TEXTCOLOR", ':' );
	if( strTmp.IsEmpty() )
		m_Color = INVALID_COLOR_VALUE;
	else
		m_Color = GetValueAsColor( strTmp );

	strTmp = GetOneParameterItem( strParam, "BGCOLOR", ':' );
	if( strTmp.IsEmpty() )
		m_BkColor = INVALID_COLOR_VALUE;
	else
		m_BkColor = GetValueAsColor( strTmp );

	return true;
}
