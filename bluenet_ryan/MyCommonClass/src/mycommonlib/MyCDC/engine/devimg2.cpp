/***************************************************************************
 *	Author     : yuanlii@29bbs.net			_________________  *
 *	Date       : 03/21/2005 15:32:47		|    | This file | *
 *	File name  : devimg3.cpp			| vi |	powered  | *
 *	Description: Display image in pimage to YUV	|____|___________| *
 *									   *
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
extern "C" {
#	include <device.h>
#	include <windef.h>
}
#include <mympegdec.h>

extern IMyMpegDecoder	*gr_pDecoder;
extern MWPALENTRY gr_palette[256];    /* current palette*/
extern void qosd_settransparent(unsigned char c, MWPIXELVAL index);

/* The display cofficients */
static RMint32 FRAME_WIDTH;		/* the frame width of YUV plane */
static RMint32 FRAME_HEIGHT;		/* the frame height of YUV plane */

static void gammacorrectedrgbtoyuv(MWUCHAR R,MWUCHAR G,MWUCHAR B, MWUCHAR *y,MWUCHAR *u,MWUCHAR *v);
void
GdSetTransparent(MWCOORD x, MWCOORD y, MWCOORD w, MWCOORD h, BYTE c)/*{{{*/
{				/* (0, 128, 128) is transparent */
	MWPIXELVAL	index = GdFindColor(TRANS_COLOR);
	MWPIXELVAL	oldfg = GdSetForeground(index);
	qosd_settransparent(c, index);
	GdFillRect(&scrdev, x, y, w, h);
	GdSetForeground(oldfg);
}/*}}}*/

