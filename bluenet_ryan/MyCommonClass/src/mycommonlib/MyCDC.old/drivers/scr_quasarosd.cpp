/*
 * Copyright (c) 1999, 2000, 2001 Greg Haerr <greg@censoft.com>
 *
 * Microwindows Screen Driver for Linux kernel framebuffers
 *
 * Portions used from Ben Pfaff's BOGL <pfaffben@debian.org>
 * 
 * Note: modify select_fb_driver() to add new framebuffer subdrivers
 */
#define _GNU_SOURCE 1
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef ARCH_LINUX_POWERPPC
#ifdef __GLIBC__
#include <sys/io.h>
#else
#include <asm/io.h>
#endif
#endif
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "device.h"
#include "genfont.h"
#include "genmem.h"

#include "caribbean_plainc.h"
#include "realmagichwl.h"		// /root/work/em8510/kernelmodule/realmagichwl_kernelland/
#include "realmagichwl_userland_api.h"

// hardware library uses these types... undefine them later on. ********************************
#define BYTE RMuint8
#define LONG RMint32
#define ULONG RMuint32
#define ULONGLONG RMuint64
#define BOOL RMbool
#define BOOLEAN RMbool
#define HANDLE void *
#define UCHAR RMuint8
#define USHORT RMuint16
#define PULONG RMuint32 *
#define PVOID void *

#include "../realmagichwl_kernelland/include/rm84cmn.h"

#define TRACE

//XXX
//#define DRAWON   while(0)
//#define DRAWOFF  while(0)
typedef unsigned char *		ADDR8;
typedef unsigned short *	ADDR16;
typedef unsigned long *		ADDR32;

static PSD  qosd_open(PSD psd);
static void qosd_close(PSD psd);
static void qosd_setpalette(PSD psd,int first, int count, MWPALENTRY *palette);
static void gen_getscreeninfo(PSD psd,PMWSCREENINFO psi);
// by yuanlii@29bbs.net, 1/31/2005 16:46:47
// unused function, comment it now
// static void qosd_unimplemented(void);
// by yuanlii@29bbs.net
static void qosd_drawpixel(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c);
static MWPIXELVAL qosd_readpixel(PSD psd, MWCOORD x, MWCOORD y);
static void qosd_drawhorzline(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y,
			      MWPIXELVAL c);
static void qosd_drawvertline(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2,
			      MWPIXELVAL c);
static void qosd_blit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD w,
		      MWCOORD h, PSD srcpsd, MWCOORD srcx, MWCOORD srcy,
		      long op);
static void qosd_stretchblit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty,
			     MWCOORD dstw, MWCOORD dsth, PSD srcpsd,
			     MWCOORD srcx, MWCOORD srcy, MWCOORD srcw,
			     MWCOORD srch, long op);
// by yuanlii@29bbs.net, 12/19/2004 12:07:08
int qosd_init(PSD psd);
static MWBOOL qosd_mapmemgc(PSD mempsd,MWCOORD w,MWCOORD h,int planes,int bpp,
		int linelen,int size,void *addr);
#if	SPEEDUP_DEVICE
/***
 * @ingroup MyCDC
 * 	draw 16x16 & 24x24 point fonts for my em8511
 */

static void	qosd_drawblock(PSD psd,MWCOORD x,MWCOORD y,MWPIXELVAL *c,
				MWCOORD width, MWCOORD height);
static void	qosd_readblock(PSD psd,MWCOORD x,MWCOORD y,MWPIXELVAL *c,
				MWCOORD width, MWCOORD height);
static void	qosd_drawline(PSD psd, MWCOORD x, MWCOORD y,
				MWPIXELVAL *c, int nCount);
#endif/*SPEEDUP_DEVICE*/
// by yuanlii@29bbs.net

/* genmem.c*/
void gen_fillrect(PSD psd,MWCOORD x1,MWCOORD y1,MWCOORD x2,MWCOORD y2,
		  MWPIXELVAL c);

SCREENDEVICE	scrdev = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL,
	qosd_open,
	qosd_close,
	gen_getscreeninfo,
	qosd_setpalette,
	qosd_drawpixel,		/* DrawPixel subdriver*/
	qosd_readpixel,		/* ReadPixel subdriver*/
	qosd_drawhorzline,	/* DrawHorzLine subdriver*/
	qosd_drawvertline,	/* DrawVertLine subdriver*/
	gen_fillrect,		/* FillRect subdriver*/
