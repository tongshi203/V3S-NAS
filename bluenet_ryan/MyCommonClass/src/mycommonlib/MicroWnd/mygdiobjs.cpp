///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2004-11-26
///
///		用途：
///			我的 Windows GDI 对象
///=======================================================

// MyGDIObjs.cpp: implementation of the CMyGDIObjs class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mygdiobjs.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDC::CDC(HDC hDC)
{
	m_hDC = hDC;
	m_bIsDCAttached = ( hDC != NULL );
}

CDC::~CDC()
{
	if( m_hDC )
	{
		if( m_bIsDCAttached )
			Detach();
		else
			DeleteDC();
	}
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		设置引用的 hDC
/// Input parameter:
///		None
/// Output parameter:
///		None
BOOL CDC::Attach(HDC hDC)
{
	DeleteDC();
	m_hDC = hDC;
	m_bIsDCAttached = TRUE;
	return true;
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		释放 hDC
/// Input parameter:
///		None
/// Output parameter:
///		None
HDC CDC::Detach()
{
	m_bIsDCAttached = FALSE;
	HDC hRetVal = m_hDC;
	m_hDC = (HDC)NULL;
	return hRetVal;
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		delete DC
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDC::DeleteDC()
{
	if( m_hDC )
	{
		::DeleteDC( m_hDC );
		m_hDC = (HDC)NULL;
	}
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		设置字体颜色
/// Input parameter:
///		None
/// Output parameter:
///		None
COLORREF CDC::SetTextColor(COLORREF newColor)
{
	return ::SetTextColor( m_hDC, newColor );
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		Set Background color
/// Input parameter:
///		None
/// Output parameter:
///		None
COLORREF CDC::SetBkColor(COLORREF newColor)
{
	return ::SetBkColor( m_hDC, newColor );
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		Set bk mode
/// Input parameter:
///		None
/// Output parameter:
///		None
int CDC::SetBkMode(int nMode)
{
	return ::SetBkMode( m_hDC, nMode );
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		Set text align
/// Input parameter:
///		None
/// Output parameter:
///		None
UINT CDC::SetTextAlign(UINT fMode)
{
	return ::SetTextAlign( m_hDC, fMode );
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		Move To
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDC::MoveTo(int x, int y)
{
	POINT tmp;
	::MoveToEx( m_hDC, x, y, &tmp );
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		Move To Ex
/// Input parameter:
///		None
/// Output parameter:
///		None
BOOL CDC::MoveToEx(int x, int y, LPPOINT lpPoint)
{
	return ::MoveToEx( m_hDC, x, y, lpPoint );
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		Line To
/// Input parameter:
///		None
/// Output parameter:
///		None
BOOL CDC::LineTo(int x, int y)
{
	return ::LineTo( m_hDC, x, y );
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		Polyline
/// Input parameter:
///		None
/// Output parameter:
///		None
BOOL CDC::Polyline(const POINT *lppt, int nPoints)
{
	return ::Polyline( m_hDC, lppt, nPoints );
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///
/// Input parameter:
///		None
/// Output parameter:
///		None
BOOL CDC::PolyPolygon(const POINT *lpPoints, LPINT lpPolyCounts, int nCount)
{
	return ::PolyPolygon( m_hDC, lpPoints, lpPolyCounts, nCount );
}

BOOL CDC::Rectangle(int nLeft, int nTop, int nRight,int nBottom)
{
	return ::Rectangle(m_hDC, nLeft, nTop, nRight, nBottom);
}

BOOL CDC::Ellipse(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	return ::Ellipse(m_hDC, nLeftRect, nTopRect, nRightRect, nBottomRect);
}

BOOL CDC::Arc(int nLeftRect, int nTopRect, int nRightRect,
		int nBottomRect, int nXStartArc, int nYStartArc,
		int nXEndArc, int nYEndArc)
{
	return ::Arc(m_hDC, nLeftRect, nTopRect, nRightRect,\
		nBottomRect, nXStartArc, nYStartArc, nXEndArc, nYEndArc);
}

BOOL CDC::Pie(int nLeftRect, int nTopRect, int nRightRect,
		int nBottomRect, int nXRadial1, int nYRadial1,
		int nXRadial2, int nYRadial2)
{
	return ::Pie(m_hDC, nLeftRect, nTopRect, nRightRect,\
		nBottomRect, nXRadial1, nYRadial1, nXRadial2, nYRadial2 );
}

BOOL CDC::Polygon(CONST POINT *lpPoints, int nCount)
{
	return ::Polygon(m_hDC, lpPoints, nCount);
}

int CDC::FillRect(CONST RECT *lprc, HBRUSH hbr)
{
	return ::FillRect(m_hDC, lprc, hbr);
}

BOOL CDC::TextOut(int x, int y, LPCSTR lpszString, int cbString)
{
	return ::TextOut(m_hDC, x, y, lpszString, cbString);
}

BOOL CDC::ExtTextOut(int x, int y, UINT fuOptions,
		CONST RECT *lprc, LPCSTR lpszString, UINT cbCount,
		CONST INT *lpDx)
{
	return ::ExtTextOut(m_hDC, x, y, fuOptions, lprc, lpszString, cbCount, lpDx );
}

int CDC::DrawText(LPCSTR lpString, int nCount, LPRECT lpRect, UINT uFormat)
{
	return ::DrawText(m_hDC, lpString, nCount, lpRect, uFormat);
}

#ifdef __FOR_MICROWIN_EMBED__
///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		Draw DIB, MicroWindow only
/// Input parameter:
///		None
/// Output parameter:
///		None
BOOL CDC::DrawDIB(int x, int y,PMWIMAGEHDR pimage)
{
	return ::DrawDIB(m_hDC, x, y, pimage);
}
#endif //_WIN32


BOOL CDC::CreateCompatibleDC( CDC * pDC )
{
	HDC hDC = ::CreateCompatibleDC( m_hDC );
	if( NULL == hDC )
		return FALSE;
	pDC->m_hDC = hDC;
	return TRUE;
}

BOOL CDC::BitBlt(int nXDest,int nYDest,int nWidth,int nHeight,
			CDC * pSrcDC,int nXSrc,int nYSrc,DWORD dwRop)
{
	return ::BitBlt(m_hDC, nXDest,nYDest,nWidth,nHeight, pSrcDC->m_hDC,nXSrc,nYSrc,dwRop );
}

BOOL CDC::StretchBlt(int nXOriginDest,int nYOriginDest,
		int nWidthDest,int nHeightDest,CDC * pSrcDC,
		int nXOriginSrc,int nYOriginSrc,int nWidthSrc,
		int nHeightSrc, DWORD dwRop)
{
	return ::StretchBlt(m_hDC, nXOriginDest,nYOriginDest, nWidthDest,nHeightDest,\
		pSrcDC->m_hDC, nXOriginSrc,nYOriginSrc,nWidthSrc, nHeightSrc, dwRop );
}

HGDIOBJ CDC::SelectObject( CGdiObject * pObject )
{
	return ::SelectObject( m_hDC, pObject->m_hObject );
}

HGDIOBJ CDC::SelectObject( HGDIOBJ hGdiObj )
{
	return ::SelectObject( m_hDC, hGdiObj );
}

HGDIOBJ  CDC::SelectStockObject( int nIndex )
{
	HGDIOBJ hNewValue = ::GetStockObject( nIndex );
	if( NULL == hNewValue )
		return (HGDIOBJ)NULL;
	return ::SelectObject( m_hDC, hNewValue );
}

int CDC::FillSolidRect( int x, int y, int cx, int cy, COLORREF crColor )
{
	RECT rectTmp={ x, y, cx+x, cy+y };
	::SetBkColor( m_hDC, crColor );
	return ::ExtTextOut( m_hDC, 0, 0, ETO_OPAQUE, &rectTmp, NULL, 0, NULL );
}

int CDC::FillSolidRect( LPRECT lpRect, COLORREF crColor )
{
	if( NULL == lpRect )
		return 0;
	::SetBkColor( m_hDC, crColor );
	return ::ExtTextOut( m_hDC, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL );
}
	
int CDC::SaveDC()
{
	HDC pSaved = new hdc;
	if( NULL == pSaved )
		return 0;
	memcpy( pSaved, m_hDC, sizeof(hdc) );
	return (int)pSaved;
}

void CDC::RestoreDC(int nSavedDC)
{
	HDC pSaved = (HDC)nSavedDC;
	if( NULL == pSaved )
		return;
	memcpy( m_hDC, pSaved, sizeof(hdc) );
	delete pSaved;
}


///////////////////////////////////////////////////////////
// CGdiObject
CGdiObject::CGdiObject()
{
	m_hObject = (HGDIOBJ)NULL;
}

CGdiObject::~CGdiObject()
{
	DeleteObject();
}

void CGdiObject::DeleteObject()
{
	if( m_hObject )
		::DeleteObject( m_hObject );
	m_hObject = (HGDIOBJ)NULL;
}

BOOL CGdiObject::Attach( HGDIOBJ hObject )
{
	if( m_hObject )
		return FALSE;
	m_hObject = hObject;
	return TRUE;
}

HGDIOBJ CGdiObject::Detach()
{
	HGDIOBJ hRetVal = m_hObject;
	m_hObject = (HGDIOBJ)NULL;
	return hRetVal;
}

///////////////////////////////////////////////////////////
// CPen
CPen::CPen()
{
}

CPen::CPen( int nPenStyle, int nWidth, COLORREF crColor )
{
	CreatePen( nPenStyle, nWidth, crColor );
}

CPen::~CPen()
{
}

BOOL CPen::CreatePen( int nPenStyle, int nWidth, COLORREF crColor )
{
	m_hObject = (HGDIOBJ)::CreatePen( nPenStyle, nWidth, crColor );
	return ( m_hObject != NULL );
}

////////////////////////////////////////////////////////////
CFont::CFont()
{
}

CFont::~CFont()
{
}

BOOL CFont::CreateFont( int nHeight, int nWidth, int nEscapement, int nOrientation, \
		int nWeight, BYTE bItalic, BYTE bUnderline, BYTE cStrikeOut, BYTE nCharSet, \
		BYTE nOutPrecision, BYTE nClipPrecision, BYTE nQuality, BYTE nPitchAndFamily, \
		LPCTSTR lpszFacename )
{
	m_hObject = (HGDIOBJ)::CreateFont( nHeight, nWidth, nEscapement, nOrientation, \
		nWeight, bItalic, bUnderline, cStrikeOut, nCharSet, \
		nOutPrecision, nClipPrecision, nQuality, nPitchAndFamily, \
		lpszFacename );
	return ( m_hObject != NULL );
}

///////////////////////////////////////////////////////
CBitmap::CBitmap()
{
}

CBitmap::~CBitmap()
{
}

BOOL CBitmap::CreateCompatibleBitmap( CDC * pDC, int nWidth, int nHeight)
{
	m_hObject = (HGDIOBJ)::CreateCompatibleBitmap( pDC->m_hDC, nWidth, nHeight );
	return (m_hObject != NULL );
}

CBrush::CBrush()
{
}

CBrush::~CBrush()
{
}

BOOL CBrush::CreateSolidBrush( COLORREF crColor )
{
	m_hObject = (HGDIOBJ)::CreateSolidBrush( crColor );
	return ( NULL != m_hObject );
}
