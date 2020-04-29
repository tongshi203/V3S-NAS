// MyGDIObjs.h: interface for the CMyGDIObjs class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYGDIOBJS_H__41E91C63_F1C9_4BF6_82ED_1479D8EBBFC9__INCLUDED_)
#define AFX_MYGDIOBJS_H__41E91C63_F1C9_4BF6_82ED_1479D8EBBFC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

class CPen;
class CFont;
class CBitmap;
class CBrush;
class CGdiObject;

class IMyMpegDecoder;
void GDI_Initialize(IMyMpegDecoder * pMpegDecoder=NULL);
void GDI_Close();
void GDI_Reset();


const int NOT_PHY_DC = 10000;
class CDC
{
public:
// by yuanlii@29bbs.net, 3/ 7/2005 11:52:58
	/* only physic dc could refresh */
	unsigned long RefreshOSDBuffer(void);
// by yuanlii@29bbs.net

	int FillSolidRect( int x, int y, int cx, int cy, COLORREF crColor );
	int FillSolidRect( LPRECT lpRect, COLORREF crColor );

	BOOL BitBlt(int nXDest,int nYDest,int nWidth,int nHeight,
			CDC * pSrcDC,int nXSrc,int nYSrc,DWORD dwRop);
	BOOL StretchBlt(int nXOriginDest,int nYOriginDest,
			int nWidthDest,int nHeightDest,CDC * pSrcDC,
			int nXOriginSrc,int nYOriginSrc,int nWidthSrc,
			int nHeightSrc, DWORD dwRop);
	BOOL CreateCompatibleDC( CDC * pDC );

	int DrawText(LPCSTR lpString, int nCount, LPRECT lpRect,
			UINT uFormat);
// microwin 
	void SetTransparent(MWCOORD x, MWCOORD y, MWCOORD w, MWCOORD h, BYTE c);
/*display the image to the screen directly, set w or h to 0 will
 display the picture in it's own size */
	BOOL DrawImage(MWCOORD left, MWCOORD top, 
			MWCOORD width, MWCOORD height,
		       	char *filename);
/* display the CBitmap object to Mem DC, set w or h to 0 will
   display the picture in it's own size */
	BOOL DrawImage(MWCOORD left, MWCOORD top,	
			MWCOORD width, MWCOORD height,
		       	CBitmap *pBitmap); 
	bool IsDrawImageToYUV()const;
	void SetDrawImageToYUV( bool bToYUV=true);

	void	DrawHugeNumber(MWCOORD x, MWCOORD y,
		       	char *szNumber, int nCount);
	void	TextOut_S(MWCOORD x, MWCOORD y, char *szString, int nCount);
// microwin
	BOOL TextOut( int x, int y, LPCSTR lpszString, int cbString);
	BOOL ExtTextOut( int x, int y, UINT fuOptions,
			CONST RECT *lprc, LPCSTR lpszString, UINT cbCount,
			CONST INT *lpDx);
	BOOL Rectangle(int nLeft, int nTop, int nRight,int nBottom);
	BOOL Ellipse(int nLeftRect, int nTopRect, int nRightRect,
			int nBottomRect);
	BOOL Arc(int nLeftRect, int nTopRect, int nRightRect,
			int nBottomRect, int nXStartArc, int nYStartArc,
			int nXEndArc, int nYEndArc);
	BOOL Pie(int nLeftRect, int nTopRect, int nRightRect,
			int nBottomRect, int nXRadial1, int nYRadial1,
			int nXRadial2, int nYRadial2);
	BOOL Polygon(CONST POINT *lpPoints, int nCount);
	int FillRect(CONST RECT *lprc, HBRUSH hbr);
	BOOL PolyPolygon( const POINT * lpPoints, LPINT lpPolyCounts, int nCount );
	BOOL Polyline( const POINT * lppt, int nPoints  );
	BOOL LineTo( int x, int y);
	BOOL MoveToEx( int x, int y, LPPOINT lpPoint );
	void MoveTo( int x, int y );
	UINT SetTextAlign( UINT fMode );
	int SetBkMode( int nMode );
	COLORREF SetBkColor( COLORREF newColor );
	COLORREF SetTextColor( COLORREF newColor );
	void DeleteDC();
	void ResetDC(int x, int y, int cx, int cy);
	HDC Detach();
	BOOL Attach( HDC hDC );
	int SaveDC();
	void RestoreDC(int nSavedDC);

