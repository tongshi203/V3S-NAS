#define FASTJPEG	1	/* =1 for temp quick jpeg 8bpp display */
#undef HAVE_MMAP
// #define HAVE_MMAP       0       /* =1 to use mmap if available         */
#define HAVE_JPEG_SUPPORT	// only support jpeg&gif now!
#define HAVE_BMP_SUPPORT
#define HAVE_GIF_SUPPORT
//#define HAVE_STB_SUPPORT	/* for TS EM8510 STB */

// after debug, comment the line below
//#define	__DEBUG
//
// by yuanlii@29bbs.net, 1/31/2005 13:17:22
// default have file io
//#if defined(HAVE_FILEIO)	/* temp for entire file*/
// by yuanlii@29bbs.net

/*
 * Copyright (c) 2000, 2001 Greg Haerr <greg@censoft.com>
 * Portions Copyright (c) 2000 Martin Jolicoeur <martinj@visuaide.com>
 * Portions Copyright (c) 2000 Alex Holden <alex@linuxhacker.org>
 * Portions Copyright (c) Independant JPEG group (ijg)
 *
 * Image load/cache/resize/display routines
 *
 * GIF, BMP and JPEG formats are supported.
 * JHC:  Instead of working with a file, we work with a buffer
 *       (either provided by the user or through mmap).  This
 *	 improves speed, and provides a mechanism by which the
 *	 client can send image data directly to the engine 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif

#include <zlib.h>
 
#include "device.h"
#include "swap.h"
#include "mylibb.h"

/* cached image list*/
typedef struct {
	MWLIST		link;		/* link list*/
	int		id;		/* image id*/
	PMWIMAGEHDR	pimage;		/* image data*/
	PSD		psd;		/* FIXME shouldn't need this*/
} IMAGEITEM, *PIMAGEITEM;

/* by yuanlii@29bbs.net, 03/21/2005 15:14:01 */
MWBOOL gr_toosd = TRUE;
/* by yuanlii@29bbs.net */
static MWLISTHEAD imagehead;		/* global image list*/
static int nextimageid = 1;

void ComputePitch(int bpp, int width, int *pitch, int *bytesperpixel);
#if defined(HAVE_JPEG_SUPPORT)
static int  LoadJPEG(buffer_t *src, PMWIMAGEHDR pimage, PSD psd,
		MWBOOL fast_grayscale);
#endif
#if defined(HAVE_BMP_SUPPORT)
static int  LoadBMP(buffer_t *src, PMWIMAGEHDR pimage);
#endif
#if defined(HAVE_GIF_SUPPORT)
static int  LoadGIF(buffer_t *src, PMWIMAGEHDR pimage);
#endif
#if defined(HAVE_STB_SUPPORT)
static int LoadSTB(buffer_t *src, PMWIMAGEHDR pimage);
#endif

static int GdDecodeImage(PSD psd, buffer_t *src, int flags);

extern void DrawImageToYUV(PSD psd, MWCOORD x, MWCOORD y, PMWIMAGEHDR pimage );

int
GdLoadImageFromBuffer(PSD psd, void *buffer, int size, int flags)
{
	buffer_t src;
	binit(buffer, size, &src);

	return(GdDecodeImage(psd, &src, flags));
}

void
GdDrawImageFromBuffer(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width,
	MWCOORD height, void *buffer, int size, int flags)
{
	int id;
	buffer_t src;

	binit(buffer, size, &src);
	id = GdDecodeImage(psd, &src, flags);

	if (id) {
		GdDrawImageToFit(psd, x, y, width, height, id);
		GdFreeImage(id);
	}
}

void
GdDrawImageFromFile(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width,
	MWCOORD height, char *path, int flags)
{
	int	id;

	id = GdLoadImageFromFile(psd, path, flags);
	if (id) {
		GdDrawImageToFit(psd, x, y, width, height, id);
		GdFreeImage(id);
	}
}