// by yuanlii@29bbs.net, 1/24/2005 17:46:35
//	gen_fonts,
	NULL,
// by yuanlii@29bbs.net
	qosd_blit,		/* Blit subdriver*/
	NULL,	                /* PreSelect*/
	NULL,	                /* DrawArea subdriver*/
	NULL,	                /* SetIOPermissions*/
	gen_allocatememgc,
//	NULL,  // qosd_mapmemgc,
	qosd_mapmemgc,  //NULL
	gen_freememgc,
//	NULL,	                /* StretchBlit subdriver*/
	qosd_stretchblit,	/* StretchBlit subdriver*/
	NULL	                /* SetPortrait*/
#if	SPEEDUP_DEVICE
	,			/* the comma of last function */
	qosd_drawblock,		/* block drawing speedup */
	qosd_readblock,		/* block reading speedup */
	qosd_drawline		/* draw a line in a bitmap speedup */
#endif/*SPEEDUP_DEVICE*/
};

/* ROP macro for 16 drawing modes*/
#define CHECK(f,d) 

/* applyOp w/stored dst*/
#define	applyOp(op, src, pdst, type)		\
{						\
	type d = (pdst);			\
	switch (op) {				\
	case MWMODE_XOR:			\
		*d ^= (src);			\
		CHECK("XOR", *d);		\
		break;				\
	case MWMODE_AND:			\
		*d &= (src);			\
		CHECK("AND", *d);		\
		break;				\
	case MWMODE_OR:				\
		*d |= (src);			\
		CHECK("OR", *d);		\
		break;				\
	case MWMODE_CLEAR:			\
		*d = 0;				\
		CHECK("CLEAR", *d);		\
		break;				\
	case MWMODE_SETTO1:			\
		*d = -1;			\
		CHECK("SETTO1", *d);		\
		break;				\
	case MWMODE_EQUIV:			\
		*d = ~(*d ^ (src));		\
		CHECK("EQUIV", *d);		\
		break;				\
	case MWMODE_NOR:			\
		*d = ~(*d | (src));		\
		CHECK("NOR", *d);		\
		break;				\
	case MWMODE_NAND:			\
		*d = ~(*d & (src));		\
		CHECK("NAND", *d);		\
		break;				\
	case MWMODE_INVERT:			\
		*d = ~*d;			\
		CHECK("INVERT", *d);		\
		break;				\
	case MWMODE_COPYINVERTED:		\
		*d = ~(src);			\
		CHECK("COPYINVERTED", *d);	\
		break;				\
	case MWMODE_ORINVERTED:			\
		*d |= ~(src);			\
		CHECK("ORINVERTED", *d);	\
		break;				\
	case MWMODE_ANDINVERTED:		\
		*d &= ~(src);			\
		CHECK("ANDINVERTED", *d);	\
		break;				\
	case MWMODE_ORREVERSE:			\
		*d = ~*d | (src);		\
		CHECK("ORREVERSE", *d);		\
		break;				\
	case MWMODE_ANDREVERSE:			\
		*d = ~*d & (src);		\
		CHECK("ANDREVERSE", *d);	\
		break;				\
	case MWMODE_COPY:			\
		*d = (src);			\
		CHECK("COPY", *d);		\
		break;				\
	case MWMODE_NOOP:			\
		CHECK("NOOP", *d);		\
		break;				\
	}					\
}

#if 0
#define assert(str) 									\
	if (!(str)) {									\
		printf("assertion failed in  file  %s, function  %s(), line  %d\n", 	\
				__FILE__, __FUNCTION__, __LINE__);			\
		abort();								\
	}
#else
	#define assert(str) 									\
	if (!(str)) {									\
		printf ("ASSERT failed: line %d, file %s\n", __LINE__,__FILE__);\
		abort();\
	}
#endif //	

extern int gr_mode;

/* static variables*/
static int status;		/* 0=never inited, 1=once inited, 2=inited. */
static char *osd_buffer_addr;