	HGDIOBJ SelectObject( CGdiObject * pObject );
// by yuanlii@29bbs.net, 2/ 2/2005 20:15:23
	HPEN	SelectObject(CPen *pPen);
	HBRUSH	SelectObject(CBrush *pBrush);
	HBITMAP	SelectObject(CBitmap *pBitmap);
	HFONT	SelectObject(CFont *pFont);
// by yuanlii@29bbs.net
	HGDIOBJ SelectObject( HGDIOBJ hGdiObj );

/* Stock objects current supported */
//#define WHITE_BRUSH         0
//#define LTGRAY_BRUSH        1
//#define GRAY_BRUSH          2
//#define DKGRAY_BRUSH        3
//#define BLACK_BRUSH         4
//#define NULL_BRUSH          5
//#define HOLLOW_BRUSH        NULL_BRUSH
//#define WHITE_PEN           6
//#define BLACK_PEN           7
//#define NULL_PEN            8
//#define DEFAULT_PALETTE     9		/* FIXME */
//#define STOCK_LAST          9
	HGDIOBJ SelectStockObject( int nIndex );

	CDC( HDC hDC );
//Micro Windows
	CDC( int x=0, int y=0, int cx=-1, int cy=-1 );
//Micro Windows
	virtual ~CDC();

public:
	HDC		m_hDC;

private:
	BOOL	m_bIsDCAttached;
};

////////////////////////////////////////////////////
class CGdiObject
{
public:
	CGdiObject();
	virtual ~CGdiObject();
	void DeleteObject();
	BOOL Attach( HGDIOBJ hObject );
	HGDIOBJ Detach();
public:
	HGDIOBJ m_hObject;
};

////////////////////////////////////////////////////
// by yuanlii@29bbs.net, 2/ 2/2005 18:40:12
// the width of the pen could only be 1
// the style of the pen could only be solid or null
class CPen : public CGdiObject
{
public:
	CPen();		// don't forget to use CPen.CreatePen to initialize the Pen
	CPen( int nPenStyle, int nWidth, COLORREF crColor );	// needn't any initialization, but this function
								// may not success, not recommend
	virtual ~CPen();
	BOOL CreatePen( int nPenStyle, int nWidth, COLORREF crColor );
	operator HPEN() const { return (HPEN)m_hObject; }
};

///////////////////////////////////////////////////////
class CBrush : public CGdiObject
{
public:
	CBrush();
	virtual ~CBrush();
	BOOL CreateSolidBrush( COLORREF crColor );
};

///////////////////////////////////////////////////////
class CBitmap : public CGdiObject
{
public:
	CBitmap();	// if use this constructor, must use LoadBitmap to initialize it
	CBitmap(CDC *pdc, char *filename);// needn't LoadBitmap, but may not return success, not recommend
	virtual ~CBitmap();
	int LoadBitmap(CDC *pdc, char *filename);
	int LoadBitmap( CDC * pdc, PBYTE pBuf, int nLen );
	MWBOOL GetBitmap(PMWIMAGEINFO pii);
	void DeleteObject();
	int GetImageId(){return m_nImageId;}
	BOOL CreateCompatibleBitmap( CDC * pDC, int nWidth, int nHeight);
	void Draw( CDC * pDC, int x, int y, int nWidth=0, int nHeight=0);
	operator HBITMAP() const { return (HBITMAP)m_hObject; }
	int GetWidth(){ return m_ImgInfo.width; }
	int GetHeight(){ return m_ImgInfo.height; }
private:
	int	m_nImageId;
	MWIMAGEINFO	m_ImgInfo;
};
///////////////////////////////////////////////////////
class CFont : public CGdiObject
{
public:
	CFont();	// must use CreatePointFont to create a font
	virtual ~CFont();
	BOOL CreatePointFont(CDC *pDC, int nPointSize, char *lpszFaceName);
	void DeleteObject();
	operator HFONT () const { return (HFONT )m_hObject; }
public:
	int		m_nPointSize;
};

#endif // !defined(AFX_MYGDIOBJS_H__41E91C63_F1C9_4BF6_82ED_1479D8EBBFC9__INCLUDED_)
