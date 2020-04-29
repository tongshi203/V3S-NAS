/****************************************************************************
 *	Author     : yuanlii@29bbs.net			_________________   *
 *	Date       : 3/ 7/2005 15:08:50			|    | This file |  *
 *	File name  : yuvimage.h				| vi |	powered  |  *
 *	Description: the header file fot display image	|____|___________|  *
 *			on YUV plane, only BMP/JPEG/GIF was supported	    *
 ****************************************************************************/
#ifndef _YUVIMAGE_H_
#define _YUVIMAGE_H_
#include <mympegdec.h>
/**
 *	@GdSetTransparent
 *	 Set the OSD plane transparent for display images in YUV plane
 *	@param x, y the position of the top-left corner
 *	@param w, h the dimension of the rectangle
 *	@param c the transparent of the rectangle, 0x00-0xFF
 *	@no return value
 */
void GdSetTransparent(MWCOORD x, MWCOORD y, MWCOORD w, MWCOORD h, BYTE c);

/**Function description
 *	Draw the image from file
 *	@param path which file is to be display
 *	@param xPos
 *	       yPos the position where the image to be displayed
 *	@param iScale the resize scale of the image, only valid with JPEG file
 *	@return 1 when success 
 *		0 when fatal error
 *		otherwise when common error
 */
//int DrawYUVImageFromFile(char *path, 
//				RMint32 xPos, RMint32 yPos, RMint32 iScale);

/**
 *	Draw the image from buffer
 *	@param buffer the memory buffer contains the image
 *	@param size the buffer size of the image
 *	@param xPos
 *	       yPos the position where the image to be displayed
 *	@param iScale the resize scale of the image, only valid with JPEG file
 *	@return 1 when success 
 *		0 when fatal error
 *		otherwise when common error
 */
//int DrawYUVImageFromBuffer(void *buffer, RMint32 size, 
//				RMint32 xPos, RMint32 yPos, RMint32 iScale);
#endif/*_YUVIMAGE_H_*/
