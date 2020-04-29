/*
 * Copyright (c) 1999, 2000, 2001 Greg Haerr <greg@censoft.com>
 *
 * Win32 API upper level graphics drawing routines
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <windows.h>
#include <wintern.h>
#include <device.h>

// by yuanlii@29bbs.net, 1/31/2005 16:51:23
// I should decrese this becaust I needn't so much fonts
#define MAXSTOCKOBJECTS	10	/* # of stock objects*/

//#define __DEBUG

extern MWPIXELVAL gr_foreground;
extern MWPIXELVAL gr_background;
extern PMWFONT	  gr_pfont;
extern PMWFONT g_pDefaultFont;
// by yuanlii@29bbs.net, 2/ 4/2005 16:04:05
static MWFONTOBJ	initFontObj = {
	{OBJ_FONT, FALSE}, NULL, "yuanlii Font"
};
// by yuanlii@29bbs.net

/* default bitmap for new DCs*/
static MWBITMAPOBJ default_bitmap = {
	{OBJ_BITMAP, TRUE}, 1, 1, 1, 1, 1, 1
};

static BOOL MwExtTextOut(HDC hdc, int x, int y, UINT fuOptions,
		CONST RECT *lprc, LPCVOID lpszString, UINT cbCount,
		CONST INT *lpDx, int flags);
static int  MwDrawText(HDC hdc, LPCVOID lpString, int nCount, LPRECT lpRect,
		UINT uFormat, int flags);
//BOOL ScreenToClient(HDC hdc, LPPOINT lpPoint);
static int MapWindowPoints(HDC hdc, BOOL bWndToDev, LPPOINT lpPoints, UINT cPoints);
static BOOL SetRect(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom);
static BOOL InflateRect(LPRECT lprc, int dx, int dy);

HDC
// by yuanlii@29bbs.net, 1/31/2005 17:07:01
// GetDCEx(HWND hwnd,HRGN hrgnClip,DWORD flags)
// by yuanlii@29bbs.net
GetDCEx(MWCOORD left, MWCOORD top, MWCOORD right, MWCOORD bottom, DWORD flags)
{
	HDC		hdc;

	initFontObj.pfont = g_pDefaultFont;
	/* add caching?*/
	hdc = GdItemNew(struct hdc);
	if(!hdc)
		return NULL;

	hdc->psd = &scrdev;
	hdc->wndrect.left = left;
	hdc->wndrect.top = top;
	hdc->wndrect.right = right;
	hdc->wndrect.bottom = bottom;

	hdc->flags = flags;
	hdc->bkmode = OPAQUE;
	hdc->textalign = TA_LEFT | TA_TOP | TA_NOUPDATECP;
	hdc->bkcolor = RGB(255, 255, 255);	/* WHITE*/
	hdc->textcolor = RGB(0, 0, 0);		/* BLACK*/
	hdc->brush = (MWBRUSHOBJ *)GetStockObject(WHITE_BRUSH);
	hdc->pen = (MWPENOBJ *)GetStockObject(BLACK_PEN);
// by yuanlii@29bbs.net, 1/31/2005 18:25:43
	hdc->font = (MWFONTOBJ *)&initFontObj;
// by yuanlii@29bbs.net
#if UPDATEREGIONS
	if(hrgnClip) {
		/* make a copy of passed region*/
		hdc->region = (MWRGNOBJ *)CreateRectRgn(0, 0, 0, 0);
		CombineRgn((HRGN)hdc->region, hrgnClip, NULL, RGN_COPY);
	}
#endif

	/* make default bitmap compatible with scrdev
	 * otherwise problems occur later because selecting
	 * in the default bitmap overwrite planes and bpp
	 * in a memory dc, and thus it becomes incompatible
	 * with scrdev.
	 */
	default_bitmap.planes = scrdev.planes;
	default_bitmap.bpp = scrdev.bpp;
	hdc->bitmap = &default_bitmap;

	hdc->drawmode = R2_COPYPEN;
	hdc->pt.x = 0;
	hdc->pt.y = 0;

	return hdc;
}

/* free a DC allocated from GetDCEx*/
int
ReleaseDC(HDC hdc)
{
	/* don't delete a memory dc on release*/
	if(!hdc || (hdc->psd->flags&PSF_MEMORY))
		return 0;

	DeleteObject((HBRUSH)hdc->brush);
	DeleteObject((HPEN)hdc->pen);
//	DeleteObject((HFONT)hdc->font);
//	DeleteObject((HRGN)hdc->region);
	/*
	 * We can only select a bitmap in a memory DC,
	 * so bitmaps aren't released except through DeleteDC.
	 */
	DeleteObject((HBITMAP)hdc->bitmap);
	GdItemFree(hdc);
	return 1;
}