// by yuanlii@29bbs.net, 1/31/2005 16:47:15
/*static void qosd_unimplemented(void)
{
	printf("*** CALL TO UNIMPLEMENTED FONCTION ***\n");
}
*/
// by yuanlii@29bbs.net

#if 0	/* use RUA_OpenDevice */
static RUA_handle h;
/* init framebuffer*/
static PSD
qosd_open(PSD psd)
{
	unsigned long flicker;
	OSDBuffer osdbuffer;
	Wnd_type Wnd;

	assert(status < 2)

	psd->portrait = MWPORTRAIT_NONE;

	psd->planes = 1;

	psd->flags = PSF_SCREEN | PSF_HAVEBLIT;

	/* set pixel format*/
	psd->pixtype = MWPF_PALETTE;

	/*DPRINTF("%dx%dx%d linelen %d type %d visual %d bpp %d\n", psd->xres,
	 	psd->yres, psd->ncolors, psd->linelen, type, visual,
		psd->bpp);*/

// XXX If we need to we can mmap the osdbuf from the device
	h = RUA_OpenDevice(0);
	if(RUA_OSDFB_SWITCH(h,&osdbuffer) != 0) {
		EPRINTF("Error getting the osd buffer\n");
		goto fail;
	} else {
		osd_buffer_addr = osdbuffer.framebuffer;
		psd->bpp = osdbuffer.bpp;
		psd->xres = psd->xvirtres = osdbuffer.width;
		psd->yres = psd->yvirtres = osdbuffer.height;
		/* set linelen to byte length, possibly converted later*/
		psd->linelen = osdbuffer.width;
		/* force subdriver init of size*/
		psd->size = osdbuffer.width * osdbuffer.height + 1024 + 8;
		psd->ncolors = (psd->bpp >= 24)? (1 << 24): (1 << psd->bpp);
		printf("OSD Buffer of size %d allocated @ %p\n", psd->size, osd_buffer_addr);
	}
	psd->addr = osd_buffer_addr + 8 + 1024;
	
	/* save original palette ??? */
	

	/* Setup the flicker filter */
	// XXX - its also set in the kernel module, but it seems not to work very well
	//  We reset it here, so that the microcode as already seen an osd frame.	
	// 0 <= flicker <= 15
	flicker = 15;
	RUA_DECODER_SET_PROPERTY (h, DECODER_SET, edecOsdFlicker, sizeof(flicker), &flicker);
	
	// default osd destination
	Wnd.x = 0;
	Wnd.y = 0;
	Wnd.w = osdbuffer.width;
	Wnd.h = osdbuffer.height;
	RUA_DECODER_SET_PROPERTY (h, OSD_SET, eOsdDestinationWindow, sizeof(Wnd), &Wnd);
	
	DPRINTF("End of qosd_open\n");

	status = 2;
	return psd;	/* success*/

fail:
	return NULL;
}

