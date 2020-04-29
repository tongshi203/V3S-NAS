///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2004-11-23
///
///		用途：
///			HTML 显示
///=======================================================

#include "stdafx.h"
#include "HTMLRender.h"
#include "HTMLParser.h"

#ifdef __FOR_MICROWIN_EMBED__
  #include <mygdiobjs.h>
#endif //__FOR_MICROWIN_EMBED__

#if defined(_DEBUG) && defined(_WIN32)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHTMLRender::CHTMLRender(CDC * pDC)
{
	m_pDC = pDC;
	m_pParser = NULL;
	m_nFontSize = 5;				//	当前使用的字体大小
	m_pFont = NULL;					//	当前使用的字体指针

	m_colorText = 0xFFFFFF;			//	普通字体颜色，白的
	m_colorLinkText = 0xFF8080;		//	连接到字体颜色，蓝色
	m_colorActiveLink = 0xFF00FF;	//	当前连接

	m_strTitle = "没有标题";

	m_bIsUnderLine = false;		//	是否有下划线
	m_bIsBold = false;			//	是否粗体	
	
#if defined(_WIN32)||defined(__FOR_MICROWIN_EMBED__)
	m_Font_x_12.CreatePointFont( 160, "宋体" );
	m_Font_x_16.CreatePointFont( 240, "宋体" );
#elif defined(__FOR_MY_OSD_EMBED__)
	m_Font_x_12.CreatePointFont( pDC, 160, "宋体" );
	m_Font_x_16.CreatePointFont( pDC, 240, "宋体" );
#else
	!!!!!!!!!!!!	
#endif //_WIN32		
}

#include <stdio.h>

CHTMLRender::~CHTMLRender()
{
}

///-------------------------------------------------------
/// CYJ,2004-11-23
/// Function:
///		显示
/// Input parameter:
///		pDC				显示
///		pParser			解析器
///		nWidth			宽度
///		nHeight			高度
/// Output parameter:
///		None
void CHTMLRender::Render( CHTMLParser * pParser, int nWidth, int nHeight )
{
	m_pParser = pParser;
	m_nWinWidth = nWidth;
	m_nWinHeight = nHeight;

#ifdef _WIN32
	int nSavedEnv = m_pDC->SaveDC();
#endif //_WIN32	
	m_pDC->SelectObject( &m_Font_x_16 );	
	m_pDC->SetBkMode( TRANSPARENT );
	
	int x = 0;
	int y = 0;
	pParser->m_HTMLRootElement.SetCoordinate( 0, 0, nWidth, nHeight );
	RenderOneItem( &pParser->m_HTMLRootElement, x, y );

#ifdef _WIN32
	m_pDC->RestoreDC( nSavedEnv );
#elif defined(__FOR_MY_OSD_EMBED__)
	m_pDC->SelectObject( &m_Font_x_16 );	
#endif //_WIN32	
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		播放一个元素
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLRender::RenderOneItem( CHTMLElementsBase * pElement, int & x, int & y )
{
	pElement->OnRender( this, x, y );

	CHTMLElementsBase * pParentItem = pElement->GetParent();
	if( NULL == pParentItem )
		pParentItem = pElement;

	if( x > pParentItem->GetWidth()+pParentItem->GetX() )
	{
		y += pElement->OnNewLine();
		x = pParentItem->GetX();
	}
	CHTMLElementsBase * pChild = pElement->GetChild();
	if( pChild )
		RenderOneItem( pChild, x, y );
	pElement->OnEndRender( this, x ,y );

	CHTMLElementsBase * pNext = pElement->GetNext();
	if( pNext )
		RenderOneItem( pNext, x, y );
}

///-------------------------------------------------------
/// CYJ,2004-11-23
/// Function:
///		设置字体大小
/// Input parameter:
///		nNewSize			新的字体大小
/// Output parameter:
///		返回原来的字体大小
int CHTMLRender::SetFontSize(int nNewSize)
{
	int nRetVal = m_nFontSize;

	m_nFontSize = nNewSize;
	if( m_nFontSize >= 5 )
		m_pFont = &m_Font_x_16;
	else
		m_pFont = &m_Font_x_12;

	GetDC()->SelectObject( m_pFont );	

	return nRetVal;
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		Set text color
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLRender::SetTextColor( COLORREF newColor )
{
	m_colorText = newColor;
	GetDC()->SetTextColor( newColor );

	GetDC()->SelectObject( (CPen*) NULL );
	m_UnderLinePen.DeleteObject();
	m_UnderLinePen.CreatePen( PS_SOLID, 1, newColor );
	GetDC()->SelectObject( &m_UnderLinePen );
}