/* free a dc allocated from CreateCompatibleDC*/
BOOL
DeleteDC(HDC hdc)
{
	/* don't delete a normal dc, only memory dc's*/
	if(!hdc || !(hdc->psd->flags&PSF_MEMORY))
		return 0;

	/* free allocated memory screen device*/
	hdc->psd->FreeMemGC(hdc->psd);

	/* make it look like a GetDC dc, and free it*/
	hdc->psd = &scrdev;
	return ReleaseDC(hdc);
}

HDC
BeginPaint(MWCOORD left, MWCOORD top, MWCOORD right, MWCOORD bottom)
{
	HDC	hdc;
	DWORD	flags;

	flags = DCX_DEFAULTCLIP|DCX_EXCLUDEUPDATE;
	hdc = GetDCEx(left, top, right, bottom, flags);	/* FIXME - bug*/
	return hdc;
}

BOOL
EndPaint(HDC hdc)
{
	ReleaseDC(hdc);
#if UPDATEREGIONS
	/* don't clear update region until done dragging*/
	if(mwERASEMOVE && !dragwp)
		GdSetRectRegion(hwnd->update, 0, 0, 0, 0);
#endif
	return TRUE;
}

COLORREF
SetTextColor(HDC hdc, COLORREF crColor)
{
	COLORREF	oldtextcolor;

	if (!hdc)
		return CLR_INVALID;
	oldtextcolor = hdc->textcolor;
	hdc->textcolor = (MWCOLORVAL)crColor;
	return oldtextcolor;
}

COLORREF
SetBkColor(HDC hdc, COLORREF crColor)
{
	COLORREF	oldbkcolor;

	if (!hdc)
		return CLR_INVALID;
	oldbkcolor = hdc->bkcolor;
	hdc->bkcolor = crColor;
	return oldbkcolor;
}

int
SetBkMode(HDC hdc, int iBkMode)
{
	int	oldbkmode;

	if(!hdc)
		return 0;
	oldbkmode = hdc->bkmode;
	hdc->bkmode = iBkMode;
	return oldbkmode;
}

UINT
SetTextAlign(HDC hdc, UINT fMode)
{
	UINT	oldfMode;

	if(!hdc)
		return GDI_ERROR;
	oldfMode = hdc->textalign;
	hdc->textalign = fMode;
	return oldfMode;
}