/* close framebuffer*/
static void
qosd_close(PSD psd)
{
	/* if not opened, return*/
	if(status != 2)
		return;
	status = 1;

	RUA_ReleaseDevice(h);
	/* unmap framebuffer*/
//	free(osd_buffer_addr);
}
#else	/* use CreateDecoder */
#include <mympegdec.h>
IMyMpegDecoder * gr_pDecoder = NULL;
static bool	 s_bIsCreateHere = false;		// 若为true表明gr_pDecoder是在本函数中创建的，退出时需要删除
static PSD
qosd_open(PSD psd)
{
	OSDBuffer osdbuffer;
	MPEG_DECODER_ERROR nRetVal;

	assert(status < 2)

	psd->portrait = MWPORTRAIT_NONE;

	psd->planes = 1;

	psd->flags = PSF_SCREEN | PSF_HAVEBLIT;

	/* set pixel format*/
	psd->pixtype = MWPF_PALETTE;

	if( NULL == gr_pDecoder )
	{
		// 若不存在，则自动创建
		gr_pDecoder = CreateDecoder( CDMPEG_FLAG_AUTO_DELETE_ON_EXIT );	
		if( NULL == gr_pDecoder ) {
			printf("Failed to create decoder.\n");
			goto fail;
		}
			
		// Do Initialize
		nRetVal = gr_pDecoder->Init();
		if( nRetVal != MPEG_DECODER_ERROR_NO_ERROR ) {							// No decoder exist
			printf("Init failed, error code=%d\n", nRetVal );
			goto fail;
		}
		// Switch display mode to VGA 800x600
		gr_pDecoder->SetupDisplay( EM85xx_VGA_800x600 );
		s_bIsCreateHere = true;
	}
	else
		s_bIsCreateHere = false;

	nRetVal = gr_pDecoder->GetOSDBuffer(&osdbuffer);
	if(nRetVal != MPEG_DECODER_ERROR_NO_ERROR) {
		EPRINTF("Error getting the osd buffer\n");
		goto fail;
	} else {
		osd_buffer_addr = osdbuffer.framebuffer;
		psd->bpp = osdbuffer.bpp;
		psd->xres = psd->xvirtres = osdbuffer.width;
		psd->yres = psd->yvirtres = osdbuffer.height;
		/* set linelen to byte length, possibly converted later*/
		psd->linelen = osdbuffer.width;
		/* force subdriver init of size*/
		psd->size = osdbuffer.width * osdbuffer.height + 1024 + 8;
		psd->ncolors = (psd->bpp >= 24)? (1 << 24): (1 << psd->bpp);
		printf("OSD Buffer of size %d allocated @ %p\n", psd->size, osd_buffer_addr);
	}
	psd->addr = osd_buffer_addr + 8 + 1024;

	DPRINTF("End of qosd_open\n");

	status = 2;
	return psd;	/* success*/

fail:
	return NULL;
}

static void
qosd_close(PSD psd)
{
	/* if not opened, return*/
	if(status != 2)
		return;
	status = 1;

	/* close the kernel module driver */
	if( s_bIsCreateHere ) {
		gr_pDecoder->Exit();

		s_bIsCreateHere = false;
		gr_pDecoder = NULL;
	}
}

#endif	/* use RUA_OpenDevice */


// This trick transform [0..255] range into [0..65535] range (instead of about 0..255*256)
#define RANGE8TO16(x) (((x)<<8)|(x))

#define	MAX(a, b)	(((a) >= (b)) ? (a) : (b))
#define	MIN(a, b)	(((a) <= (b)) ? (a) : (b))

// see video demystified page 43
void gammacorrectedrgbtoyuv(unsigned short R,unsigned short G,unsigned short B,unsigned short *y,unsigned short *u,unsigned short *v)
{
	long yraw,uraw,vraw;
	
	yraw=( 257*R  +504*G + 98*B)/1000 + RANGE8TO16(16);
	uraw=(-148*R  -291*G +439*B)/1000 + RANGE8TO16(128);
	vraw=( 439*R  -368*G - 71*B)/1000 + RANGE8TO16(128);

	/* Obviously the computation of yraw garantees >= RANGE8TO16(16) ;-)
	   This is also true for uraw and vraw */
	
	*y=MAX(MIN(yraw,RANGE8TO16(235)),RANGE8TO16(16)); 
	*u=MAX(MIN(uraw,RANGE8TO16(240)),RANGE8TO16(16));
	*v=MAX(MIN(vraw,RANGE8TO16(240)),RANGE8TO16(16));
}

/* convert Microwindows palette to Quasar format and set it*/
static void
qosd_setpalette(PSD psd,int first, int count, MWPALENTRY *palette)
{
	int i;
// by yuanlii@29bbs.net, 3/ 7/2005 10:57:11
//	unsigned char *pal=osd_buffer_addr+8;
	unsigned char *pal=(ADDR8)osd_buffer_addr+8;
// by yuanlii@29bbs.net

#undef  TRACE
#define TRACE printf("In %s (%s:%d)\n", __FUNCTION__, __FILE__, __LINE__);
	TRACE
#undef TRACE
#define TRACE

	pal+=first*4;
	/* convert palette to quasar format*/
	for(i=0; i < count; i++) {
		MWPALENTRY *p = &palette[i];
		unsigned short Y,U,V;
		unsigned short R,G,B;
		

		// RGB->YUVe computation:
		
		R = RANGE8TO16(p->r);
		G = RANGE8TO16(p->g);
		B = RANGE8TO16(p->b);

		gammacorrectedrgbtoyuv(R,G,B,&Y,&U,&V);
//  	        vgargbtotvyuv(R,G,B,&Y,&U,&V);

//		printf("[%3d] RGB= %5d , %5d , %5d\t",i+first,R,G,B);
//		printf("[%3d] YUV= %5d , %5d , %5d\n",i+first,Y,U,V);

		// hardcode alpha blending values 

/*                if (i==0)
			pal[0] = 0x00;
		else if (i == 6)
			pal[0] = 0x66;
		else if (i == 15)
			pal[0] = 0x80;
		else if (i == 242)
			pal[0] = 0x80;
		else
			pal[0] = 0xff;
*/		
		pal[0] = 0xff;
		pal[1] = (unsigned char)(Y >> 8);
		pal[2] = (unsigned char)(U >> 8);
		pal[3] = (unsigned char)(V >> 8);

		pal+=4;
	}

}

