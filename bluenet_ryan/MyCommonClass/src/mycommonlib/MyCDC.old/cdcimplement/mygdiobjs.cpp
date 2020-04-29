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

#include <stdio.h>
#include <assert.h>
#include <string.h>	// for memcpy and the conversion from `void*' to `const char*'
#include <stdlib.h>
#include "mygdiobjs.h"
extern "C" {
	#include <device.h>
	#include "wintern.h"
}
#include <mympegdec.h>
#include <yuvimage.h>

extern IMyMpegDecoder * gr_pDecoder;
extern MWBOOL gr_toosd;

#ifdef	_DEBUG
#include <sys/time.h>
#endif/*_DEBUG*/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Micro Windows

void GDI_Initialize(IMyMpegDecoder * pMpegDecoder)
{
	gr_pDecoder = pMpegDecoder;
	MwInitialize();
}

void GDI_Close()
{
	MwTerminate();
}

void GDI_Reset()
{
	MwReset();
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		构造 CDC
/// Input parameter:
///		x				Left
///		y				Top
///		cx				width, default is -1, automatic detect
///		cy				height, default is -1, automatic detect
/// Output parameter:
///		None
CDC::CDC( int x, int y, int cx, int cy )
{
	if( cx <= 0 || cy <= 0 )
	{
		if( cx <= 0 )
			cx = GetDeviceCaps( NULL,HORZRES ) - 1 - x;
		if( cy <= 0 )
			cy = GetDeviceCaps( NULL,VERTRES ) - 1 - y;
	}
	m_hDC = BeginPaint(x, y, x+cx, y+cy);
	m_bIsDCAttached = false;
}
// Micro windows

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

void CDC::ResetDC(int x, int y, int cx, int cy)
{
	DeleteDC();

	if( cx <= 0 || cy <= 0 )
	{
		if( cx <= 0 )
			cx = GetDeviceCaps( NULL,HORZRES ) - 1 - x;
		if( cy <= 0 )
			cy = GetDeviceCaps( NULL,VERTRES ) - 1 - y;
	}
	m_hDC = BeginPaint(x, y, x+cx, y+cy);
	m_bIsDCAttached = false;
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

/***
 * @ingroup CDC
 * 	DrawHugeNumber draw a huge number string
 * @param x
 * 	  y	the position of the string drawed
 * @param szNumber, the huge number string to be drawed
 * @nCount string length of szNumber
 *
 * some features:
 * 	1. Green textcolor
 * 	2. Black border
 * 	3. Transparent background
 * 	4. Didn't affect the textcolor and the bgcolor(fixed color display)
 */
void
CDC::DrawHugeNumber(MWCOORD x, MWCOORD y, char *szNumber, int nCount)
{
	::GdDrawHugeNumber(m_hDC->psd, x, y, szNumber, nCount);
}

/***
 * @ingroup CDC
 * 	TextOut_S draw a string with border
 * @param x
 * 	  y	the position of the string drawed
 * @param szString, the string to be drawed
 * @nCount string length of szString
 * some features:
 * 	1. Green textcolor
 * 	2. Black border
 * 	3. Transparent background
 * 	4. Didn't affect the textcolor and the bgcolor(fixed color display)
 */
void
CDC::TextOut_S(MWCOORD x, MWCOORD y, char *szString, int nCount)
{
	::GdText_S(m_hDC->psd, x, y, szString, nCount);
}

int CDC::DrawText(LPCSTR lpString, int nCount, LPRECT lpRect, UINT uFormat)
{
	return ::DrawText(m_hDC, lpString, nCount, lpRect, uFormat);
}

// Micro Windows
///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		display the image to the screen directly,
///		if display it to the screen dc, use this function
/// Input parameter:
///		left, top	the position to be draw the image
///		width, height	the scale of the picture, set the width or the height to 0, display the
///				picture as it's own size
///		filename	the file which to be draw
/// Output parameter:
///		BOOL		TRUE when success, otherwise FALSE
BOOL CDC::DrawImage(MWCOORD left, MWCOORD top, MWCOORD width, MWCOORD height,
	       	char *filename)
{	
	int		nImageId;
	MWIMAGEINFO	ii;
	POINT		beg;

	beg.x = left;
	beg.y = top;
	ClientToScreen(m_hDC, &beg);

	nImageId = GdLoadImageFromFile(m_hDC->psd, filename, 0);
	if (nImageId != 0) {
		if ((width > 0) && (height > 0)) {
			GdDrawImageToFit(m_hDC->psd, beg.x, beg.y, width, height, nImageId);
		} else {
			GdGetImageInfo(nImageId, &ii);
			GdDrawImageToFit(m_hDC->psd, beg.x, beg.y, ii.width, ii.height, nImageId);
			GdFreeImage(nImageId);
		}
		return TRUE;
	}
	else
		return FALSE;
}

//--------------------------------------------------------------
//	获取当前图形输出是否到 YUV
bool CDC::IsDrawImageToYUV()const
{
	return !gr_toosd;
}

//----------------------------------------------------------------
// 设资图形输出到 YUV
void CDC::SetDrawImageToYUV(bool bToYUV)
{
	gr_toosd = !bToYUV;
}

///-------------------------------------------------------
/// by yuanlii@29bbs.net, 2/ 3/2005 13:48:57
/// Function:
///		display the image to the Mem DC
///		if buffer it to the Mem dc, use this function
/// Input parameter:
///		left, top	the position to be draw the image
///		width, height	the scale of the picture, set the width or the height to 0, display the
///				picture as it's own size
///		pBitmap		the pointer to the CBitmap which will be displayed
///				the CBitmap object must use have initialized with CBitmap::LoadBitmap
///				or the constructor CBitmap(CDC *pdc, char *filename)
/// Output parameter:
///		BOOL		TRUE when success, otherwis FALSE
BOOL CDC::DrawImage(MWCOORD left, MWCOORD top, MWCOORD width, MWCOORD height,
	       	CBitmap *pBitmap)
{
	MWIMAGEINFO	ii;
	POINT		beg;

#ifdef	_DEBUG
	long		lbegin, lend;
	struct timeval	begin, end;
	struct timezone	tz;
	gettimeofday(&begin, &tz);
	lbegin = begin.tv_sec * 1000000 + begin.tv_usec;
#endif/*_DEBUG*/

	beg.x = left;
	beg.y = top;
	ClientToScreen(m_hDC, &beg);

	if ((pBitmap->GetImageId()) != 0) {
		if ((width > 0) && (height > 0)) {
			GdDrawImageToFit(m_hDC->psd, beg.x, beg.y, width, height, pBitmap->GetImageId());
		} else {
			GdGetImageInfo(pBitmap->GetImageId(), &ii);
			GdDrawImageToFit(m_hDC->psd, beg.x, beg.y, ii.width, ii.height, pBitmap->GetImageId());
		}
#ifdef	_DEBUG
		gettimeofday(&end, &tz);
		lend = end.tv_sec * 1000000 + end.tv_usec;
		printf("cost %fs\n", ((double)lend - (double)lbegin) / 1000000);
#endif/*_DEBUG*/
		return TRUE;
	}
	else
		return FALSE;
}
/**
 *	@CDC::RefreshOSDBuffer by yuanlii@29bbs.net, 3/ 7/2005 16:38:12
 *	 refresh the screen according the OSD buffer
 *	@param void
 *	@return -1 when not physic DC
 *		otherwise return the return value of IMpegDecoder::RefreshOSDBuffer();
 */
unsigned long CDC::RefreshOSDBuffer(void)
{
	if (m_hDC->psd->flags&PSF_SCREEN) {
		return ::gr_pDecoder->RefreshOSDBuffer();
	} else {
		fprintf(stderr, "Not a physic DC, can't refresh\n");
		return NOT_PHY_DC;
	}
}
/**
 *	@CDC::SetTransparent by yuanlii@29bbs.net, 3/ 7/2005 16:38:12
 *	 set the transparent of the rectangle
 *	@param x, y, w & h assigned the rectangle
 *	@param c is the transparent constant of the rectangle
 *		 0x00-0xff, 0x00 is full transparent, 0xff is opaque
 *	@no return value
 */
void CDC::SetTransparent(MWCOORD x, MWCOORD y, MWCOORD w, MWCOORD h, BYTE c)
{
	if (m_hDC->psd->flags&PSF_SCREEN) {
		::GdSetTransparent(x, y, w, h, c);
	} else {
		fprintf(stderr, "Not a physic DC, can't set transparent\n");
	}
}

// Micro windows


BOOL CDC::CreateCompatibleDC( CDC * pDC )
{
	HDC hDC = ::CreateCompatibleDC( pDC->m_hDC );
	if( NULL == hDC )
		return FALSE;
	m_hDC = hDC;
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

// by yuanlii@29bbs.net, 2/ 2/2005 20:15:39
HPEN CDC::SelectObject( CPen * pPen )
{
	return ::SelectObject( m_hDC, pPen->m_hObject );
}

HBRUSH CDC::SelectObject( CBrush * pBrush )
{
	return ::SelectObject( m_hDC, pBrush->m_hObject );
}

HBITMAP CDC::SelectObject( CBitmap * pBitmap )
{
	return ::SelectObject( m_hDC, pBitmap->m_hObject );
}

HFONT CDC::SelectObject(CFont *pFont)
{
	return ::SelectObject(m_hDC, pFont->m_hObject);
}

// by yuanlii@29bbs.net

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
	m_hObject = (HGDIOBJ)::CreatePen( nPenStyle, nWidth, crColor );
}

CPen::~CPen()
{
}

BOOL CPen::CreatePen( int nPenStyle, int nWidth, COLORREF crColor )
{
	m_hObject = (HGDIOBJ)::CreatePen( nPenStyle, nWidth, crColor );
	return ( m_hObject != NULL );
}

///////////////////////////////////////////////////////
// CBrush
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

///////////////////////////////////////////////////////
// CBitmap
CBitmap::CBitmap()
{
	m_nImageId = 0;
	memset( &m_ImgInfo, 0, sizeof(m_ImgInfo) );
}

CBitmap::CBitmap(CDC *pdc, char *filename)
{
	m_nImageId = GdLoadImageFromFile(pdc->m_hDC->psd, filename, 0);
	GetBitmap( &m_ImgInfo );
}

CBitmap::~CBitmap()
{
	DeleteObject();
}

int CBitmap::LoadBitmap(CDC *pdc, char *filename)
{
	m_nImageId = GdLoadImageFromFile(pdc->m_hDC->psd, filename, 0);
	GetBitmap( &m_ImgInfo );
	return (m_nImageId) ? m_nImageId : 0;
}

int CBitmap::LoadBitmap( CDC * pdc, PBYTE pBuf, int nLen )
{
	m_nImageId = GdLoadImageFromBuffer(pdc->m_hDC->psd, pBuf, nLen, 0);
	GetBitmap( &m_ImgInfo );
	return (m_nImageId) ? m_nImageId : 0;
}

MWBOOL CBitmap::GetBitmap(PMWIMAGEINFO pii)
{
	if (m_nImageId)
		return GdGetImageInfo(m_nImageId, pii);
	else
		return FALSE;
}

void CBitmap::DeleteObject()
{
	if (m_nImageId)
		GdFreeImage(m_nImageId);
	if( m_hObject )
		::DeleteObject( m_hObject );
	m_hObject = (HGDIOBJ)NULL;
}

BOOL CBitmap::CreateCompatibleBitmap( CDC * pDC, int nWidth, int nHeight)
{
	m_hObject = (HGDIOBJ)::CreateCompatibleBitmap( pDC->m_hDC, nWidth, nHeight );
	return (m_hObject != NULL );
}

void CBitmap::Draw( CDC * pDC, int x, int y, int nWidth, int nHeight)
{
	pDC->DrawImage( x, y, nWidth, nHeight, this );
}

CFont::CFont()
{
	m_nPointSize = 240;
	m_hObject = (HGDIOBJ)NULL;
}

CFont::~CFont()
{
	DeleteObject();
}

BOOL CFont::CreatePointFont(CDC *pDC, int nPointSize, char *lpszFaceName)
{
	if ( 160 != nPointSize )
		nPointSize = 240;
					// 非24点阵，则强制为16点阵
	MWFONTOBJ	*hfont;
	m_nPointSize = nPointSize;
	hfont = GdItemNew(MWFONTOBJ);
	if (!hfont)
		return FALSE;

	hfont->hdr.type = OBJ_FONT;
	hfont->hdr.stockobj = FALSE;
	hfont->pfont = GdCreateFont(pDC->m_hDC->psd, "HZKFONT", nPointSize/10, NULL);
	m_hObject = (HFONT)hfont;
	return TRUE;	
}

void CFont::DeleteObject()
{
	if ( m_hObject )
		::DeleteObject(m_hObject);
	m_hObject = (HGDIOBJ)NULL;
}