/* FIXME: releasing a DC does NOT change back the drawing mode!*/
int
SetROP2(HDC hdc, int fnDrawMode)
{
	int	newmode, oldmode;

	if(!hdc || (fnDrawMode <= 0 || fnDrawMode > R2_LAST))
		return 0;

	oldmode = hdc->drawmode;
	newmode = fnDrawMode - 1;	/* map to MWMODE_xxx*/
	hdc->drawmode = newmode;
	GdSetMode(newmode);
	return oldmode;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	MwPrepareDC here!
//	GetPixel&SetPixel here!
//	define ClientToScreen here which is define in winuser.c in MicroWindows
//
////////////////////////////////////////////////////////////////////////////////////

BOOL 
ClientToScreen(HDC hdc, LPPOINT lpPoint)
{
	if(!lpPoint)
		return FALSE;
	MapWindowPoints(hdc, TRUE, lpPoint, 1);
	return TRUE;
}

/*
static BOOL
ScreenToClient(HDC hdc, LPPOINT lpPoint)
{
	if(!lpPoint)
		return FALSE;
	MapWindowPoints(hdc, FALSE, lpPoint, 1);
	return TRUE;
}
*/

static int
MapWindowPoints(HDC hdc, BOOL bWndToDev, LPPOINT lpPoints, UINT cPoints)
{
	MWCOORD	offx = 0;
	MWCOORD	offy = 0;


	if(bWndToDev) {		/* map window to screen coords*/
		offx = hdc->wndrect.left;
		offy = hdc->wndrect.top;
	} else {		/* map screen to window coords*/
		offx = -1 * hdc->wndrect.left;
		offy = -1 * hdc->wndrect.top;
	}

	/* adjust points*/
	while(cPoints--) {
		lpPoints->x += offx;
		lpPoints->y += offy;
		++lpPoints;
	}
	return (int)MAKELONG(offx, offy);
}

static BOOL
SetRect(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom)
{
	lprc->left = xLeft;
	lprc->top = yTop;
	lprc->right = xRight;
	lprc->bottom = yBottom;
	return TRUE;
}

static BOOL
InflateRect(LPRECT lprc, int dx, int dy)
{
	lprc->left -= dx;
	lprc->top -= dy;
	lprc->right += dx;
	lprc->bottom += dy;
	return TRUE;
}

BOOL
MoveToEx(HDC hdc, int x, int y, LPPOINT lpPoint)
{
	if(!hdc)
		return FALSE;
	if(lpPoint)
		*lpPoint = hdc->pt;
	hdc->pt.x = x;
	hdc->pt.y = y;
	return TRUE;
}

BOOL
LineTo(HDC hdc, int x, int y)
{
	POINT		beg, end;

	beg.x = hdc->pt.x;
	beg.y = hdc->pt.y;
	end.x = x;
	end.y = y;
	
#ifdef	__DEBUG
	fprintf(stderr, "Draw line from (%d, %d) to (%d, %d)\n", beg.x, beg.y, end.x, end.y);
#endif
	if(MwIsClientDC(hdc)) {
		ClientToScreen(hdc, &beg);
		ClientToScreen(hdc, &end);
	}
#ifdef	__DEBUG
	fprintf(stderr, "Draw line from (%d, %d) to (%d, %d)\n", beg.x, beg.y, end.x, end.y);
#endif
	/* draw line in current pen color*/
	if(hdc->pen->style != PS_NULL) {
		GdSetForeground(GdFindColor(hdc->pen->color));
		/* don't draw last point*/
		GdLine(hdc->psd, beg.x, beg.y, end.x, end.y, FALSE);
	}
	hdc->pt.x = x;
	hdc->pt.y = y;
	return TRUE;
}

/* draw line segments by connecting passed points*/
BOOL
Polyline(HDC hdc, CONST POINT *lppt, int cPoints)
{
	POINT		beg, end;

	if(cPoints <= 1)
		return FALSE;

	if(hdc->pen->style == PS_NULL)
		return TRUE;

	/* draw line in current pen color*/
	GdSetForeground(GdFindColor(hdc->pen->color));

	beg = *lppt++;
	if(MwIsClientDC(hdc))
		ClientToScreen(hdc, &beg);
	while(--cPoints > 0) {
		end = *lppt++;
		if(MwIsClientDC(hdc))
			ClientToScreen(hdc, &end);

		/* don't draw last point*/
		GdLine(hdc->psd, beg.x, beg.y, end.x, end.y, FALSE);

		beg = end;
	}
	return TRUE;
}

BOOL
Rectangle(HDC hdc, int nLeft, int nTop, int nRight, int nBottom)
{
	RECT	rc;

	SetRect(&rc, nLeft, nTop, nRight, nBottom);
	if(MwIsClientDC(hdc))
		MapWindowPoints(hdc, TRUE, (LPPOINT)&rc, 2);

	/* draw rectangle in current pen color*/
	if(hdc->pen->style != PS_NULL) {
		GdSetForeground(GdFindColor(hdc->pen->color));
		GdRect(hdc->psd, rc.left, rc.top,
			rc.right - rc.left, rc.bottom - rc.top);
	}

	/* fill rectangle in current brush color*/
	if(hdc->brush->style != BS_NULL) {
		InflateRect(&rc, -1, -1);
		GdSetForeground(GdFindColor(hdc->brush->color));
		GdFillRect(hdc->psd, rc.left, rc.top, rc.right - rc.left,
			rc.bottom - rc.top);
	}

	return TRUE;
}

BOOL
Ellipse(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	int	rx, ry;
	RECT	rc;

	SetRect(&rc, nLeftRect, nTopRect, nRightRect, nBottomRect);
	if(MwIsClientDC(hdc))
		MapWindowPoints(hdc, TRUE, (LPPOINT)&rc, 2);

	rx = (rc.right - rc.left)/2 - 1;
	ry = (rc.bottom - rc.top)/2 - 1;
	rc.left += rx;
	rc.top += ry;

	/* fill ellipse in current brush color*/
	if(hdc->brush->style != BS_NULL) {
		InflateRect(&rc, -1, -1);
		GdSetForeground(GdFindColor(hdc->brush->color));
		GdEllipse(hdc->psd, rc.left, rc.top, rx, ry, TRUE);
	}

	/* draw ellipse outline in current pen color*/
	if(hdc->pen->style != PS_NULL) {
		GdSetForeground(GdFindColor(hdc->pen->color));
		GdEllipse(hdc->psd, rc.left, rc.top, rx, ry, FALSE);
	}

	return TRUE;
}

static void
dopiearc(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect,
	int ax, int ay, int bx, int by, int type)
{
	int	rx, ry;
	RECT	rc, rc2;

	SetRect(&rc, nLeftRect, nTopRect, nRightRect, nBottomRect);
	SetRect(&rc2, ax, ay, bx, by);
	if(MwIsClientDC(hdc)) {
		MapWindowPoints(hdc, TRUE, (LPPOINT)&rc, 2);
		MapWindowPoints(hdc, TRUE, (LPPOINT)&rc2, 2);
	}

	rx = (rc.right - rc.left)/2 - 1;
	ry = (rc.bottom - rc.top)/2 - 1;
	rc.left += rx;
	rc.top += ry;

// by yuanlii@29bbs.net, 2/ 1/2005 14:24:50
// make the misregistration of rc2 to rc
	rc2.left -= rc.left;
	rc2.top -= rc.top;
	rc2.right -= rc.left;
	rc2.bottom -= rc.top;
// by yuanlii@29bbs.net

	/* fill ellipse in current brush color*/
	if(hdc->brush->style != BS_NULL && type == MWPIE) {
		GdSetForeground(GdFindColor(hdc->brush->color));
		GdArc(hdc->psd, rc.left, rc.top, rx, ry,
			rc2.left, rc2.top, rc2.right, rc2.bottom, MWPIE);
	}

	/* draw ellipse outline in current pen color*/
	if(hdc->pen->style != PS_NULL) {
		GdSetForeground(GdFindColor(hdc->pen->color));
		if(type == MWPIE)
			type = MWARC;	/* MWARCOUTLINE?*/
		GdArc(hdc->psd, rc.left, rc.top, rx, ry,
			rc2.left, rc2.top, rc2.right, rc2.bottom, type);
	}
}

BOOL
Arc(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect,
	int nXStartArc, int nYStartArc, int nXEndArc, int nYEndArc)
{
	dopiearc(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect,
		nXStartArc, nYStartArc, nXEndArc, nYEndArc, MWARC);
	return TRUE;
}

BOOL
Pie(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect,
	int nXRadial1, int nYRadial1, int nXRadial2, int nYRadial2)
{
	dopiearc(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect,
		nXRadial1, nYRadial1, nXRadial2, nYRadial2, MWPIE);
	return TRUE;
}

BOOL
Polygon(HDC hdc, CONST POINT *lpPoints, int nCount)
{
	int	i;
	LPPOINT	pp, ppAlloc = NULL;

	if(MwIsClientDC(hdc)) {
		/* convert points to client coords*/
		ppAlloc = (LPPOINT)malloc(nCount * sizeof(POINT));
		if(!ppAlloc)
			return FALSE;
		memcpy(ppAlloc, lpPoints, nCount*sizeof(POINT));
		pp = ppAlloc;
		for(i=0; i<nCount; ++i)
			ClientToScreen(hdc, pp++);
		pp = ppAlloc;
	} else pp = (LPPOINT)lpPoints;

	/* fill polygon in current brush color*/
	if(hdc->brush->style != BS_NULL) {
		GdSetForeground(GdFindColor(hdc->brush->color));
		GdFillPoly(hdc->psd, nCount, pp);
	}

	/* draw polygon outline in current pen color*/
	if(hdc->pen->style != PS_NULL) {
		GdSetForeground(GdFindColor(hdc->pen->color));
		GdPoly(hdc->psd, nCount, pp);
	}

	if(ppAlloc)
		free(ppAlloc);
	return TRUE;
}

/* draw nCount polygons*/
BOOL
PolyPolygon(HDC hdc, CONST POINT *lpPoints, LPINT lpPolyCounts, int nCount)
{
	while(--nCount >= 0) {
		if (!Polygon(hdc, lpPoints, *lpPolyCounts))
			return FALSE;
		lpPoints += *lpPolyCounts++;
	}
	return TRUE;
}

int
FillRect(HDC hdc, CONST RECT *lprc, HBRUSH hbr)
{
	RECT 		rc;
	MWBRUSHOBJ *	obr = (MWBRUSHOBJ *)hbr;
	COLORREF	crFill;

	if(!obr)
		return FALSE;

	rc = *lprc;
	if(MwIsClientDC(hdc))
		MapWindowPoints(hdc, TRUE, (LPPOINT)&rc, 2);

	/* get color from passed HBRUSH*/
	if(obr->style == BS_NULL)
		return TRUE;
	crFill = obr->color;

	/* fill rectangle in passed brush color*/
	GdSetForeground(GdFindColor(crFill));
	GdFillRect(hdc->psd, rc.left, rc.top,
		rc.right - rc.left, rc.bottom - rc.top);
	return TRUE;
}

/* ascii*/
BOOL
TextOut(HDC hdc, int x, int y, LPCSTR lpszString, int cbString)
{
	/* kaffe port wants MWTF_UTF8 here...*/
	return MwExtTextOut(hdc, x, y, 0, NULL, lpszString, cbString, NULL,
			MWTF_ASCII);
}

/* ascii*/
BOOL
ExtTextOut(HDC hdc, int x, int y, UINT fuOptions, CONST RECT *lprc,
	LPCSTR lpszString, UINT cbCount, CONST INT *lpDx)
{
	return MwExtTextOut(hdc, x, y, fuOptions, lprc, lpszString,
		cbCount, lpDx, MWTF_ASCII);
}

/* unicode*/
// by yuanlii@29bbs.net, 2/ 3/2005 14:07:35
/*BOOL
ExtTextOutW(HDC hdc, int x, int y, UINT fuOptions, CONST RECT *lprc,
	LPCWSTR lpszString, UINT cbCount, CONST INT *lpDx)
{
	return MwExtTextOut(hdc, x, y, fuOptions, lprc, lpszString,
		cbCount, lpDx, MWTF_UC16);
}
*/
// by yuanlii@29bbs.net

/* internal version of ExtTextOut, passed flags for text data type*/
static BOOL
MwExtTextOut(HDC hdc, int x, int y, UINT fuOptions, CONST RECT *lprc,
	LPCVOID lpszString, UINT cbCount, CONST INT *lpDx, int flags)
{
	POINT	pt;
	RECT	rc;

	pt.x = x;
	pt.y = y;
	if(MwIsClientDC(hdc))
		ClientToScreen(hdc, &pt);

	/* optionally fill passed rectangle*/
	if(lprc && (fuOptions&ETO_OPAQUE)) {
		rc = *lprc;
		if(MwIsClientDC(hdc))
			MapWindowPoints(hdc, TRUE, (LPPOINT)&rc, 2);

		/* fill rectangle with current background color*/
		GdSetForeground(GdFindColor(hdc->bkcolor));
		GdFillRect(hdc->psd, rc.left, rc.top, rc.right - rc.left,
				rc.bottom - rc.top);
		GdSetUseBackground(FALSE);
	} else {
		/* use current background mode for text background draw*/
		GdSetUseBackground(hdc->bkmode == OPAQUE? TRUE: FALSE);
		/* always set background color in case GdArea is
		 * used to draw, which compares gr_foreground != gr_background
		 * if gr_usebg is false...
		 */
		/*if(hdc->bkmode == OPAQUE)*/
		GdSetBackground(GdFindColor(hdc->bkcolor));
	}

	/* nyi: lpDx*/

	/* draw text in current text foreground and background color*/
	GdSetForeground(GdFindColor(hdc->textcolor));

	/* this whole text alignment thing needs rewriting*/
	if((hdc->textalign & TA_BASELINE) == TA_BASELINE) {
		/* this is not right... changed for kaffe port
		   flags |= MWTF_TOP;
		   */
		flags |= MWTF_BASELINE;
	} else if(hdc->textalign & TA_BOTTOM) {
		MWCOORD	ph, pw, pb;

		if(lprc)
			pt.y += lprc->bottom - lprc->top;
		else {
			GdGetTextSize(hdc->font->pfont, lpszString, cbCount,
					&pw, &ph, &pb, flags);
			pt.y += ph;
		}
		flags |= MWTF_BOTTOM;
	} else
		flags |= MWTF_TOP;
	GdText(hdc->psd, pt.x, pt.y, lpszString, cbCount, flags);

	return TRUE;
}

/* ascii*/
int
DrawTextA(HDC hdc, LPCSTR lpString, int nCount, LPRECT lpRect, UINT uFormat)
{
	return MwDrawText(hdc, lpString, nCount, lpRect, uFormat, MWTF_ASCII);
}

/* unicode*/
// by yuanlii@29bbs.net, 2/ 3/2005 14:08:28
/*int
DrawTextW(HDC hdc, LPCWSTR lpString, int nCount, LPRECT lpRect, UINT uFormat)
{
	return MwDrawText(hdc, lpString, nCount, lpRect, uFormat, MWTF_UC16);
}
*/
// by yuanlii@29bbs.net

/* note: many DT_x aren't implemented in this function*/
/* internal version of DrawText, passed flags for text data type*/
static int
MwDrawText(HDC hdc, LPCVOID lpString, int nCount, LPRECT lpRect, UINT uFormat,
	int flags)
{
	MWCOORD	x, y, width, height, baseline;

	if(nCount == -1)
		nCount = strlen(lpString);

	if(uFormat & (DT_CALCRECT|DT_CENTER|DT_RIGHT)) {
		if(!hdc)
			return 0;
		GdGetTextSize(hdc->font->pfont, lpString, nCount,
			&width, &height, &baseline, MWTF_ASCII);
	}
	x = lpRect->left;
	y = lpRect->top;

	if(uFormat & DT_CALCRECT) {
		lpRect->right = x + width;
		lpRect->bottom = y + height;
		return height;
	}

	if(uFormat & DT_CENTER)
		x = (lpRect->left + lpRect->right - width) / 2;
	else if(uFormat & DT_RIGHT)
		x += lpRect->right - width;

	/* draw text at DT_TOP using current fg, bg and bkmode*/
	MwExtTextOut(hdc, x, y, 0, NULL, lpString, nCount, NULL, flags);
	return height;
}

static MWBRUSHOBJ OBJ_WHITE_BRUSH = {
	{OBJ_BRUSH, TRUE}, BS_SOLID, RGB(255, 255, 255)
};

static MWBRUSHOBJ OBJ_LTGRAY_BRUSH = {
	{OBJ_BRUSH, TRUE}, BS_SOLID, RGB(192, 192, 192)
};

static MWBRUSHOBJ OBJ_GRAY_BRUSH = {
	{OBJ_BRUSH, TRUE}, BS_SOLID, RGB(128, 128, 128)
};

static MWBRUSHOBJ OBJ_DKGRAY_BRUSH = {
	{OBJ_BRUSH, TRUE}, BS_SOLID, RGB(32, 32, 32)
};

static MWBRUSHOBJ OBJ_BLACK_BRUSH = {
	{OBJ_BRUSH, TRUE}, BS_SOLID, RGB(0, 0, 0)
};

static MWBRUSHOBJ OBJ_NULL_BRUSH = {
	{OBJ_BRUSH, TRUE}, BS_NULL, RGB(0, 0, 0)
};

static MWPENOBJ OBJ_WHITE_PEN = {
	{OBJ_PEN, TRUE}, PS_SOLID, RGB(255, 255, 255)
};

static MWPENOBJ OBJ_BLACK_PEN = {
	{OBJ_PEN, TRUE}, PS_SOLID, RGB(0, 0, 0)
};

static MWPENOBJ OBJ_NULL_PEN = {
	{OBJ_PEN, TRUE}, PS_NULL, RGB(0, 0, 0)
};

static struct hgdiobj *stockObjects[MAXSTOCKOBJECTS] = {
	(struct hgdiobj *)&OBJ_WHITE_BRUSH,		/* WHITE_BRUSH*/
	(struct hgdiobj *)&OBJ_LTGRAY_BRUSH,		/* LTGRAY_BRUSH*/
	(struct hgdiobj *)&OBJ_GRAY_BRUSH,		/* GRAY_BRUSH*/
	(struct hgdiobj *)&OBJ_DKGRAY_BRUSH,		/* DKGRAY_BRUSH*/
	(struct hgdiobj *)&OBJ_BLACK_BRUSH,		/* BLACK_BRUSH*/
	(struct hgdiobj *)&OBJ_NULL_BRUSH,		/* NULL_BRUSH*/
	(struct hgdiobj *)&OBJ_WHITE_PEN,		/* WHITE_PEN*/
	(struct hgdiobj *)&OBJ_BLACK_PEN,		/* BLACK_PEN*/
	(struct hgdiobj *)&OBJ_NULL_PEN,		/* NULL_PEN*/
	(struct hgdiobj *)NULL,				/* DEFAULT_PALETTE*/
};

HGDIOBJ
GetStockObject(int nObject)
{
	HGDIOBJ		pObj;

	if(nObject >= 0 && nObject < MAXSTOCKOBJECTS) {
		pObj = stockObjects[nObject];

		return pObj;
	}
	return NULL;
}

HGDIOBJ
SelectObject(HDC hdc, HGDIOBJ hObject)
{
	HGDIOBJ		objOrg;
	MWBITMAPOBJ *	pb;

	if( !hdc || !hObject )
		return NULL;
	switch(hObject->hdr.type) {
	case OBJ_PEN:
		objOrg = (HGDIOBJ)hdc->pen;
		hdc->pen = (MWPENOBJ *)hObject;
		break;
	case OBJ_BRUSH:
		objOrg = (HGDIOBJ)hdc->brush;
		hdc->brush = (MWBRUSHOBJ *)hObject;
		break;
	case OBJ_FONT:
		objOrg = (HGDIOBJ)hdc->font;
		hdc->font = (MWFONTOBJ *)hObject;
		// by yuanlii@29bbs.net, 2/ 4/2005 15:40:15
		GdSetFont(hdc->font->pfont);
		// by yuanlii@29bbs.net
		break;
	case OBJ_BITMAP:
		/* must be memory dc to select bitmap*/
		if(!(hdc->psd->flags&PSF_MEMORY))
			return NULL;
		objOrg = (HGDIOBJ)hdc->bitmap;

		/* setup mem dc for drawing into bitmap*/
		pb = (MWBITMAPOBJ *)hObject;

		/* init memory context*/
		if (!hdc->psd->MapMemGC(hdc->psd, pb->width, pb->height,
			pb->planes, pb->bpp, pb->linelen, pb->size,
			&pb->bits[0]))
				return NULL;

		hdc->bitmap = (MWBITMAPOBJ *)hObject;
	    	break;
#if UPDATEREGIONS
	case OBJ_REGION:
		/*objOrg = (HGDIOBJ)hdc->region;*/
		objOrg = NULL;	/* FIXME? hdc->region is destroyed below*/
		SelectClipRgn(hdc, (HRGN)hObject);
		break;
#endif
	default:
		return NULL;
	}

	return objOrg;
}

BOOL
DeleteObject(HGDIOBJ hObject)
{
	if(!hObject || hObject->hdr.stockobj)
		return FALSE;
	if(hObject->hdr.type == OBJ_FONT)
	{
		if( ((MWFONTOBJ *)hObject)->pfont != g_pDefaultFont && ((MWFONTOBJ *)hObject)->pfont )
			GdDestroyFont(((MWFONTOBJ *)hObject)->pfont);
		((MWFONTOBJ *)hObject)->pfont = NULL;		// 2005.3.16 CYJ Add
	}
	if(hObject->hdr.type == OBJ_REGION)
	{
		if( ((MWRGNOBJ *)hObject)->rgn )
			GdDestroyRegion(((MWRGNOBJ *)hObject)->rgn);
		((MWRGNOBJ *)hObject)->rgn = NULL;			// 2005.3.16 CYJ Add
	}
	GdItemFree(hObject);
	return TRUE;
}

HBRUSH
CreateSolidBrush(COLORREF crColor)
{
	MWBRUSHOBJ *hbr;

	hbr = GdItemNew(MWBRUSHOBJ);
	if(!hbr)
		return NULL;
	hbr->hdr.type = OBJ_BRUSH;
	hbr->hdr.stockobj = FALSE;
	hbr->style = BS_SOLID;
	hbr->color = crColor;
	return (HBRUSH)hbr;
}

HPEN
CreatePen(int nPenStyle, int nWidth, COLORREF crColor)
{
	MWPENOBJ *hpen;

	/* fix: nWidth > 1*/
	hpen = GdItemNew(MWPENOBJ);
	if(!hpen)
		return NULL;
	hpen->hdr.type = OBJ_PEN;
	hpen->hdr.stockobj = FALSE;
	hpen->style = nPenStyle;
	hpen->color = crColor;
	return (HPEN)hpen;
}

//////////////////////////////////////////////////////////////////
//
//	The difficulty of this file
//
//////////////////////////////////////////////////////////////////

HBITMAP
CreateCompatibleBitmap(HDC hdc, int nWidth, int nHeight)
{
	MWBITMAPOBJ *	hbitmap;
	int		size;
	int		linelen;

	if(!hdc)
		return NULL;

	nWidth = max(nWidth, 1);
	nHeight = max(nHeight, 1);

	/* calc memory allocation size and linelen from width and height*/
	if(!GdCalcMemGCAlloc(hdc->psd, nWidth, nHeight, 0, 0, &size, &linelen))
		return NULL;

	/* allocate gdi object*/
	hbitmap = (MWBITMAPOBJ *)GdItemAlloc(sizeof(MWBITMAPOBJ)-1+size);
	if(!hbitmap)
		return NULL;
	hbitmap->hdr.type = OBJ_BITMAP;
	hbitmap->hdr.stockobj = FALSE;
	hbitmap->width = nWidth;
	hbitmap->height = nHeight;

	/* create compatible with hdc*/
	hbitmap->planes = hdc->psd->planes;
	hbitmap->bpp = hdc->psd->bpp;
	hbitmap->linelen = linelen;
	hbitmap->size = size;

	return (HBRUSH)hbitmap;
}

/* return NULL if no driver bitblit available*/
HDC
CreateCompatibleDC(HDC hdc)
{
	HDC	hdcmem;
	PSD	psd;
	PSD	mempsd;

	/* allow NULL hdc to mean screen*/
	psd = hdc? hdc->psd: &scrdev;

	/* allocate memory device, if driver doesn't blit will fail*/
	mempsd = psd->AllocateMemGC(psd);
	if(!mempsd)
		return NULL;

	/* allocate a DC for DesktopWindow*/
	hdcmem = GetDCEx(0, 0, scrdev.xvirtres, scrdev.yvirtres, DCX_DEFAULTCLIP);
	if(!hdcmem) {
		mempsd->FreeMemGC(mempsd);
		return NULL;
	}
	hdcmem->psd = mempsd;

	/* select in default bitmap to setup mem device parms*/
	SelectObject(hdcmem, (HGDIOBJ)&default_bitmap);
	return hdcmem;
}

BOOL
BitBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight,
	HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop)
{
	/* use stretch blit with equal src and dest width/height*/
	return StretchBlt(hdcDest, nXDest, nYDest, nWidth, nHeight,
		hdcSrc, nXSrc, nYSrc, nWidth, nHeight, dwRop);
}