static void
gen_getscreeninfo(PSD psd,PMWSCREENINFO psi)
{
//	TRACE

	psi->rows = psd->yvirtres;
	psi->cols = psd->xvirtres;
	psi->planes = psd->planes;
	psi->bpp = psd->bpp;
	psi->ncolors = psd->ncolors;
	psi->fonts = NUMBER_FONTS;
	psi->portrait = psd->portrait;
	psi->fbdriver = FALSE;	/* not running fb driver, can direct map*/

	psi->pixtype = psd->pixtype;
	switch (psd->pixtype) {
	case MWPF_TRUECOLOR0888:
	case MWPF_TRUECOLOR888:
		psi->rmask 	= 0xff0000;
		psi->gmask 	= 0x00ff00;
		psi->bmask	= 0x0000ff;
		break;
	case MWPF_TRUECOLOR565:
		psi->rmask 	= 0xf800;
		psi->gmask 	= 0x07e0;
		psi->bmask	= 0x001f;
		break;
	case MWPF_TRUECOLOR555:
		psi->rmask 	= 0x7c00;
		psi->gmask 	= 0x03e0;
		psi->bmask	= 0x001f;
		break;
	case MWPF_TRUECOLOR332:
		psi->rmask 	= 0xe0;
		psi->gmask 	= 0x1c;
		psi->bmask	= 0x03;
		break;
	case MWPF_PALETTE:
	default:
		psi->rmask 	= 0xff;
		psi->gmask 	= 0xff;
		psi->bmask	= 0xff;
		break;
	}

	if(psd->yvirtres > 480) {
		/* SVGA 800x600*/
		psi->xdpcm = 33;	/* assumes screen width of 24 cm*/
		psi->ydpcm = 33;	/* assumes screen height of 18 cm*/
	} else if(psd->yvirtres > 350) {
		/* VGA 640x480*/
		psi->xdpcm = 27;	/* assumes screen width of 24 cm*/
		psi->ydpcm = 27;	/* assumes screen height of 18 cm*/
        } else if(psd->yvirtres <= 240) {
		/* half VGA 640x240 */
		psi->xdpcm = 14;        /* assumes screen width of 24 cm*/ 
		psi->ydpcm =  5;
	} else {
		/* EGA 640x350*/
		psi->xdpcm = 27;	/* assumes screen width of 24 cm*/
		psi->ydpcm = 19;	/* assumes screen height of 18 cm*/
	}
}

static void qosd_drawpixel(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c)
{
// by yuanlii@29bbs.net, 3/ 7/2005 10:57:37
//	ADDR8	addr = psd->addr;
	ADDR8	addr = (ADDR8)psd->addr;
// by yuanlii@29bbs.net

//	TRACE

	assert (addr != 0)
	assert (x >= 0 && x < psd->xres)
	assert (y >= 0 && y < psd->yres)
	assert (c < psd->ncolors)

//	DRAWON;
	if(gr_mode == MWMODE_COPY)
		addr[x + y * psd->linelen] = c;
	else
		applyOp(gr_mode, c, &addr[ x + y * psd->linelen], ADDR8);
//	DRAWOFF;
}