int
GdLoadImageFromFile(PSD psd, char *path, int flags)
{
  int fd, id;
  struct stat s;
  void *buffer = 0;
  buffer_t src;
  
  fd = open(path, O_RDONLY);
  if (fd < 0 || fstat(fd, &s) < 0) {
    EPRINTF("GdLoadImageFromFile: can't open image: %s\n", path);
    return(0);
  }
  
#ifdef HAVE_MMAP
  buffer = mmap(0, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  if (!buffer) {
    EPRINTF("GdLoadImageFromFile: Couldn't map image %s\n", path);
    close(fd);
    return(0);
  }
#else
  buffer = malloc(s.st_size);
  if (!buffer) {
     EPRINTF("GdLoadImageFromFile: Couldn't load image %s\n", path);
     close(fd);
     return(0);
  }
  
  if (read(fd, buffer, s.st_size) != s.st_size) {
    EPRINTF("GdLoadImageFromFile: Couldn't load image %s\n", path);
    free(buffer);
    close(fd);
    return(0);
  }
#endif

  binit(buffer, s.st_size, &src);
  id = GdDecodeImage(psd, &src, flags);
  
#ifdef HAVE_MMAP
  munmap(buffer, s.st_size);
#else
  free(buffer);
#endif

  close(fd);
  return(id);
}

static int
GdDecodeImage(PSD psd, buffer_t * src, int flags)
{
        int         loadOK = 0;
        PMWIMAGEHDR pimage;
        PIMAGEITEM  pItem;

	/* allocate image struct*/
	pimage = (PMWIMAGEHDR)malloc(sizeof(MWIMAGEHDR));
	if(!pimage) {
		return 0;
	}
	pimage->imagebits = NULL;
	pimage->palette = NULL;
	pimage->transcolor = -1L;

#if defined(HAVE_STB_SUPPORT)
	if (loadOK == 0) {
		loadOK = LoadSTB(src, pimage);
	}
#endif
#if defined(HAVE_BMP_SUPPORT)
	if (loadOK == 0) 
		loadOK = LoadBMP(src, pimage);
#endif
#if defined(HAVE_GIF_SUPPORT)
	if (loadOK == 0) 
		loadOK = LoadGIF(src, pimage);
#endif
#if defined(HAVE_JPEG_SUPPORT)
	if (loadOK == 0) 
		loadOK = LoadJPEG(src, pimage, psd, flags);
#endif

	if (loadOK == 0) {
		EPRINTF("GdLoadImageFromFile: unknown image type\n");
		goto err;		/* image loading error*/
	}
	if (loadOK != 1)
		goto err;		/* image loading error*/

	/* allocate id*/
	pItem = GdItemNew(IMAGEITEM);
	if (!pItem)
		goto err;
	pItem->id = nextimageid++;
	pItem->pimage = pimage;
	pItem->psd = psd;
	GdListAdd(&imagehead, &pItem->link);

	return pItem->id;

err:
	free(pimage);
	return 0;			/* image loading error*/
}

PIMAGEITEM
findimage(int id)
{
	PMWLIST		p;
	PIMAGEITEM	pimagelist;

	for (p=imagehead.head; p; p=p->next) {
		pimagelist = GdItemAddr(p, IMAGEITEM, link);
		if (pimagelist->id == id)
			return pimagelist;
	}
	return NULL;
}

void
GdDrawImageToFit(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height,
	int id)
{
	PIMAGEITEM	pItem;
	PMWIMAGEHDR	pimage;

	pItem = findimage(id);
	if (!pItem)
		return;
	pimage = pItem->pimage;

	/*
	 * Display image, possibly stretch/shrink to resize
	 */
	if (height <= 0)
		height = pimage->height;
	if (width <= 0)
		width = pimage->width;

	if (height != pimage->height || width != pimage->width) {
		MWCLIPRECT	rcDst;
		MWIMAGEHDR	image2;

		/* create similar image, different width/height*/

		image2.width = width;
		image2.height = height;
		image2.planes = pimage->planes;
		image2.bpp = pimage->bpp;
		ComputePitch(pimage->bpp, width, &image2.pitch,
			&image2.bytesperpixel);
		image2.compression = pimage->compression;
		image2.palsize = pimage->palsize;
		image2.palette = pimage->palette;	/* already allocated*/
		image2.transcolor = pimage->transcolor;
		if( (image2.imagebits = malloc(image2.pitch*height)) == NULL) {
			EPRINTF("GdDrawImageToFit: no memory\n");
			return;
		}

		rcDst.x = 0;
		rcDst.y = 0;
		rcDst.width = width;
		rcDst.height = height;

		/* Stretch full soruce to destination rectangle*/
		GdStretchImage(pimage, NULL, &image2, &rcDst);

		
		if (gr_toosd)		/* Display image2 to osd */
			GdDrawImage(psd, x, y, &image2);
		else			/* Display image2 to yuv */
			DrawImageToYUV(psd, x, y, &image2);

		free(image2.imagebits);
	} else
		if (gr_toosd)		/* Display image to osd */
			GdDrawImage(psd, x, y, pimage);
//			;
		else
			DrawImageToYUV(psd, x, y, pimage);
}

void
GdFreeImage(int id)
{
	PIMAGEITEM	pItem;
	PMWIMAGEHDR	pimage;

	pItem = findimage(id);
	if (pItem) {
		GdListRemove(&imagehead, &pItem->link);
		pimage = pItem->pimage;

		/* delete image bits*/
		if(pimage->imagebits)
			free(pimage->imagebits);
		if(pimage->palette)
			free(pimage->palette);

		free(pimage);
		GdItemFree(pItem);
	}
}

MWBOOL
GdGetImageInfo(int id, PMWIMAGEINFO pii)
{
	PMWIMAGEHDR	pimage;
	PIMAGEITEM	pItem;
	int		i;

	pItem = findimage(id);
	if (!pItem) {
		memset(pii, 0, sizeof(*pii));
		return FALSE;
	}
	pimage = pItem->pimage;
	pii->id = id;
	pii->width = pimage->width;
	pii->height = pimage->height;
	pii->planes = pimage->planes;
	pii->bpp = pimage->bpp;
	pii->pitch = pimage->pitch;
	pii->bytesperpixel = pimage->bytesperpixel;
	pii->compression = pimage->compression;
	pii->palsize = pimage->palsize;
	if (pimage->palsize) {
		if (pimage->palette) {
			for (i=0; i<pimage->palsize; ++i)
				pii->palette[i] = pimage->palette[i];
		} else {
			/* FIXME handle jpeg's without palette*/
			GdGetPalette(pItem->psd, 0, pimage->palsize,
				pii->palette);
		}
	}
	return TRUE;
}

#define PIX2BYTES(n)	(((n)+7)/8)
/*
 * compute image line size and bytes per pixel
 * from bits per pixel and width
 */
void
ComputePitch(int bpp, int width, int *pitch, int *bytesperpixel)
{
	int	linesize;
	int	bytespp = 1;

	if(bpp == 1)
		linesize = PIX2BYTES(width);
	else if(bpp <= 4)
		linesize = PIX2BYTES(width<<2);
	else if(bpp <= 8)
		linesize = width;
	else if(bpp <= 16) {
		linesize = width * 2;
		bytespp = 2;
	} else if(bpp <= 24) {
		linesize = width * 3;
		bytespp = 3;
	} else {
		linesize = width * 4;
		bytespp = 4;
	}

	/* rows are DWORD right aligned*/
	*pitch = (linesize + 3) & ~3;
	*bytesperpixel = bytespp;
}

/*
 * StretchImage - Resize an image
 *
 * Major portions from SDL Simple DirectMedia Layer by Sam Lantinga
 * Copyright (C) 1997, 1998, 1999, 2000  Sam Lantinga <slouken@devolution.com>
 * This a stretch blit implementation based on ideas given to me by
 *  Tomasz Cejner - thanks! :)
 */
/*
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define DEFINE_COPY_ROW(name, type)					\
static void name(type *src, int src_w, type *dst, int dst_w)		\
{									\
	int i;								\
	int pos, inc;							\
	type pixel = 0;							\
									\
	pos = 0x10000;							\
	inc = (src_w << 16) / dst_w;					\
	for ( i=dst_w; i>0; --i ) {					\
		while ( pos >= 0x10000L ) {				\
			pixel = *src++;					\
			pos -= 0x10000L;				\
		}							\
		*dst++ = pixel;						\
		pos += inc;						\
	}								\
}

DEFINE_COPY_ROW(copy_row1, unsigned char)
DEFINE_COPY_ROW(copy_row2, unsigned short)
DEFINE_COPY_ROW(copy_row4, unsigned long)

static void copy_row3(unsigned char *src, int src_w, unsigned char *dst,
	int dst_w)
{
	int i;
	int pos, inc;
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;

	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for ( i=dst_w; i>0; --i ) {
		while ( pos >= 0x10000L ) {
			b = *src++;
			g = *src++;
			r = *src++;
			pos -= 0x10000L;
		}
		*dst++ = b;
		*dst++ = g;
		*dst++ = r;
		pos += inc;
	}
}

/* Perform a stretch blit between two image structs of the same format.*/
void
GdStretchImage(PMWIMAGEHDR src, MWCLIPRECT *srcrect, PMWIMAGEHDR dst,
	MWCLIPRECT *dstrect)
{
	int pos, inc;
	int bytesperpixel;
	int dst_maxrow;
	int src_row, dst_row;
	MWUCHAR *srcp = 0;
	MWUCHAR *dstp;
	MWCLIPRECT full_src;
	MWCLIPRECT full_dst;

	if ( src->bytesperpixel != dst->bytesperpixel ) {
		EPRINTF("GdStretchImage: bytesperpixel mismatch\n");
		return;
	}

	/* Verify the blit rectangles */
	if ( srcrect ) {
		if ( (srcrect->x < 0) || (srcrect->y < 0) ||
		     ((srcrect->x+srcrect->width) > src->width) ||
		     ((srcrect->y+srcrect->height) > src->height) ) {
			EPRINTF("GdStretchImage: invalid source rect\n");
			return;
		}
	} else {
		full_src.x = 0;
		full_src.y = 0;
		full_src.width = src->width;
		full_src.height = src->height;
		srcrect = &full_src;
	}
	if ( dstrect ) {
		/* if stretching to nothing, return*/
		if (!dstrect->width || !dstrect->height)
			return;
		if ( (dstrect->x < 0) || (dstrect->y < 0) ||
		     ((dstrect->x+dstrect->width) > dst->width) ||
		     ((dstrect->y+dstrect->height) > dst->height) ) {
			EPRINTF("GdStretchImage: invalid dest rect\n");
			return;
		}
	} else {
		full_dst.x = 0;
		full_dst.y = 0;
		full_dst.width = dst->width;
		full_dst.height = dst->height;
		dstrect = &full_dst;
	}

	/* Set up the data... */
	pos = 0x10000;
	inc = (srcrect->height << 16) / dstrect->height;
	src_row = srcrect->y;
	dst_row = dstrect->y;
	bytesperpixel = dst->bytesperpixel;

	/* Perform the stretch blit */
	for ( dst_maxrow = dst_row+dstrect->height; dst_row<dst_maxrow;
								++dst_row ) {
		dstp = (MWUCHAR *)dst->imagebits + (dst_row*dst->pitch)
				    + (dstrect->x*bytesperpixel);
		while ( pos >= 0x10000L ) {
			srcp = (MWUCHAR *)src->imagebits + (src_row*src->pitch)
				    + (srcrect->x*bytesperpixel);
			++src_row;
			pos -= 0x10000L;
		}

		switch (bytesperpixel) {
		case 1:
			copy_row1(srcp, srcrect->width, dstp, dstrect->width);
			break;
		case 2:
			copy_row2((unsigned short *)srcp, srcrect->width,
				(unsigned short *)dstp, dstrect->width);
			break;
		case 3:
			copy_row3(srcp, srcrect->width, dstp, dstrect->width);
			break;
		case 4:
			copy_row4((unsigned long *)srcp, srcrect->width,
				(unsigned long *)dstp, dstrect->width);
			break;
		}

		pos += inc;
	}
}

// by yuanlii@29bbs.net, 1/31/2005 13:35:06
//#if defined(HAVE_FILEIO) && defined(HAVE_JPEG_SUPPORT)
#if defined(HAVE_JPEG_SUPPORT)
// by yuanlii@29bbs.net
#include "jpeglib.h"
/*
 * JPEG decompression routine
 *
 * JPEG support must be enabled (see README.txt in contrib/jpeg)
 *
 * SOME FINE POINTS: (from libjpeg)
 * In the below code, we ignored the return value of jpeg_read_scanlines,
 * which is the number of scanlines actually read.  We could get away with
 * this because we asked for only one line at a time and we weren't using
 * a suspending data source.  See libjpeg.doc for more info.
 *
 * We cheated a bit by calling alloc_sarray() after jpeg_start_decompress();
 * we should have done it beforehand to ensure that the space would be
 * counted against the JPEG max_memory setting.  In some systems the above
 * code would risk an out-of-memory error.  However, in general we don't
 * know the output image dimensions before jpeg_start_decompress(), unless we
 * call jpeg_calc_output_dimensions().  See libjpeg.doc for more about this.
 *
 * Scanlines are returned in the same order as they appear in the JPEG file,
 * which is standardly top-to-bottom.  If you must emit data bottom-to-top,
 * you can use one of the virtual arrays provided by the JPEG memory manager
 * to invert the data.  See wrbmp.c for an example.
 *
 * As with compression, some operating modes may require temporary files.
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.  See libjpeg.doc.
 */
static int
LoadJPEG(buffer_t *src, PMWIMAGEHDR pimage, PSD psd, MWBOOL fast_grayscale)
{
  int 	i;
  int	ret = 2;	/* image load error*/
  unsigned char magic[4];

#if FASTJPEG
  extern MWPALENTRY mwstdpal8[256];
#else
  MWPALENTRY palette[256];
#endif

  struct jpeg_source_mgr smgr;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  
  static void init_source(j_compress_ptr dinfo) {
    smgr.next_input_byte = src->start;
    smgr.bytes_in_buffer = src->size;
  }

  static void fill_input_buffer(j_compress_ptr dinfo) {
    return;
  }

  static void skip_input_data(j_compress_ptr dinfo, long num_bytes) {
    if (num_bytes >= src->size) return;
    smgr.next_input_byte += num_bytes;
    smgr.bytes_in_buffer -= num_bytes;
  }

  static boolean resync_to_restart(j_decompress_ptr dinfo, int desired) {
    return(jpeg_resync_to_restart(dinfo, desired));
  }

  static void term_source(j_compress_ptr dinfo) {
    return;
  }
	      
  /* first determine if JPEG file since decoder will error if not*/
  bseek(src, 0, SEEK_SET);
  
  if (!bread(src, magic, 2)) 
    return(0);
  
  if (magic[0] != 0xFF || magic[1] != 0xD8) 
    return(0);		/* not JPEG image*/
  
  
  bread(src, magic, 4);
  bread(src, magic, 4);
  
  if (strncmp(magic, "JFIF", 4) != 0) 
    return(0);		/* not JPEG image*/
  
  bread(src, 0, SEEK_SET);
  pimage->imagebits = NULL;
  pimage->palette = NULL;
  
  /* Step 1: allocate and initialize JPEG decompression object */
  
  /* We set up the normal JPEG error routines. */
  cinfo.err = jpeg_std_error (&jerr);
  
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress (&cinfo);
     
  
  /* Step 2:  Setup the source manager */

  smgr.init_source = (void *)init_source;
  smgr.fill_input_buffer = (void *)fill_input_buffer;
  smgr.skip_input_data = (void *)skip_input_data;
  smgr.resync_to_restart = (void *)resync_to_restart;
  smgr.term_source = (void *)term_source;
  
  cinfo.src = &smgr;

  /* Step 2: specify data source (eg, a file) */
  /* jpeg_stdio_src (&cinfo, fp); */
  
  /* Step 3: read file parameters with jpeg_read_header() */
  jpeg_read_header (&cinfo, TRUE);

	/* Step 4: set parameters for decompression */
//	cinfo.out_color_space = fast_grayscale? JCS_GRAYSCALE: JCS_RGB;
  if (gr_toosd) {
	  cinfo.out_color_space = fast_grayscale? JCS_GRAYSCALE: JCS_RGB;
	  cinfo.quantize_colors = FALSE;

#if FASTJPEG
	  goto fastjpeg;
#endif
	  if (!fast_grayscale)
	  {
		  if (psd->pixtype == MWPF_PALETTE)
		  {
fastjpeg:
			  cinfo.quantize_colors = TRUE;

#if FASTJPEG
			  cinfo.actual_number_of_colors = 256;
#else
			  /* Get system palette */
			  cinfo.actual_number_of_colors = 
				  GdGetPalette(psd, 0, psd->ncolors, palette);
#endif

			  /* Allocate jpeg colormap space */
			  cinfo.colormap = (*cinfo.mem->alloc_sarray)
				  ((j_common_ptr) &cinfo, JPOOL_IMAGE,
				   (JDIMENSION)cinfo.actual_number_of_colors,
				   (JDIMENSION)3);

			  /* Set colormap from system palette */
			  for(i = 0; i < cinfo.actual_number_of_colors; ++i)
			  {
#if FASTJPEG
				  cinfo.colormap[0][i] = mwstdpal8[i].r;
				  cinfo.colormap[1][i] = mwstdpal8[i].g;
				  cinfo.colormap[2][i] = mwstdpal8[i].b;
#else
				  cinfo.colormap[0][i] = palette[i].r;
				  cinfo.colormap[1][i] = palette[i].g;
				  cinfo.colormap[2][i] = palette[i].b;
#endif
			  }
		  }
	  }
	  else 
	  {
		  /* Grayscale output asked */
		  cinfo.quantize_colors = TRUE;
		  cinfo.out_color_space = JCS_GRAYSCALE;
		  cinfo.desired_number_of_colors = psd->ncolors;
	  }
  } else {
/* by yuanlii@29bbs.net, 05/09/2005 10:22:29 */
	  cinfo.out_color_space = JCS_YCbCr;
//	  cinfo.out_color_space = JCS_RGB;
/* by yuanlii@29bbs.net */
  }

	jpeg_calc_output_dimensions(&cinfo);

	pimage->width = cinfo.output_width;
	pimage->height = cinfo.output_height;
	pimage->planes = 1;
	if (gr_toosd) {
#if FASTJPEG
		pimage->bpp = 8;
#else
		pimage->bpp = (fast_grayscale || psd->pixtype == MWPF_PALETTE)?
			8: cinfo.output_components*8;
#endif
	} else {
		pimage->bpp = cinfo.output_components*8;
	}

	ComputePitch(pimage->bpp, pimage->width, &pimage->pitch,
		&pimage->bytesperpixel);
	pimage->compression = MWIMAGE_RGB;	/* RGB not BGR order*/
	if (gr_toosd) {
		pimage->palsize = (pimage->bpp == 8)? 256: 0;
	} else {
		pimage->palsize = 257;		/* display to YUV */
	}
	pimage->imagebits = malloc(pimage->pitch * pimage->height);
	if(!pimage->imagebits)
		goto err;
	pimage->palette = NULL;
#if FASTJPEG
	if(pimage->bpp == 8) {
		pimage->palette = malloc(256*sizeof(MWPALENTRY));
		if(!pimage->palette)
			goto err;
		for (i=0; i<256; ++i)
			pimage->palette[i] = mwstdpal8[i];
	}
#endif

	/* Step 5: Start decompressor */
	jpeg_start_decompress (&cinfo);

	/* Step 6: while (scan lines remain to be read) */
	while(cinfo.output_scanline < cinfo.output_height) {
		JSAMPROW rowptr[1];
		rowptr[0] = (JSAMPROW)(pimage->imagebits +
			cinfo.output_scanline * pimage->pitch);
		jpeg_read_scanlines (&cinfo, rowptr, 1);
	}
	ret = 1;

err:
	/* Step 7: Finish decompression */
	jpeg_finish_decompress (&cinfo);

	/* Step 8: Release JPEG decompression object */
	jpeg_destroy_decompress (&cinfo);

	/* May want to check to see whether any corrupt-data
	 * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	 */
#ifdef	__DEBUG
	fprintf(stderr, "JPEG image load ok!\n");
#endif//__DEBUG
	return ret;
}
#endif /* defined(HAVE_FILEIO) && defined(HAVE_JPEG_SUPPORT)*/

// by yuanlii@29bbs.net, 1/31/2005 13:34:19
//#if defined(HAVE_FILEIO) && defined(HAVE_GIF_SUPPORT)
#if defined(HAVE_GIF_SUPPORT)
// by yuanlii@29bbs.net
/* Code for GIF decoding has been adapted from XPaint:                   */
/* +-------------------------------------------------------------------+ */
/* | Copyright 1990, 1991, 1993 David Koblas.			       | */
/* | Copyright 1996 Torsten Martinsen.				       | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.	       | */
/* +-------------------------------------------------------------------+ */
/* Portions Copyright (C) 1999  Sam Lantinga*/
/* Adapted for use in SDL by Sam Lantinga -- 7/20/98 */
/*
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/* GIF stuff*/
/*
 * GIF decoding routine
 */
#define	MAXCOLORMAPSIZE		256
#define	MAX_LWZ_BITS		12
#define INTERLACE		0x40
#define LOCALCOLORMAP		0x80

#define CM_RED		0
#define CM_GREEN	1
#define CM_BLUE		2

#define BitSet(byte, bit)	(((byte) & (bit)) == (bit))
#define	ReadOK(src,buffer,len)	bread(src, buffer, len)
#define LM_to_uint(a,b)		(((b)<<8)|(a))

struct {
    unsigned int Width;
    unsigned int Height;
    unsigned char ColorMap[3][MAXCOLORMAPSIZE];
    unsigned int BitPixel;
    unsigned int ColorResolution;
    unsigned int Background;
    unsigned int AspectRatio;
    int GrayScale;
} GifScreen;

static struct {
    int transparent;
    int delayTime;
    int inputFlag;
    int disposal;
} Gif89;

static int ReadColorMap(buffer_t *src, int number,
		unsigned char buffer[3][MAXCOLORMAPSIZE], int *flag);
static int DoExtension(buffer_t *src, int label);
static int GetDataBlock(buffer_t *src, unsigned char *buf);
static int GetCode(buffer_t *src, int code_size, int flag);
static int LWZReadByte(buffer_t *src, int flag, int input_code_size);
static int ReadImage(buffer_t *src, PMWIMAGEHDR pimage, int len, int height, int,
		unsigned char cmap[3][MAXCOLORMAPSIZE],
		int gray, int interlace, int ignore);

static int
LoadGIF(buffer_t *src, PMWIMAGEHDR pimage)
{
    unsigned char buf[16];
    unsigned char c;
    unsigned char localColorMap[3][MAXCOLORMAPSIZE];
    int grayScale;
    int useGlobalColormap;
    int bitPixel;
    int imageCount = 0;
    char version[4];
    int imageNumber = 1;
    int ok = 0;

    bseek(src, 0, SEEK_SET);

    pimage->imagebits = NULL;
    pimage->palette = NULL;

    if (!ReadOK(src, buf, 6))
        return 0;		/* not gif image*/
    if (strncmp((char *) buf, "GIF", 3) != 0)
        return 0;
    strncpy(version, (char *) buf + 3, 3);
    version[3] = '\0';

    if (strcmp(version, "87a") != 0 && strcmp(version, "89a") != 0) {
	EPRINTF("LoadGIF: GIF version number not 87a or 89a\n");
        return 2;		/* image loading error*/
    }
    Gif89.transparent = -1;
    Gif89.delayTime = -1;
    Gif89.inputFlag = -1;
    Gif89.disposal = 0;

    if (!ReadOK(src, buf, 7)) {
	EPRINTF("LoadGIF: bad screen descriptor\n");
        return 2;		/* image loading error*/
    }
    GifScreen.Width = LM_to_uint(buf[0], buf[1]);
    GifScreen.Height = LM_to_uint(buf[2], buf[3]);
    GifScreen.BitPixel = 2 << (buf[4] & 0x07);
    GifScreen.ColorResolution = (((buf[4] & 0x70) >> 3) + 1);
    GifScreen.Background = buf[5];
    GifScreen.AspectRatio = buf[6];

    if (BitSet(buf[4], LOCALCOLORMAP)) {	/* Global Colormap */
	if (ReadColorMap(src, GifScreen.BitPixel, GifScreen.ColorMap,
			 &GifScreen.GrayScale)) {
	    EPRINTF("LoadGIF: bad global colormap\n");
            return 2;		/* image loading error*/
	}
    }

    do {
	if (!ReadOK(src, &c, 1)) {
	    EPRINTF("LoadGIF: EOF on image data\n");
            goto done;
	}
	if (c == ';') {		/* GIF terminator */
	    if (imageCount < imageNumber) {
		EPRINTF("LoadGIF: no image %d of %d\n", imageNumber,imageCount);
                goto done;
	    }
	}
	if (c == '!') {		/* Extension */
	    if (!ReadOK(src, &c, 1)) {
		EPRINTF("LoadGIF: EOF on extension function code\n");
                goto done;
	    }
	    DoExtension(src, c);
	    continue;
	}
	if (c != ',') {		/* Not a valid start character */
	    continue;
	}
	++imageCount;

	if (!ReadOK(src, buf, 9)) {
	    EPRINTF("LoadGIF: bad image size\n");
            goto done;
	}
	useGlobalColormap = !BitSet(buf[8], LOCALCOLORMAP);

	bitPixel = 1 << ((buf[8] & 0x07) + 1);

	if (!useGlobalColormap) {
	    if (ReadColorMap(src, bitPixel, localColorMap, &grayScale)) {
		EPRINTF("LoadGIF: bad local colormap\n");
                goto done;
	    }
	    ok = ReadImage(src, pimage, LM_to_uint(buf[4], buf[5]),
			      LM_to_uint(buf[6], buf[7]),
			      bitPixel, localColorMap, grayScale,
			      BitSet(buf[8], INTERLACE),
			      imageCount != imageNumber);
	} else {
	    ok = ReadImage(src, pimage, LM_to_uint(buf[4], buf[5]),
			      LM_to_uint(buf[6], buf[7]),
			      GifScreen.BitPixel, GifScreen.ColorMap,
			      GifScreen.GrayScale, BitSet(buf[8], INTERLACE),
			      imageCount != imageNumber);
	}
    } while (ok == 0);

    /* set transparent color, if any*/
    pimage->transcolor = Gif89.transparent;

    if (ok)
#ifdef	__DEBUG
	    fprintf(stderr, "GIF image load ok!\n");
#endif//__DEBUG
	    return 1;		/* image load ok*/

done:
    if (pimage->imagebits)
	    free(pimage->imagebits);
    if (pimage->palette)
	    free(pimage->palette);
    return 2;			/* image load error*/
}

static int
ReadColorMap(buffer_t *src, int number, unsigned char buffer[3][MAXCOLORMAPSIZE],
    int *gray)
{
    int i;
    unsigned char rgb[3];
    int flag;

    flag = TRUE;

    for (i = 0; i < number; ++i) {
	if (!ReadOK(src, rgb, sizeof(rgb)))
	    return 1;
	buffer[CM_RED][i] = rgb[0];
	buffer[CM_GREEN][i] = rgb[1];
	buffer[CM_BLUE][i] = rgb[2];
	flag &= (rgb[0] == rgb[1] && rgb[1] == rgb[2]);
    }

#if 0
    if (flag)
	*gray = (number == 2) ? PBM_TYPE : PGM_TYPE;
    else
	*gray = PPM_TYPE;
#else
    *gray = 0;
#endif

    return FALSE;
}

static int
DoExtension(buffer_t *src, int label)
{
    static unsigned char buf[256];

    switch (label) {
    case 0x01:			/* Plain Text Extension */
	break;
    case 0xff:			/* Application Extension */
	break;
    case 0xfe:			/* Comment Extension */
	while (GetDataBlock(src, (unsigned char *) buf) != 0);
	return FALSE;
    case 0xf9:			/* Graphic Control Extension */
	GetDataBlock(src, (unsigned char *) buf);
	Gif89.disposal = (buf[0] >> 2) & 0x7;
	Gif89.inputFlag = (buf[0] >> 1) & 0x1;
	Gif89.delayTime = LM_to_uint(buf[1], buf[2]);
	if ((buf[0] & 0x1) != 0)
	    Gif89.transparent = buf[3];

	while (GetDataBlock(src, (unsigned char *) buf) != 0);
	return FALSE;
    default:
	break;
    }

    while (GetDataBlock(src, (unsigned char *) buf) != 0);

    return FALSE;
}

static int ZeroDataBlock = FALSE;

static int
GetDataBlock(buffer_t *src, unsigned char *buf)
{
    unsigned char count;

    if (!ReadOK(src, &count, 1))
	return -1;
    ZeroDataBlock = count == 0;

    if ((count != 0) && (!ReadOK(src, buf, count)))
	return -1;
    return count;
}

static int
GetCode(buffer_t *src, int code_size, int flag)
{
    static unsigned char buf[280];
    static int curbit, lastbit, done, last_byte;
    int i, j, ret;
    unsigned char count;

    if (flag) {
	curbit = 0;
	lastbit = 0;
	done = FALSE;
	return 0;
    }
    if ((curbit + code_size) >= lastbit) {
	if (done) {
	    if (curbit >= lastbit)
		EPRINTF("LoadGIF: bad decode\n");
	    return -1;
	}
	buf[0] = buf[last_byte - 2];
	buf[1] = buf[last_byte - 1];

	if ((count = GetDataBlock(src, &buf[2])) == 0)
	    done = TRUE;

	last_byte = 2 + count;
	curbit = (curbit - lastbit) + 16;
	lastbit = (2 + count) * 8;
    }
    ret = 0;
    for (i = curbit, j = 0; j < code_size; ++i, ++j)
	ret |= ((buf[i / 8] & (1 << (i % 8))) != 0) << j;

    curbit += code_size;

    return ret;
}

static int
LWZReadByte(buffer_t *src, int flag, int input_code_size)
{
    int code, incode;
    register int i;
    static int fresh = FALSE;
    static int code_size, set_code_size;
    static int max_code, max_code_size;
    static int firstcode, oldcode;
    static int clear_code, end_code;
    static int table[2][(1 << MAX_LWZ_BITS)];
    static int stack[(1 << (MAX_LWZ_BITS)) * 2], *sp;

    if (flag) {
	set_code_size = input_code_size;
	code_size = set_code_size + 1;
	clear_code = 1 << set_code_size;
	end_code = clear_code + 1;
	max_code_size = 2 * clear_code;
	max_code = clear_code + 2;

	GetCode(src, 0, TRUE);

	fresh = TRUE;

	for (i = 0; i < clear_code; ++i) {
	    table[0][i] = 0;
	    table[1][i] = i;
	}
	for (; i < (1 << MAX_LWZ_BITS); ++i)
	    table[0][i] = table[1][0] = 0;

	sp = stack;

	return 0;
    } else if (fresh) {
	fresh = FALSE;
	do {
	    firstcode = oldcode = GetCode(src, code_size, FALSE);
	} while (firstcode == clear_code);
	return firstcode;
    }
    if (sp > stack)
	return *--sp;

    while ((code = GetCode(src, code_size, FALSE)) >= 0) {
	if (code == clear_code) {
	    for (i = 0; i < clear_code; ++i) {
		table[0][i] = 0;
		table[1][i] = i;
	    }
	    for (; i < (1 << MAX_LWZ_BITS); ++i)
		table[0][i] = table[1][i] = 0;
	    code_size = set_code_size + 1;
	    max_code_size = 2 * clear_code;
	    max_code = clear_code + 2;
	    sp = stack;
	    firstcode = oldcode = GetCode(src, code_size, FALSE);
	    return firstcode;
	} else if (code == end_code) {
	    int count;
	    unsigned char buf[260];

	    if (ZeroDataBlock)
		return -2;

	    while ((count = GetDataBlock(src, buf)) > 0);

	    if (count != 0) {
		/*
		 * EPRINTF("missing EOD in data stream (common occurence)");
		 */
	    }
	    return -2;
	}
	incode = code;

	if (code >= max_code) {
	    *sp++ = firstcode;
	    code = oldcode;
	}
	while (code >= clear_code) {
	    *sp++ = table[1][code];
	    if (code == table[0][code])
		EPRINTF("LoadGIF: circular table entry\n");
	    code = table[0][code];
	}

	*sp++ = firstcode = table[1][code];

	if ((code = max_code) < (1 << MAX_LWZ_BITS)) {
	    table[0][code] = oldcode;
	    table[1][code] = firstcode;
	    ++max_code;
	    if ((max_code >= max_code_size) &&
		(max_code_size < (1 << MAX_LWZ_BITS))) {
		max_code_size *= 2;
		++code_size;
	    }
	}
	oldcode = incode;

	if (sp > stack)
	    return *--sp;
    }
    return code;
}

static int
ReadImage(buffer_t* src, PMWIMAGEHDR pimage, int len, int height, int cmapSize,
	  unsigned char cmap[3][MAXCOLORMAPSIZE],
	  int gray, int interlace, int ignore)
{
    unsigned char c;
    int i, v;
    int xpos = 0, ypos = 0, pass = 0;

    /*
     *	Initialize the compression routines
     */
    if (!ReadOK(src, &c, 1)) {
	EPRINTF("LoadGIF: EOF on image data\n");
	return 0;
    }
    if (LWZReadByte(src, TRUE, c) < 0) {
	EPRINTF("LoadGIF: error reading image\n");
	return 0;
    }

    /*
     *	If this is an "uninteresting picture" ignore it.
     */
    if (ignore) {
	while (LWZReadByte(src, FALSE, c) >= 0);
	return 0;
    }
    /*image = ImageNewCmap(len, height, cmapSize);*/
    pimage->width = len;
    pimage->height = height;
    pimage->planes = 1;
    pimage->bpp = 8;
    ComputePitch(8, len, &pimage->pitch, &pimage->bytesperpixel);
    pimage->compression = 0;
    pimage->palsize = cmapSize;
    pimage->palette = malloc(256*sizeof(MWPALENTRY));
    pimage->imagebits = malloc(height*pimage->pitch);
    if(!pimage->imagebits || !pimage->palette)
	    return 0;

    for (i = 0; i < cmapSize; i++) {
	/*ImageSetCmap(image, i, cmap[CM_RED][i],
		     cmap[CM_GREEN][i], cmap[CM_BLUE][i]);*/
	pimage->palette[i].r = cmap[CM_RED][i];
	pimage->palette[i].g = cmap[CM_GREEN][i];
	pimage->palette[i].b = cmap[CM_BLUE][i];
    }

    while ((v = LWZReadByte(src, FALSE, c)) >= 0) {
	pimage->imagebits[ypos * pimage->pitch + xpos] = v;

	++xpos;
	if (xpos == len) {
	    xpos = 0;
	    if (interlace) {
		switch (pass) {
		case 0:
		case 1:
		    ypos += 8;
		    break;
		case 2:
		    ypos += 4;
		    break;
		case 3:
		    ypos += 2;
		    break;
		}

		if (ypos >= height) {
		    ++pass;
		    switch (pass) {
		    case 1:
			ypos = 4;
			break;
		    case 2:
			ypos = 2;
			break;
		    case 3:
			ypos = 1;
			break;
		    default:
			goto fini;
		    }
		}
	    } else {
		++ypos;
	    }
	}
	if (ypos >= height)
	    break;
    }

fini:
    return 1;
}
#endif /* defined(HAVE_FILEIO) && defined(HAVE_GIF_SUPPORT)*/

// by yuanlii@29bbs.net, 1/31/2005 13:17:46
//#endif /* defined(HAVE_FILEIO)*/
// by yuanlii@29bbs.net

#if defined(HAVE_BMP_SUPPORT)
/* BMP stuff*/
#define BI_RGB		0L
#define BI_RLE8		1L
#define BI_RLE4		2L
#define BI_BITFIELDS	3L

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef long		LONG;

typedef struct {
	/* BITMAPFILEHEADER*/
	BYTE	bfType[2];
	DWORD	bfSize;
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits;
} BMPFILEHEAD;

#define FILEHEADSIZE 14

/* windows style*/
typedef struct {
	/* BITMAPINFOHEADER*/
	DWORD	BiSize;
	DWORD	BiWidth;
	DWORD	BiHeight;
	WORD	BiPlanes;
	WORD	BiBitCount;
	DWORD	BiCompression;
	DWORD	BiSizeImage;
	DWORD	BiXpelsPerMeter;
	DWORD	BiYpelsPerMeter;
	DWORD	BiClrUsed;
	DWORD	BiClrImportant;
} BMPINFOHEAD;

#define INFOHEADSIZE 40

/* os/2 style*/
typedef struct {
	/* BITMAPCOREHEADER*/
	DWORD	bcSize;
	WORD	bcWidth;
	WORD	bcHeight;
	WORD	bcPlanes;
	WORD	bcBitCount;
} BMPCOREHEAD;

#define COREHEADSIZE 12

static int	DecodeRLE8(MWUCHAR *buf, buffer_t *src);
static int	DecodeRLE4(MWUCHAR *buf, buffer_t *src);
static void	put4(int b);

/*
 * BMP decoding routine
 */

/* Changed by JHC to allow a buffer instead of a filename */

static int
LoadBMP(buffer_t *src, PMWIMAGEHDR pimage)
{
	int		h, i, compression;
	int		headsize;
	MWUCHAR		*imagebits;
	BMPFILEHEAD	bmpf;
	BMPINFOHEAD	bmpi;
	BMPCOREHEAD	bmpc;
	MWUCHAR 	headbuffer[INFOHEADSIZE];

	bseek(src, 0, SEEK_SET);

	pimage->imagebits = NULL;
	pimage->palette = NULL;

	/* read BMP file header*/
	if (bread(src, &headbuffer, FILEHEADSIZE) != FILEHEADSIZE)
	  return(0);

	bmpf.bfType[0] = headbuffer[0];
	bmpf.bfType[1] = headbuffer[1];

	/* Is it really a bmp file ? */
	if (*(WORD*)&bmpf.bfType[0] != wswap(0x4D42)) /* 'BM' */
		return 0;	/* not bmp image*/

	/*bmpf.bfSize = dwswap(dwread(&headbuffer[2]));*/
	bmpf.bfOffBits = dwswap(dwread(&headbuffer[10]));

	/* Read remaining header size */
	if (bread(src,&headsize,sizeof(DWORD)) != sizeof(DWORD))
		return 0;	/* not bmp image*/
	headsize = dwswap(headsize);

	/* might be windows or os/2 header */
	if(headsize == COREHEADSIZE) {

		/* read os/2 header */
		if(bread(src, &headbuffer, COREHEADSIZE-sizeof(DWORD)) !=
			COREHEADSIZE-sizeof(DWORD))
				return 0;	/* not bmp image*/

		/* Get data */
		bmpc.bcWidth = wswap(*(WORD*)&headbuffer[0]);
		bmpc.bcHeight = wswap(*(WORD*)&headbuffer[2]);
		bmpc.bcPlanes = wswap(*(WORD*)&headbuffer[4]);
		bmpc.bcBitCount = wswap(*(WORD*)&headbuffer[6]);
		
		pimage->width = (int)bmpc.bcWidth;
		pimage->height = (int)bmpc.bcHeight;
		pimage->bpp = bmpc.bcBitCount;
		if (pimage->bpp <= 8)
			pimage->palsize = 1 << pimage->bpp;
		else pimage->palsize = 0;
		compression = BI_RGB;
	} else {
		/* read windows header */
		if(bread(src, &headbuffer, INFOHEADSIZE-sizeof(DWORD)) !=
			INFOHEADSIZE-sizeof(DWORD))
				return 0;	/* not bmp image*/

		/* Get data */
		bmpi.BiWidth = dwswap(*(DWORD*)&headbuffer[0]);
		bmpi.BiHeight = dwswap(*(DWORD*)&headbuffer[4]);
		bmpi.BiPlanes = wswap(*(WORD*)&headbuffer[8]);
		bmpi.BiBitCount = wswap(*(WORD*)&headbuffer[10]);
		bmpi.BiCompression = dwswap(*(DWORD*)&headbuffer[12]);
		bmpi.BiSizeImage = dwswap(*(DWORD*)&headbuffer[16]);
		bmpi.BiXpelsPerMeter = dwswap(*(DWORD*)&headbuffer[20]);
		bmpi.BiYpelsPerMeter = dwswap(*(DWORD*)&headbuffer[24]);
		bmpi.BiClrUsed = dwswap(*(DWORD*)&headbuffer[28]);
		bmpi.BiClrImportant = dwswap(*(DWORD*)&headbuffer[32]);

		pimage->width = (int)bmpi.BiWidth;
		pimage->height = (int)bmpi.BiHeight;
		pimage->bpp = bmpi.BiBitCount;
		pimage->palsize = (int)bmpi.BiClrUsed;
		if (pimage->palsize > 256)
			pimage->palsize = 0;
		else if(pimage->palsize == 0 && pimage->bpp <= 8)
			pimage->palsize = 1 << pimage->bpp;
		compression = bmpi.BiCompression;
	}
	pimage->compression = MWIMAGE_BGR;	/* right side up, BGR order*/
	pimage->planes = 1;

	/* currently only 1, 4, 8 and 24 bpp bitmaps*/
	if(pimage->bpp > 8 && pimage->bpp != 24) {
		EPRINTF("LoadBMP: image bpp not 1, 4, 8 or 24\n");
		return 2;	/* image loading error*/
	}

	/* compute byte line size and bytes per pixel*/
	ComputePitch(pimage->bpp, pimage->width, &pimage->pitch,
		&pimage->bytesperpixel);

	/* Allocate image */
	if( (pimage->imagebits = malloc(pimage->pitch*pimage->height)) == NULL)
		goto err;
	if( (pimage->palette = malloc(256*sizeof(MWPALENTRY))) == NULL)
		goto err;

	/* get colormap*/
	if(pimage->bpp <= 8) {
		for(i=0; i<pimage->palsize; i++) {
			pimage->palette[i].b = bgetc(src);
			pimage->palette[i].g = bgetc(src);
			pimage->palette[i].r = bgetc(src);
			if(headsize != COREHEADSIZE)
				bgetc(src);
		}
	}

	/* decode image data*/
	bseek(src, bmpf.bfOffBits, SEEK_SET);

	h = pimage->height;
	/* For every row ... */
	while (--h >= 0) {
		/* turn image rightside up*/
		imagebits = pimage->imagebits + h*pimage->pitch;

		/* Get row data from file */
		if(compression == BI_RLE8) {
			if(!DecodeRLE8(imagebits, src))
				break;
		} else if(compression == BI_RLE4) {
			if(!DecodeRLE4(imagebits, src))
				break;
		} else {
			if(bread(src, imagebits, pimage->pitch) !=
				pimage->pitch)
					goto err;
		}
	}
	return 1;		/* bmp image ok*/
	
err:
	EPRINTF("LoadBMP: image loading error\n");
	if(pimage->imagebits)
		free(pimage->imagebits);
	if(pimage->palette)
		free(pimage->palette);
	return 2;		/* bmp image error*/
}

/*
 * Decode one line of RLE8, return 0 when done with all bitmap data
 */
static int
DecodeRLE8(MWUCHAR *buf, buffer_t *src)
{
	int		c, n;
	MWUCHAR *	p = buf;

	for( ;;) {
	  switch( n = bgetc(src)) {
	  case EOF:
	    return( 0);
	  case 0:			/* 0 = escape*/
	    switch( n = bgetc(src)) {
	    case 0: 	/* 0 0 = end of current scan line*/
	      return( 1);
	    case 1:		/* 0 1 = end of data*/
	      return( 1);
	    case 2:		/* 0 2 xx yy delta mode NOT SUPPORTED*/
	      (void)bgetc(src);
	      (void)bgetc(src);
	      continue;
	    default:	/* 0 3..255 xx nn uncompressed data*/
	      for( c=0; c<n; c++)
		*p++ = bgetc(src);
	      if( n & 1)
		(void)bgetc(src);
	      continue;
	    }
	  default:
	    c = bgetc(src);
	    while( n--)
	      *p++ = c;
	    continue;
	  }
	}
}

/*
 * Decode one line of RLE4, return 0 when done with all bitmap data
 */
static MWUCHAR *p;
static int	once;

static void
put4(int b)
{
	static int	last;

	last = (last << 4) | b;
	if( ++once == 2) {
		*p++ = last;
		once = 0;
	}
}
	
static int
DecodeRLE4(MWUCHAR *buf, buffer_t *src)
{
	int		c, n, c1, c2;

	p = buf;
	once = 0;
	c1 = 0;

	for( ;;) {
	  switch( n = bgetc(src)) {
	  case EOF:
	    return( 0);
	  case 0:			/* 0 = escape*/
	    switch( n = bgetc(src)) {
	    case 0: 	/* 0 0 = end of current scan line*/
	      if( once)
		put4( 0);
	      return( 1);
	    case 1:		/* 0 1 = end of data*/
	      if( once)
		put4( 0);
	      return( 1);
	    case 2:		/* 0 2 xx yy delta mode NOT SUPPORTED*/
	      (void)bgetc(src);
	      (void)bgetc(src);
	      continue;
	    default:	/* 0 3..255 xx nn uncompressed data*/
	      c2 = (n+3) & ~3;
	      for( c=0; c<c2; c++) {
		if( (c & 1) == 0)
		  c1 = bgetc(src);
		if( c < n)
		  put4( (c1 >> 4) & 0x0f);
		c1 <<= 4;
	      }
	      continue;
	    }
	  default:
	    c = bgetc(src);
	    c1 = (c >> 4) & 0x0f;
	    c2 = c & 0x0f;
	    for( c=0; c<n; c++)
	      put4( (c&1)? c2: c1);
	    continue;
	  }
	}
}
#endif /* defined(HAVE_BMP_SUPPORT) */
#if defined(HAVE_STB_SUPPORT)
static int
LoadSTB(buffer_t *src, PMWIMAGEHDR pimage)
{
	MWBOOL	bCompressed = FALSE;
	int	nPaletteNo;
	MWUCHAR	headbuffer[8];

	if (bread(src, &headbuffer, 4) != 4)
		return 0;
	/* read the signature of the file */
	if (strncpy((char *)headbuffer, "TSTB", 4) != 0 && 
			strncpy((char *)headbuffer, "CSTB", 4) != 0) {
		return 0;
	}
	/* check whether compressed */
	if (headbuffer[0] == 'C')
		bCompressed = TRUE;

	if (bread(src, &headbuffer, 4) != 4)
		return 0;
	/* ignore the Version now */
	nPaletteNo = headbuffer[1];

	/* read the width and height from the file */
	if (bread(src, &headbuffer, 8) != 8)
		return 0;
	pimage->width = LM_to_uint(headbuffer[0], headbuffer[1]);
	pimage->height = LM_to_uint(headbuffer[2], headbuffer[3]);
	pimage->planes = 1;
	pimage->bpp = 8;
	pimage->pitch = pimage->width;
	pimage->bytesperpixel = 1;
	pimage->compression = 0;
	pimage->palsize = 0;
	pimage->palette = NULL;
	pimage->imagebits = malloc(pimage->height * pimage->pitch
		       	* sizeof(MWUCHAR));
	if (NULL == pimage->imagebits) {
		fprintf(stderr, "Alloc mem for STB file error\n");
		return 0;
	}

	if (bCompressed) {
		/* read other pixel data that compressed */
		int	err;
		int	nLen;
		uLong	uncomprLen, compressLen;
		MWUCHAR	*imagebytes;

		nLen = pimage->height * pimage->pitch;
		compressLen = bsize(src) - bpos(src);
		imagebytes = malloc(compressLen * sizeof(MWUCHAR));
		if (NULL == imagebytes) {
			fprintf(stderr, "Alloc mem for CSTB file error\n");
			return 0;
		}
		if (bread(src, &imagebytes, compressLen) != compressLen) {
			fprintf(stderr, "Read data error in CSTB file\n");
			free(imagebytes);
			return 0;
		}

		/* uncompress it from imagebytes to pimage->imagebits */
		err = uncompress(pimage->imagebits, &uncomprLen,
			       	imagebytes, compressLen);
		if ((err != Z_OK) || (uncomprLen != nLen)) {
			fprintf(stderr, "uncompress error: %d\n", err);
			free(imagebytes);
			return 0;
		}

		free(imagebytes);
	} else {
		/* read other uncompressed pixel data */
		int	nLen;
		nLen = bsize(src) - bpos(src);
		if (nLen != pimage->height * pimage->pitch) {
			fprintf(stderr, "Wrong file length in TSTB file\n");
			return 0;
		}
		if (bread(src, pimage->imagebits, nLen) != nLen) {
			fprintf(stderr, "Read data error in TSTB file\n");
			return 0;
		}
	}

	return 1;
}
#endif /* defined(HAVE_STB_SUPPORT) */