BOOL
StretchBlt(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest,
	int nHeightDest, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc,
	int nWidthSrc, int nHeightSrc, DWORD dwRop)
{

	POINT	dst, src;

	if(!hdcDest || !hdcSrc)
		return FALSE;
	dst.x = nXOriginDest;
	dst.y = nYOriginDest;
	src.x = nXOriginSrc;
	src.y = nYOriginSrc;

	/* if src screen DC, convert coords*/
	/* FIXME: src clipping isn't checked, only one set of cliprects also*/
	if(!MwIsMemDC(hdcSrc) && MwIsClientDC(hdcSrc)) {
		ClientToScreen(hdcSrc, &src);
	}
	/* if dst screen DC, convert coords and set clipping*/
	/* FIXME: if dest is also screen, src clipping will be overwritten*/
	if(!MwIsMemDC(hdcDest) && MwIsClientDC(hdcDest)) {
		ClientToScreen(hdcDest, &dst);
	}

	if (nWidthDest == nWidthSrc && nHeightDest == nHeightSrc) {
		GdBlit(hdcDest->psd, dst.x, dst.y, nWidthDest, nHeightDest,
			hdcSrc->psd, src.x, src.y, dwRop);
	} else {
		GdStretchBlit(hdcDest->psd, dst.x, dst.y,
			nWidthDest, nHeightDest, hdcSrc->psd, src.x, src.y,
			nWidthSrc, nHeightSrc, dwRop);
	}
	return TRUE;
}