/* Read pixel at x, y*/
static MWPIXELVAL qosd_readpixel(PSD psd, MWCOORD x, MWCOORD y)
{
// by yuanlii@29bbs.net, 3/ 7/2005 10:57:37
//	ADDR8	addr = psd->addr;
	ADDR8	addr = (ADDR8)psd->addr;
// by yuanlii@29bbs.net

//	TRACE

	assert (addr != 0)
	assert (x >= 0 && x < psd->xres)
	assert (y >= 0 && y < psd->yres)

	return addr[x + y * psd->linelen];
}

/* Draw horizontal line from x1,y to x2,y including final point*/
static void qosd_drawhorzline(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y,
			      MWPIXELVAL c)
{
// by yuanlii@29bbs.net, 3/ 7/2005 10:57:37
//	ADDR8	addr = psd->addr;
	ADDR8	addr = (ADDR8)psd->addr;
// by yuanlii@29bbs.net

//	TRACE
		
	assert (addr != 0)
	assert (x1 >= 0 && x1 < psd->xres)
	assert (x2 >= 0 && x2 < psd->xres)
	assert (x2 >= x1)
	assert (y >= 0 && y < psd->yres)
	assert (c < psd->ncolors)
	
//	DRAWON;
	addr += x1 + y * psd->linelen;
	if(gr_mode == MWMODE_COPY)
		memset(addr, c, x2 - x1 + 1);
	else {
		while(x1++ <= x2) {			
			applyOp(gr_mode, c, addr, ADDR8);
			++addr;
		}		
	}
//	DRAWOFF;
}

/* Draw a vertical line from x,y1 to x,y2 including final point*/
static void qosd_drawvertline(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2,
			      MWPIXELVAL c)
{
// by yuanlii@29bbs.net, 3/ 7/2005 10:57:37
//	ADDR8	addr = psd->addr;
	ADDR8	addr = (ADDR8)psd->addr;
// by yuanlii@29bbs.net
	int	linelen = psd->linelen;

//	TRACE

	assert (addr != 0)
	assert (x >= 0 && x < psd->xres)
	assert (y1 >= 0 && y1 < psd->yres)
	assert (y2 >= 0 && y2 < psd->yres)
	assert (y2 >= y1)
	assert (c < psd->ncolors)

//	DRAWON;
	addr += x + y1 * linelen;
	if(gr_mode == MWMODE_COPY) {
		while(y1++ <= y2) {
			*addr = c;
			addr += linelen;
		}
	} else {
		while(y1++ <= y2) {
			applyOp(gr_mode, c, addr, ADDR8);
			addr += linelen;
		}
	}
//	DRAWOFF;
}

//#define memcpy(d,s,nbytes)	memcpy16(d,s,(nbytes)>>1)
#define memmove(d,s,nbytes)	memcpy16(d,s,(nbytes)>>1)
static void
memcpy16(unsigned short *dst, unsigned short *src, int nwords)
{
	while (--nwords >= 0)
		*dst++ = *src++;
}

/* srccopy bitblt*/
static void qosd_blit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD w,
		      MWCOORD h, PSD srcpsd, MWCOORD srcx, MWCOORD srcy,
		      long op)
{
	ADDR8	dst;
	ADDR8	src;
	int	dlinelen = dstpsd->linelen;
	int	slinelen = srcpsd->linelen;
#if 0 //ALPHABLEND
	unsigned int srcalpha, dstalpha;
#endif
	
	TRACE

	assert (dstpsd->addr != 0)
	assert (dstx >= 0 && dstx < dstpsd->xres)
	assert (dsty >= 0 && dsty < dstpsd->yres)
	assert (w > 0)
	assert (h > 0)
	assert (srcpsd->addr != 0)
	assert (srcx >= 0 && srcx < srcpsd->xres)
	assert (srcy >= 0 && srcy < srcpsd->yres)
	assert (dstx+w <= dstpsd->xres)
	assert (dsty+h <= dstpsd->yres)
	assert (srcx+w <= srcpsd->xres)
	assert (srcy+h <= srcpsd->yres)

//	DRAWON;
// by yuanlii@29bbs.net, 3/ 7/2005 11:02:30
//	dst = dstpsd->addr + dstx + dsty * dlinelen;
//	src = srcpsd->addr + srcx + srcy * slinelen;
	dst = (ADDR8)dstpsd->addr + dstx + dsty * dlinelen;
	src = (ADDR8)srcpsd->addr + srcx + srcy * slinelen;
// by yuanlii@29bbs.net

	if (op == MWROP_COPY) {
		/* copy from bottom up if dst in src rectangle*/
		/* memmove is used to handle x case*/
		if (srcy < dsty) {
			src += (h-1) * slinelen;
			dst += (h-1) * dlinelen;
			slinelen *= -1;
			dlinelen *= -1;
		}

		while(--h >= 0) {
			/* a _fast_ memcpy is a _must_ in this routine*/
// by yuanlii@29bbs.net, 3/ 7/2005 11:08:44
//			memmove(dst, src, w);
			memmove((ADDR16)dst, (ADDR16)src, w);
// by yuanlii@29bbs.net
			dst += dlinelen;
			src += slinelen;
		}
	} else {
		while (--h >= 0) {
			int i;
			for (i=0; i<w; i++) {
				applyOp(MWROP_TO_MODE(op), *src, dst, ADDR8);
				++src;
				++dst;
			}
			dst += dlinelen - w;
			src += slinelen - w;
		}
	}
//	DRAWOFF;
}

