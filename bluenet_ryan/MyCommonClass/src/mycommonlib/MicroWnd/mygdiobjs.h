// MyGDIObjs.h: interface for the CMyGDIObjs class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYGDIOBJS_H__41E91C63_F1C9_4BF6_82ED_1479D8EBBFC9__INCLUDED_)
#define AFX_MYGDIOBJS_H__41E91C63_F1C9_4BF6_82ED_1479D8EBBFC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPen;
class CFont;
class CBitmap;
class CBrush;
class CGdiObject;

class CDC
{
public:	

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
#ifdef __FOR_MICROWIN_EMBED__
	BOOL DrawDIB(int x, int y, PMWIMAGEHDR pimage );	/* microwin*/
#endif //__FOR_MICROWIN_EMBED__
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
	HDC Detach();
	BOOL Attach( HDC hDC );
	int SaveDC();
	void RestoreDC(int nSavedDC);

	HGDIOBJ SelectObject( CGdiObject * pObject );
	HGDIOBJ SelectObject( HGDIOBJ hGdiObj );
	HGDIOBJ SelectStockObject( int nIndex );

	CDC( HDC hDC = (HDC)NULL );
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
class CPen : public CGdiObject
{
public:
	CPen();
	CPen( int nPenStyle, int nWidth, COLORREF crColor );
	virtual ~CPen();
	BOOL CreatePen( int nPenStyle, int nWidth, COLORREF crColor );
	operator HPEN() const { return (HPEN)m_hObject; }
};

////////////////////////////////////////////////////
class CFont : public CGdiObject
{
public:
	CFont();
	virtual ~CFont();
	BOOL CreateFont( int nHeight, int nWidth, int nEscapement, int nOrientation, \
		int nWeight, BYTE bItalic, BYTE bUnderline, BYTE cStrikeOut, BYTE nCharSet, \
		BYTE nOutPrecision, BYTE nClipPrecision, BYTE nQuality, BYTE nPitchAndFamily, \
		LPCTSTR lpszFacename );
	operator HFONT () const { return (HFONT )m_hObject; }
};

///////////////////////////////////////////////////////
class CBitmap : public CGdiObject
{
public:
	CBitmap();
	virtual ~CBitmap();
	BOOL CreateCompatibleBitmap( CDC * pDC, int nWidth, int nHeight);
	operator HBITMAP() const { return (HBITMAP)m_hObject; }
};

///////////////////////////////////////////////////////
class CBrush : public CGdiObject
{
public:
	CBrush();
	virtual ~CBrush();
	BOOL CreateSolidBrush( COLORREF crColor );
};

#endif // !defined(AFX_MYGDIOBJS_H__41E91C63_F1C9_4BF6_82ED_1479D8EBBFC9__INCLUDED_)
