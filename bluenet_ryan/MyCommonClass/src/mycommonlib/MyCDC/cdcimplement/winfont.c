/*
 * Copyright (c) 2000 Greg Haerr <greg@censoft.com>
 * GetTextExtent*Point by Roman Guseynov
 * Original contributions by Shane Nay
 *
 * Win32 API upper level font selection routines
 */
#include "windows.h"
#include "wintern.h"
#include "device.h"
#include <stdlib.h>
#include <string.h>

BOOL WINAPI
GetTextExtentPoint(
	HDC hdc,		/* handle to DC*/
	LPCTSTR lpszStr,	/* character string*/
	int cchString,		/* number of characters*/
	LPSIZE lpSize)		/* string dimensions*/
{
	int width = 1, height = 1, baseline = 0;

	if (lpSize) {
		lpSize->cx = 0;
		lpSize->cy = 0;
	}
	if (!hdc || !lpszStr || !cchString || !lpSize)
		return FALSE;
// 2005.3.16 CYJ Modify, not support UTF8		
#if 0		
	GdGetTextSize(hdc->font->pfont, lpszStr, cchString, &width, &height,
		&baseline, MWTF_UTF8);
#else
	GdGetTextSize(hdc->font->pfont, lpszStr, cchString, &width, &height,
		&baseline, MWTF_ASCII);
#endif //		
	lpSize->cx = width;
	lpSize->cy = height;

	/*printf("<MWIN>: lpszStr=\"%s\", cchString=%d, lpsize->cx=%d, lpSize->cy=%d\n", lpszStr, cchString, lpSize->cx, lpSize->cy);*/
	return TRUE;
}

BOOL WINAPI
GetTextExtentExPoint(HDC hdc,	/* handle to DC*/
	  LPCTSTR lpszStr,	/* character string*/
	  int cchString,	/* number of characters*/
	  int nMaxExtent,	/* maximum width of formatted string*/
	  LPINT lpnFit,		/* maximum number of characters*/
	  LPINT alpDx,	 	/* array of partial string widths*/
	  LPSIZE lpSize)	/* string dimensions*/

{
	int attr,width=0,height=0;

	if(!hdc || !lpszStr)
		return FALSE;
	if (cchString<0)
		cchString = strlen((char *)lpszStr);
	attr=hdc->font->pfont->fontattr;
	if (attr&FS_FREETYPE)
	{ 
// 2005.3.16 CYJ Modify, not support UTF8	
#if 0	
		if (GdGetTextSizeEx(hdc->font->pfont,lpszStr,cchString,
			nMaxExtent,lpnFit,alpDx,&width,&height,NULL,MWTF_UTF8))
#else
		if (GdGetTextSizeEx(hdc->font->pfont,lpszStr,cchString,
			nMaxExtent,lpnFit,alpDx,&width,&height,NULL,MWTF_ASCII))
#endif //			
		{
			lpSize->cx=width;
			lpSize->cy=height;
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		SIZE sz;
		int i;

		if (!GetTextExtentPoint(hdc, lpszStr, cchString, lpSize))
			return FALSE;
		if ((!nMaxExtent)||(!lpnFit))	// cyj modify, remove condition '||(!alpDx)'
			return TRUE;
		for (i=0; i<cchString; i++) 
		{
			if( (unsigned char)lpszStr[i] >= 0xA0 && (unsigned char)lpszStr[i+1] >= 0xA0 )
				i ++;		// cyj modify, this is a Chinese string
			if (!GetTextExtentPoint(hdc, lpszStr, i+1, &sz))
				return FALSE;
			if (sz.cx <= nMaxExtent)
			{
				if( alpDx )			// cyj add, when alpDx != NULL, then record the cx
					alpDx[i] = sz.cx;
			}
			else {
				(*lpnFit) = i+1;
				return TRUE;
			}
		}
		(*lpnFit) = cchString;
		return TRUE;	
	}
}     