/* srccopy stretchblt*/
static void qosd_stretchblit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty,
			     MWCOORD dstw, MWCOORD dsth, PSD srcpsd,
			     MWCOORD srcx, MWCOORD srcy, MWCOORD srcw,
			     MWCOORD srch, long op)
{
	ADDR8	dst;
	ADDR8	src;
	int	dlinelen = dstpsd->linelen;
	int	slinelen = srcpsd->linelen;
	int	i, ymax;
	int	row_pos, row_inc;
	int	col_pos, col_inc;
	unsigned char pixel = 0;
	
	TRACE

	assert (dstpsd->addr != 0)
	assert (dstx >= 0 && dstx < dstpsd->xres)
	assert (dsty >= 0 && dsty < dstpsd->yres)
	assert (dstw > 0)
	assert (dsth > 0)
	assert (srcpsd->addr != 0)
	assert (srcx >= 0 && srcx < srcpsd->xres)
	assert (srcy >= 0 && srcy < srcpsd->yres)
	assert (srcw > 0)
	assert (srch > 0)
	assert (dstx+dstw <= dstpsd->xres)
	assert (dsty+dsth <= dstpsd->yres)
	assert (srcx+srcw <= srcpsd->xres)
	assert (srcy+srch <= srcpsd->yres)

//	DRAWON;
	row_pos = 0x10000;
	row_inc = (srch << 16) / dsth;

	/* stretch blit using integer ratio between src/dst height/width*/
	for (ymax = dsty+dsth; dsty<ymax; ++dsty) {

		/* find source y position*/
		while (row_pos >= 0x10000L) {
			++srcy;
			row_pos -= 0x10000L;
		}

// by yuanlii@29bbs.net, 3/ 7/2005 11:03:15
//		dst = dstpsd->addr + dstx + dsty*dlinelen;
//		src = srcpsd->addr + srcx + (srcy-1)*slinelen;
		dst = (ADDR8)dstpsd->addr + dstx + dsty*dlinelen;
		src = (ADDR8)srcpsd->addr + srcx + (srcy-1)*slinelen;
// by yuanlii@29bbs.net

		/* copy a row of pixels*/
		col_pos = 0x10000;
		col_inc = (srcw << 16) / dstw;
		for (i=0; i<dstw; ++i) {
			/* get source x pixel*/
			while (col_pos >= 0x10000L) {
				pixel = *src++;
				col_pos -= 0x10000L;
			}
			*dst++ = pixel;
			col_pos += col_inc;
		}

		row_pos += row_inc;
	}
//	DRAWOFF;
}

/***************************************************************************
 *	Author     : yuanlii@29bbs.net			_________________  *
 *	Date       : 01/19/2005 11:33:05		|    | This file | *
 *	File name  : not valid for this file		| vi |	powered  | *
 *	Description: The MapMemGC for OSD layer: 	|____|___________| *
 *			qosd_mapmemgc					   *
 *			int qosd_init(PSD psd);				   *
 *			SUBDRIVER qosd_yldriver{};			   *
 ***************************************************************************/