/*
 * Initialize the graphics and mouse devices at startup.
 * Returns nonzero with a message printed if the initialization failed.
 */
int
MwInitialize(void)/*{{{*/
{
	PSD		psd;

	if ((psd = GdOpenScreen()) == NULL) {
		EPRINTF("Cannot initialise screen\n");
		return -1;
	}

	return 0;
}/*}}}*/

/*
 * Here to close down the server.
 */
void
MwTerminate(void)/*{{{*/
{
	GdCloseScreen(&scrdev);
	
}/*}}}*/

/* by yuanlii@29bbs.net, 07/29/2005 16:30:57 */
/***
 * @ingroup MyCDC
 * 	Reset the screen display
 * @no param
 * @return int 0  success
 * 	       -1 failure
 */	
int
MwReset(void)
{
	PSD	psd;

	if ((psd = GdResetScreen(&scrdev)) == NULL) {
#ifdef	_DEBUG
		EPRINTF("Cannot reset screen\n");
		return -1;
#endif/*_DEBUG*/
	}

	return 0;
}
/* by yuanlii@29bbs.net */

/* allow NULL hdc for scrdev*/
int GetDeviceCaps(HDC hdc, int nIndex)
{
	PSD	psd;

	if (!hdc)
		psd = &scrdev;
	else psd = hdc->psd;

	switch(nIndex) {
	case HORZRES:
		return psd->xvirtres;
	case VERTRES:
		return psd->yvirtres;
	case BITSPIXEL:
		return psd->bpp;
	case PLANES:
		return psd->planes;
	case LOGPIXELSX:
	case LOGPIXELSY:
		return 96;
	case SIZEPALETTE:
		if (psd->bpp <= 8)
			return psd->ncolors;
		break;
	}
	return 0;
}