extern "C" void
DrawImageToYUV(PSD psd, MWCOORD x, MWCOORD y, PMWIMAGEHDR pimage )
{
	int		clip;
	int		rgborder;
	MWCOORD		yoff;
	MWCOORD		height, width;
	unsigned long	transcolor;

	int		i, index;
	MWUCHAR		Y, U, V;
	MWPALENTRY	*palette;
	MWCOORD		xPos, yPos;
	MWPALENTRY	YUVpal[256];
	unsigned int	deltaY, deltaUV;
	MWUCHAR		*pRow0, *pRow1;
	int		bufferSize;	/* buffer size, dispWidth*dispHeight */
	MWCOORD		dispWidth, dispHeight;

	assert(pimage);

	height = pimage->height;
	width = pimage->width;
	transcolor = pimage->transcolor;

	/* determine if entire image is clipped out, save clipresult for later*/
	clip = GdClipArea(psd, x, y, x + width - 1, y + height - 1);
	if(clip == CLIP_INVISIBLE)
		return;

	/* modify x, y, height&width to be even */
	if (1 == (x%2))
		x--;
	if (1 == (y%2))
		y--;
	if (1 == (width%2))
		width--;
	if (1 == (height%2))
		height--;
	xPos = x;
	yPos = y;
	dispWidth = width;
	dispHeight = height;

	/* allocate memory for YUV display */
	bufferSize = dispWidth * dispHeight;
	if (bufferSize <= 0) {
		fprintf(stderr, "It is too faint to draw the picture there\n");
		return;	/* not fatal error, just can't display it*/
	}
	MWUCHAR *ary_Y = (MWUCHAR *)malloc(bufferSize * sizeof(RMuint8));
	if (NULL == ary_Y) {
		fprintf(stderr, "Allocate memory error\n");
		return;
	}
	MWUCHAR *ary_UV = (MWUCHAR *)malloc((bufferSize/2) * sizeof(RMuint8));
	if (NULL == ary_UV) {
		fprintf(stderr, "Allocate memory error\n");
		free(ary_Y);
		return;
	}

	/* 4:4:4 YCbCr true color image,convert it to 4:2:0 */
	if (257 == pimage->palsize) {
		for (height = 0; height < dispHeight; height += 2) {
			pRow0 = pimage->imagebits 
				+ height * pimage->pitch;
			pRow1 = pRow0 + pimage->pitch;
			deltaY = height * dispWidth;
			deltaUV = height * dispWidth / 2;

			for ( i = 0; i < dispWidth; i += 2 )
			{

				ary_Y[deltaY] = pRow0[0];
				ary_Y[deltaY + 1] = pRow0[3];
				ary_Y[deltaY + dispWidth] = pRow1[0];
				ary_Y[deltaY + dispWidth + 1] = pRow1[3];
				deltaY += 2;

				ary_UV[deltaUV] = pRow1[1];
				ary_UV[deltaUV + 1] = pRow0[2];
				deltaUV += 2;

				pRow0 += 6;
				pRow1 += 6;
			}
		}
		goto display;
	}

	/*
	 * Use the system or the pimage's palette to draw the image
	 */
	if (pimage->bpp <= 8) {
		if(!pimage->palette) {
			/* use the system palette */
			palette = gr_palette;
		} else {
			/* use the image's private palette */
			palette = pimage->palette;
		}
		/* convert RGB palette to YUV palette */
		for (yoff=0; yoff<256; ++yoff) {
			gammacorrectedrgbtoyuv(palette[yoff].r,
				       	       palette[yoff].g,
					       palette[yoff].b, 
					       &Y, &U, &V);
				YUVpal[yoff].r = Y;
				YUVpal[yoff].g = U;
				YUVpal[yoff].b = V;

		}
	}

	/* check for bottom-up image*/
	if(pimage->compression & MWIMAGE_UPSIDEDOWN)
		yoff = -1;
	else
		yoff = 1;

	/* 24bpp RGB rather than BGR byte order?*/
	rgborder = pimage->compression & MWIMAGE_RGB; 

	height = 0;
/*	if ((pimage->bpp == 32)) {					*/
/*	    && ((pimage->compression & MWIMAGE_ALPHA_CHANNEL) != 0)) {	*/
/*		printf("true-color bitmap with transparency\n");	*/
	switch (pimage->bpp) {
	    case 32:
	    case 24:
		while (height <= dispHeight) {
			if (1 == yoff) {
				pRow0 = pimage->imagebits 
					+ height * pimage->pitch;
				pRow1 = pRow0 + pimage->pitch;
			} else {
				pRow0 = pimage->imagebits
					+ (dispHeight-height-1)*pimage->pitch;
				pRow1 = pRow0 - pimage->pitch;
			}
			deltaY = height * dispWidth;
			deltaUV = height * dispWidth / 2;
			
			/* convert 2 column each time */
			for (i = 0; i < dispWidth; i += 2) {
				if (rgborder) {
					gammacorrectedrgbtoyuv(
						pRow0[0], pRow0[1], pRow0[2],
						&Y, &U, &V);
				} else {
					gammacorrectedrgbtoyuv(
						pRow0[2], pRow0[1], pRow0[0],
						&Y, &U, &V);
				}
				*(ary_Y + deltaY) = Y;
				*(ary_UV + deltaUV) = U;

				if (rgborder) {
					gammacorrectedrgbtoyuv(
						pRow1[0], pRow1[1], pRow1[2],
						&Y, &U, &V);
				} else {
					gammacorrectedrgbtoyuv(
						pRow1[2], pRow1[1], pRow1[0],
						&Y, &U, &V);
				}
				*(ary_Y + deltaY + dispWidth) = Y;
				*(ary_UV + deltaUV + 1) = V;

				/* go through the transparency of 32bpp */
				if (32 == pimage->bpp) {
					pRow0 += 4;
					pRow1 += 4;
				} else {
					pRow0 += 3;
					pRow1 += 3;
				}

				if (rgborder) {
					gammacorrectedrgbtoyuv(
						pRow0[0], pRow0[1], pRow0[2],
						&Y, &U, &V);
				} else {
					gammacorrectedrgbtoyuv(
						pRow0[2], pRow0[1], pRow0[0],
						&Y, &U, &V);
				}
				*(ary_Y + deltaY + 1) = Y;

				if (rgborder) {
					gammacorrectedrgbtoyuv(
						pRow1[0], pRow1[1], pRow1[2],
						&Y, &U, &V);
				} else {
					gammacorrectedrgbtoyuv(
						pRow1[2], pRow1[1], pRow1[0],
						&Y, &U, &V);
				}
				*(ary_Y+deltaY+dispWidth+1) = Y;

				if (32 == pimage->bpp) {
					pRow0 += 4;
					pRow1 += 4;
				} else {
					pRow0 += 3;
					pRow1 += 3;
				}
				deltaY += 2;
				deltaUV += 2;
			}
			height += 2;
		}
		break;
	    case 8:  /* bpp == 8, 4, 1, palettized image */
		while (height <= dispHeight) {
			if (1 == yoff) {
				pRow0 = pimage->imagebits
				       	+ height * pimage->pitch;
				pRow1 = pRow0 + pimage->pitch;
			} else {
				pRow0 = pimage->imagebits
				       	+ (dispHeight-height-1)*pimage->pitch;
				pRow1 = pRow0 - pimage->pitch;
			}
			deltaY = height * dispWidth;
			deltaUV = height * dispWidth / 2;

			/* convert 2 column each time */
			for (i = 0; i < dispWidth; i += 2) {
				/* Don't forget the palette is YUV palette */
				index = pRow0[0];	// left-top pixel
				*(ary_Y+deltaY) = YUVpal[index].r;
				*(ary_UV+deltaUV) = YUVpal[index].g;

				index = pRow1[0];	// left-bottom pixel
				*(ary_Y+deltaY+dispWidth) = YUVpal[index].r;
				*(ary_UV+deltaUV+1) = YUVpal[index].b;

				index = pRow0[1];	// right-top pixel
				*(ary_Y+deltaY+1) = YUVpal[index].r;

				index = pRow1[1];	// right-bottom pixel
				*(ary_Y+deltaY+dispWidth+1)=YUVpal[index].r;

				pRow0 += 2;
				pRow1 += 2;
				deltaY += 2;
				deltaUV += 2;
			}
			height += 2;
		}
		break;
	    case 4:
		while (height <= dispHeight) {
			if (1 == yoff) {
				pRow0 = pimage->imagebits 
					+ height * pimage->pitch;
				pRow1 = pRow0 + pimage->pitch;
			} else {
				pRow0 = pimage->imagebits
				       	+ (dispHeight-height-1)*pimage->pitch;
				pRow1 = pRow0 - pimage->pitch;
			}
			deltaY = height*dispWidth;
			deltaUV = height*dispWidth/2;

			/* convert 2 column each time */
			for (i = 0; i < dispWidth; i += 2) {
				/* Don't forget the palette is YUV palette */
				index = (pRow0[0] & 0xf0) >> 4;
				*(ary_Y+deltaY) = YUVpal[index].r;
				*(ary_UV+deltaUV) = YUVpal[index].g;

				index = (pRow1[0] & 0xf0) >> 4;
				*(ary_Y+deltaY+dispWidth) = YUVpal[index].r;
				*(ary_UV+deltaUV+1) = YUVpal[index].b;

				index = pRow0[0] & 0x0f;
				*(ary_Y+deltaY+1) = YUVpal[index].r;

				index = pRow1[0] & 0x0f;
				*(ary_Y+deltaY+dispWidth+1)=YUVpal[index].r;

				pRow0++;
				pRow1++;
				deltaY += 2;
				deltaUV += 2;
			}
			height += 2;
		}
		break;
	    case 1:
		while (height <= dispHeight) {
			if (1 == yoff) {
				pRow0 = pimage->imagebits 
					+ height * pimage->pitch;
				pRow1 = pRow0 + pimage->pitch;
			} else {
				pRow0 = pimage->imagebits
				       	+ (dispHeight-height-1)*pimage->pitch;
				pRow1 = pRow0 - pimage->pitch;
			}
			deltaY = height*dispWidth;
			deltaUV = height*dispWidth/2;

			/* convert 2 column each time */
			for (i = 0; i < dispWidth; i += 2) {
				/* Don't forget the palette is YUV palette */
				index = (pRow0[i/8] & (0x80 >> (i%8))) ? 1 : 0;
				*(ary_Y+deltaY) =YUVpal[index].r;
				*(ary_UV+deltaUV) =YUVpal[index].g;

				index = (pRow1[i/8] & (0x80 >> (i%8))) ? 1 : 0;
				*(ary_Y+deltaY+dispWidth) =YUVpal[index].r;
				*(ary_UV+deltaUV+1) =YUVpal[index].b;

				index = (pRow0[(i+1)/8] & (0x80 >> ((i+1)%8))) ? 1 : 0;
				*(ary_Y+deltaY+1) =YUVpal[index].r;

				index = (pRow1[(i+1)/8] & (0x80 >> ((i+1)%8))) ? 1 : 0;
				*(ary_Y+deltaY+dispWidth+1) =YUVpal[index].r;

				deltaY += 2;
				deltaUV += 2;
			}

			height += 2;
		}
		break;
	    default:
		printf("unsupported bpp bitmap\n");
		free(ary_Y);
		free(ary_UV);
		return;
	}

/* by yuanlii@29bbs.net, 03/14/2005 13:39:07 */
display: 
	/* if the image is not invisible, display it */
	if (clip !=  CLIP_INVISIBLE) { 
		GdSetTransparent(xPos, yPos, dispWidth, dispHeight - 4, 0x00);
		gr_pDecoder->GetScreenDimensions( &FRAME_WIDTH, &FRAME_HEIGHT,
				(RMint32 *)0, (RMint32 *)0 );
		gr_pDecoder->DisplayYUV420(ary_Y, ary_UV, xPos, yPos,
				dispWidth, dispHeight,
				FRAME_WIDTH, FRAME_HEIGHT);
	}
/* by yuanlii@29bbs.net */

	free(ary_Y);
	free(ary_UV);
}

#define RANGE8TO16(x) (((x)<<8)|(x))
#define MAX(a, b) 	( (a) > (b) ? (a) : (b) )
#define MIN(a, b) 	( (a) < (b) ? (a) : (b) )

static void
gammacorrectedrgbtoyuv(MWUCHAR R,MWUCHAR G,MWUCHAR B, MWUCHAR *y,MWUCHAR *u,MWUCHAR *v)
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