int 
qosd_init(PSD psd)
{
	if (!psd->size) {
		psd->size = psd->yres * psd->linelen;
		/* convert linelen from byte to pixel len for bpp 16, 24, 32*/
#undef  TRACE
#define TRACE printf("In %s (%s:%d), psd->linelen=%d\n", __FUNCTION__, __FILE__, __LINE__, psd->linelen);
	TRACE
#undef TRACE
#define TRACE
//	psd->linelen /= 1;
	}
	return 1;
}

SUBDRIVER qosd_driver = {
	qosd_init,
	qosd_drawpixel,
	qosd_readpixel,
	qosd_drawhorzline,
	qosd_drawvertline,
	gen_fillrect,
	qosd_blit,
	NULL,
	qosd_stretchblit
};

static MWBOOL
qosd_mapmemgc(PSD mempsd,MWCOORD w,MWCOORD h,int planes,int bpp,
	int linelen,int size,void *addr)
{
	initmemgc(mempsd, w, h, planes, bpp, linelen, size, addr);
	/* set and initialize subdriver into mem screen driver*/
	if (!set_subdriver(mempsd, &qosd_driver, TRUE))
		return 0;

	return 1;
}

void qosd_settransparent(unsigned char c, MWPIXELVAL index)
{
	unsigned char *p;
	p = (unsigned char *)osd_buffer_addr + 8 + 4*index;
	*p = c;
}

#if	SPEEDUP_DEVICE
static void
qosd_drawblock(PSD psd,MWCOORD x,MWCOORD y,MWPIXELVAL *c,
	       	MWCOORD width, MWCOORD height)
{
	ADDR8	addr = (ADDR8)psd->addr;
	ADDR8	ptr;
	int	i, j;
		
	assert (addr != 0)
	assert (x >= 0 && (x + width - 1) < psd->xres)
	assert (y >= 0 && (y + height - 1) < psd->yres)
	
	addr += x + y * psd->linelen;
	if(gr_mode == MWMODE_COPY) {
		for (j = 0; j < height; j++) {
			memcpy(addr, c + j * width * sizeof(MWPIXELVAL),
				       	width * sizeof(MWPIXELVAL));
			addr += psd->linelen;
		}
	} else {
		c = (ADDR8)c;
		
		for (j = 0; j < height; j++)
			ptr = addr + j * psd->linelen;
			for (i = 0; i < width; i++) {
				applyOp(gr_mode, c[j * width + i], ptr, ADDR8);
				++ptr;
			}
	}
}

static void
qosd_readblock(PSD psd,MWCOORD x,MWCOORD y,MWPIXELVAL *c,
	       	MWCOORD width, MWCOORD height)
{
	ADDR8	addr = (ADDR8)psd->addr;
	ADDR8	ptr;
	int	i, j;
		
	assert (addr != 0)
	assert (x >= 0 && (x + width - 1) < psd->xres)
	assert (y >= 0 && (y + height - 1) < psd->yres)
	
	addr += x + y * psd->linelen;
	if(gr_mode == MWMODE_COPY) {
		for (j = 0; j < height; j++) {
			memcpy(c + j * width * sizeof(MWPIXELVAL), addr, 
				       	width * sizeof(MWPIXELVAL));
			addr += psd->linelen;
		}
	} else {
		c = (ADDR8)c;
		
		for (j = 0; j < height; j++)
			ptr = addr + j * psd->linelen;
			for (i = 0; i < width; i++) {
				applyOp(gr_mode, *ptr, c + j * width + i, ADDR8);
				++ptr;
			}
	}
}

static void
qosd_drawline(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL *c, int nCount)
{
	int	i;
	ADDR8	addr = (ADDR8)psd->addr;

	assert (addr != 0)
	assert (x >= 0 && (x + nCount - 1) < psd->xres)
	assert (y >= 0 && y < psd->yres)

	addr += x + y * psd->linelen;
	if (gr_mode == MWMODE_COPY)
		memcpy(addr, c, nCount * sizeof(MWPIXELVAL));
	else {
		for (i = 0; i < nCount; i++) {
			applyOp(gr_mode, c[i], addr, ADDR8);
			++addr;
		}
	}

}
#endif/*SPEEDUP_DEVICE*/
// by yuanlii@29bbs.net
