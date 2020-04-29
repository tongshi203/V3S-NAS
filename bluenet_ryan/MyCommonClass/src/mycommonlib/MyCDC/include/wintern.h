/* wintern.h*/
/*
 * Copyright (c) 1999 Greg Haerr <greg@censoft.com>
 *
 * Microwindows internal routines header file
 */
#include <string.h>
#include <windef.h>
#include <mwtypes.h>

#if (UNIX | DOS_DJGPP)
#define strcmpi	strcasecmp
#elif (VXWORKS)
int strcmpi(const char *s1, const char *s2);
#endif

#ifdef __PACIFIC__
#define strcmpi		stricmp
#endif  

#define DBLCLICKSPEED	750		/* mouse dblclik speed msecs (was 450)*/

/* gotPaintMsg values*/
#define PAINT_PAINTED		0	/* WM_PAINT msg has been processed*/
#define PAINT_NEEDSPAINT	1	/* WM_PAINT seen, paint when can*/
#define PAINT_DELAYPAINT	2	/* WM_PAINT seen,paint after user move*/

/* wingdi.c*/
#define MwIsClientDC(hdc)	(((hdc)->flags & DCX_WINDOW) == 0)
#define MwIsMemDC(hdc)		((hdc)->psd->flags == PSF_MEMORY)

/* winmain.c*/
int		MwOpen(void);
void		MwClose(void);
void		MwSelect(void);
int		MwInitialize(void);
void		MwTerminate(void);
/* by yuanlii@29bbs.net, 07/29/2005 16:35:50 */
int		MwReset(void);
/* by yuanlii@29bbs.net */
extern	HWND	listwp;			/* list of all windows */
extern	HWND	rootwp;			/* root window pointer */
extern	HWND	focuswp;		/* focus window for keyboard */
extern	HWND	mousewp;		/* window mouse is currently in */
extern	HWND	capturewp;		/* capture window*/
extern  HWND	dragwp;			/* window user is dragging*/
extern	HCURSOR	curcursor;		/* currently enabled cursor */
extern	MWCOORD	cursorx;		/* x position of cursor */
extern	MWCOORD	cursory;		/* y position of cursor */
extern	MWSCREENINFO	sinfo;		/* screen information */
extern  DWORD	startTicks;		/* tickcount on startup */
extern  int	mwpaintNC;		/* experimental nonclient regions*/
extern  BOOL	mwforceNCpaint;		/* force NC paint for alphablend*/

#if VTSWITCH
/* temp framebuffer vt switch stuff at upper level
 * this should be handled at the lower level, just like vgalib does.
 */
void MwInitVt(void);
int  MwCurrentVt(void);
int  MwCheckVtChange(void);
void MwRedrawVt(int t);
void MwExitVt(void);
extern int mwvterm;
#endif /* VTSWITCH*/
