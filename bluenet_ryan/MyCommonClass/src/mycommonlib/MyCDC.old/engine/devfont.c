/*
 * Copyright (c) 2000 Greg Haerr <greg@censoft.com>
 * T1lib Adobe type1 routines contributed by Vidar Hokstad
 * Freetype TrueType routines contributed by Martin Jolicoeur
 * Han Zi Ku routines contributed by Tanghao and Jauming
 *
 * Device-independent font and text drawing routines
 *
 * These routines do the necessary range checking, clipping, and cursor
 * overwriting checks, and then call the lower level device dependent
 * routines to actually do the drawing.  The lower level routines are
 * only called when it is known that all the pixels to be drawn are
 * within the device area and are visible.
 */
/*#define NDEBUG*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include <zlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <device.h>
#define strcmpi	strcasecmp

#if HAVE_HZK_SUPPORT
/*
 * 16x16 and 24x24 ascii and chinese fonts
 * Big5 and GB2312 encodings supported
 */
#define MAX_PATH	256
typedef struct {
	int	width;
	int	height;
	int	size;
	unsigned long use_count;
	char *	pFont;
	char	file[MAX_PATH + 1];
} HZKFONT;

static int use_big5=1;
static HZKFONT CFont[2];	/* font cache*/
static HZKFONT AFont[2];	/* font cache*/

/* jmt: moved inside MWHZKFONT*/
static int afont_width = 8;
static int cfont_width = 16;
static int font_height = 16;
static char *afont_address;
static char *cfont_address;

typedef struct {
	PMWFONTPROCS	fontprocs;	/* common hdr*/
	MWCOORD		fontsize;
	int		fontrotation;
	int		fontattr;		

	HZKFONT 	CFont;		/* hzkfont stuff */
	HZKFONT 	AFont;
	int 		afont_width;
	int 		cfont_width;
	int 		font_height;
	char 		*afont_address;
	char 		*cfont_address;
} MWHZKFONT, *PMWHZKFONT;

static int  hzk_init(PSD psd);
static PMWHZKFONT hzk_createfont(const char *name, MWCOORD height,int fontattr);
static MWBOOL hzk_getfontinfo(PMWFONT pfont, PMWFONTINFO pfontinfo);
static void hzk_gettextsize(PMWFONT pfont, const void *text,
		int cc, MWCOORD *pwidth, MWCOORD *pheight, MWCOORD *pbase);
#if 0
static void hzk_gettextbits(PMWFONT pfont, int ch, IMAGEBITS *retmap,
		MWCOORD *pwidth, MWCOORD *pheight, MWCOORD *pbase);
static void hzk_setfontrotation(PMWFONT pfont, int tenthdegrees);
#endif
static void hzk_destroyfont(PMWFONT pfont);
static void hzk_drawtext(PMWFONT pfont, PSD psd, MWCOORD x, MWCOORD y,
		const void *text, int cc, int flags);
#if	HAVE_SHADOW_SUPPORT
static void hzk_drawtext_s(PMWFONT pfont, PSD psd, MWCOORD ax, MWCOORD ay,
		char *text, int cc);
#endif/*HAVE_SHADOW_SUPPORT*/
static void hzk_setfontsize(PMWFONT pfont, MWCOORD fontsize);
		
/* handling routines for MWHZKFONT*/
static MWFONTPROCS hzk_procs = {
	MWTF_ASCII,			/* routines expect ASCII*/
	hzk_getfontinfo,
	hzk_gettextsize,
	NULL,				/* hzk_gettextbits*/
	hzk_destroyfont,
	hzk_drawtext,
#if	HAVE_SHADOW_SUPPORT
	hzk_drawtext_s,			/* draw text with border */
#endif/*HAVE_SHADOW_SUPPORT*/
	hzk_setfontsize,
	NULL, 				/* setfontrotation*/
	NULL,				/* setfontattr*/
};

static int
UC16_to_GB(const unsigned char *uc16, int cc, unsigned char *ascii);
#endif /* HAVE_HZK_SUPPORT*/

PMWFONT	gr_pfont;            	/* current font*/

/* temp extern decls*/
extern MWPIXELVAL gr_foreground;
extern MWPIXELVAL gr_background;
#if	HZK24_SMOOTH
extern MWPALENTRY gr_palette[256];
#endif/*HZK24_SMOOTH*/
extern MWBOOL gr_usebg;

static int  utf8_to_utf16(const unsigned char *utf8, int cc,
		unsigned short *unicode16);

/*
 * Set the font for future calls.
 */
PMWFONT
GdSetFont(PMWFONT pfont)
{
	PMWFONT	oldfont = gr_pfont;

	gr_pfont = pfont;
	return oldfont;
}

/*
 * Select a font, based on various parameters.
 * If plogfont is specified, name and height parms are ignored
 * and instead used from MWLOGFONT.
 * 
 * If height is 0, return builtin font from passed name.
 * Otherwise find builtin font best match based on height.
 */
PMWFONT
GdCreateFont(PSD psd, const char *name, MWCOORD height,
	const PMWLOGFONT plogfont)
{
	int 		i;
	int		fontht;
	int		fontno;
 	int		fontclass;
	int		fontattr = 0;
	PMWFONT		pfont;
	PMWCOREFONT	pf = psd->builtin_fonts;
	MWFONTINFO	fontinfo;
	MWSCREENINFO 	scrinfo;
 	char		fontname[128];

	GdGetScreenInfo(psd, &scrinfo);

	/* if plogfont not specified, use name and height*/
	if (!plogfont) {
		if (!name)
			name = MWFONT_SYSTEM_VAR;
		strcpy(fontname, name);
		fontclass = MWLF_CLASS_ANY;
	} else {
		if (!name)
			name = MWFONT_SYSTEM_VAR;
		strcpy(fontname, name);
		fontclass = MWLF_CLASS_ANY;
		height = plogfont->lfHeight;
		if (plogfont->lfUnderline)
			fontattr = MWTF_UNDERLINE;
	}
	height = abs(height);
 
 	if (!fontclass)
 		goto first;
 
	/* use builtin screen fonts, FONT_xxx, if height is 0 */
 	if (height == 0 || fontclass == MWLF_CLASS_ANY ||
	    fontclass == MWLF_CLASS_BUILTIN) {
  		for(i = 0; i < scrinfo.fonts; ++i) {
 			if(!strcmpi(pf[i].name, fontname)) {
  				pf[i].fontsize = pf[i].cfont->height;
				pf[i].fontattr = fontattr;
  				return (PMWFONT)&pf[i];
  			}
  		}
 
		/* return first builtin font*/
		if (height == 0 || fontclass == MWLF_CLASS_BUILTIN)
			goto first;
  	}

#if HAVE_HZK_SUPPORT
        /* Make sure the library is initialized */
	if (hzk_init(psd)) {
		pfont = (PMWFONT)hzk_createfont(name, height, fontattr);
		if(pfont)		
			return pfont;
		fprintf(stderr, "hzk_createfont: %s not found\n", name);
	}
#endif

	/* find builtin font closest in height*/
	if(height != 0) {
		fontno = 0;
		height = abs(height);
		fontht = MAX_MWCOORD;
		for(i = 0; i < scrinfo.fonts; ++i) {
			pfont = (PMWFONT)&pf[i];
			GdGetFontInfo(pfont, &fontinfo);
			if(fontht > abs(height-fontinfo.height)) { 
				fontno = i;
				fontht = abs(height-fontinfo.height);
			}
		}
		pf[fontno].fontsize = pf[fontno].cfont->height;
		pf[fontno].fontattr = fontattr;
		return (PMWFONT)&pf[fontno];
	}

first:
	/* Return first builtin font*/
	pf->fontsize = pf->cfont->height;
	pf->fontattr = fontattr;
	return (PMWFONT)&pf[0];
}

/* Set the font size for the passed font*/
MWCOORD
GdSetFontSize(PMWFONT pfont, MWCOORD fontsize)
{
	MWCOORD	oldfontsize = pfont->fontsize;

	pfont->fontsize = fontsize;

	if (pfont->fontprocs->SetFontSize)
	    pfont->fontprocs->SetFontSize(pfont, fontsize);

	return oldfontsize;
}

/* Set the font rotation angle in tenths of degrees for the passed font*/
/* by yuanlii@29bbs.net, 07/16/2005 10:05:43 */
/*
int
GdSetFontRotation(PMWFONT pfont, int tenthdegrees)
{
	MWCOORD	oldrotation = pfont->fontrotation;

	pfont->fontrotation = tenthdegrees;

	if (pfont->fontprocs->SetFontRotation)
	    pfont->fontprocs->SetFontRotation(pfont, tenthdegrees);
	
	return oldrotation;
}
*/
/* by yuanlii@29bbs.net */

/*
 * Set/reset font attributes (MWTF_KERNING, MWTF_ANTIALIAS)
 * for the passed font.
 */
int
GdSetFontAttr(PMWFONT pfont, int setflags, int clrflags)
{
	MWCOORD	oldattr = pfont->fontattr;

	pfont->fontattr &= ~clrflags;
	pfont->fontattr |= setflags;

	if (pfont->fontprocs->SetFontAttr)
	    pfont->fontprocs->SetFontAttr(pfont, setflags, clrflags);
	
	return oldattr;
}

/* Unload and deallocate font*/
void
GdDestroyFont(PMWFONT pfont)
{
	if (pfont->fontprocs->DestroyFont) {
		pfont->fontprocs->DestroyFont(pfont);
	}
}

/* Return information about a specified font*/
MWBOOL
GdGetFontInfo(PMWFONT pfont, PMWFONTINFO pfontinfo)
{
	if(!pfont || !pfont->fontprocs->GetFontInfo)
		return FALSE;

	return pfont->fontprocs->GetFontInfo(pfont, pfontinfo);
}

/*
 * Convert from one encoding to another
 * Input cc and returned cc is character count, not bytes
 * Return < 0 on error or can't translate
 */
int
GdConvertEncoding(const void *istr, int iflags, int cc, void *ostr, int oflags)
{
	const unsigned char 	*istr8;
	const unsigned short 	*istr16;
	const unsigned long	*istr32;
	unsigned char 		*ostr8;
	unsigned short 		*ostr16;
	unsigned long		*ostr32;
	unsigned int		ch;
	int			icc;
	unsigned short		buf16[512];

	iflags &= MWTF_PACKMASK;
	oflags &= MWTF_PACKMASK;

	/* allow -1 for len with ascii*/
	if(cc == -1 && (iflags == MWTF_ASCII))
		cc = strlen((char *)istr);

	/* first check for utf8 input encoding*/
	if(iflags == MWTF_UTF8) {
		/* we've only got uc16 now so convert to uc16...*/
		cc = utf8_to_utf16((unsigned char *)istr, cc,
			oflags==MWTF_UC16?(unsigned short*) ostr: buf16);

		if(oflags == MWTF_UC16 || cc < 0)
			return cc;

		/* will decode again to requested format (probably ascii)*/
		iflags = MWTF_UC16;
		istr = buf16;
	}

#if HAVE_HZK_SUPPORT
	if(iflags == MWTF_UC16 && oflags == MWTF_ASCII) {
		/* only support uc16 convert to ascii now...*/
		cc = UC16_to_GB( istr, cc, ostr);
		return cc;
	}
#endif

	icc = cc;
	istr8 = istr;
	istr16 = istr;
	istr32 = istr;
	ostr8 = ostr;
	ostr16 = ostr;
	ostr32 = ostr;

	/* Convert between formats.  Note that there's no error
	 * checking here yet.
	 */
	while(--icc >= 0) {
		switch(iflags) {
		default:
			ch = *istr8++;
			break;
		case MWTF_UC16:
			ch = *istr16++;
			break;
		case MWTF_UC32:
			ch = *istr32++;
		}
		switch(oflags) {
		default:
			*ostr8++ = (unsigned char)ch;
			break;
		case MWTF_UC16:
			*ostr16++ = (unsigned short)ch;
			break;
		case MWTF_UC32:
			*ostr32++ = ch;
		}
	}
	return cc;
}

/* Get the width and height of passed text string in the passed font*/
void
GdGetTextSize(PMWFONT pfont, const void *str, int cc, MWCOORD *pwidth,
	MWCOORD *pheight, MWCOORD *pbase, int flags)
{
	const void *	text;
	unsigned long	buf[256];
	int		defencoding = pfont->fontprocs->encoding;

	/* convert encoding if required*/
	if((flags & MWTF_PACKMASK) != defencoding) {
		cc = GdConvertEncoding(str, flags, cc, buf, defencoding);
		flags &= ~MWTF_PACKMASK;
		flags |= defencoding;
		text = buf;
	} else text = str;

	if(cc == -1 && (flags & MWTF_PACKMASK) == MWTF_ASCII)
		cc = strlen((char *)str);

	if(cc <= 0 || !pfont->fontprocs->GetTextSize) {
		*pwidth = *pheight = *pbase = 0;
		return;
	}

	/* calc height and width of string*/
	pfont->fontprocs->GetTextSize(pfont, text, cc, pwidth, pheight, pbase);
}

/* Draw a text string at a specifed coordinates in the foreground color
 * (and possibly the background color), applying clipping if necessary.
 * The background color is only drawn if the gr_usebg flag is set.
 * Use the current font.
 */
void
GdText(PSD psd, MWCOORD x, MWCOORD y, const void *str, int cc, int flags)
{
	const void *	text;
	unsigned long	buf[256];
	int		defencoding = gr_pfont->fontprocs->encoding;

	/* convert encoding if required*/
	if((flags & MWTF_PACKMASK) != defencoding) {
		cc = GdConvertEncoding(str, flags, cc, buf, defencoding);
		flags &= ~MWTF_PACKMASK;
		flags |= defencoding;
		text = buf;
	} else text = str;

	if(cc == -1 && (flags & MWTF_PACKMASK) == MWTF_ASCII)
		cc = strlen((char *)str);

	if(cc <= 0 || !gr_pfont->fontprocs->DrawText)
		return;

	/* draw text string*/
	gr_pfont->fontprocs->DrawText(gr_pfont, psd, x, y, text, cc, flags);
}

#if	HAVE_SHADOW_SUPPORT
void
GdText_S(PSD psd, MWCOORD x, MWCOORD y, char *szString, int nCount)
{
	if(nCount == -1)
		nCount = strlen(szString);

	if(nCount <= 0 || !gr_pfont->fontprocs->DrawText_S)
		return;

	/* draw text string*/
	gr_pfont->fontprocs->DrawText_S(gr_pfont, psd, x, y, szString, nCount);
}
#endif/*HAVE_SHADOW_SUPPORT*/

/*
 * Draw ascii text using COREFONT type font.
 */
void
corefont_drawtext(PMWFONT pfont, PSD psd, MWCOORD x, MWCOORD y,
	const void *text, int cc, int flags)
{
	const unsigned char *str = text;
	MWCOORD		width;			/* width of text area */
	MWCOORD 	height;			/* height of text area */
	MWCOORD		base;			/* baseline of text*/
	MWCOORD		startx, starty;
						/* bitmap for characters */
	MWIMAGEBITS bitmap[MAX_CHAR_HEIGHT*MAX_CHAR_WIDTH/MWIMAGE_BITSPERIMAGE];

	pfont->fontprocs->GetTextSize(pfont, str, cc, &width, &height, &base);
	
	if(flags & MWTF_BASELINE)
		y -= base;
	else if(flags & MWTF_BOTTOM)
		y -= (height - 1);
	startx = x;
	starty = y + base;

	switch (GdClipArea(psd, x, y, x + width - 1, y + height - 1)) {
	case CLIP_VISIBLE:
		/*
		 * For size considerations, there's no low-level text
		 * draw, so we've got to draw all text
		 * with per-point clipping for the time being
		if (gr_usebg)
			psd->FillRect(psd, x, y, x + width - 1, y + height - 1,
				gr_background);
		psd->DrawText(psd, x, y, str, cc, gr_foreground, pfont);
		GdFixCursor(psd);
		return;
		*/
		break;

	case CLIP_INVISIBLE:
		return;
	}

	/* Get the bitmap for each character individually, and then display
	 * them using clipping for each one.
	 */
	while (--cc >= 0 && x < psd->xvirtres) {
		unsigned int ch = *str++;

		pfont->fontprocs->GetTextBits(pfont, ch, bitmap, &width,
			&height, &base);

		/* note: change to bitmap*/
		GdBitmap(psd, x, y, width, height, bitmap);
		x += width;
	}

	if (pfont->fontattr & MWTF_UNDERLINE)
		GdLine(psd, startx, starty, x, starty, FALSE);

// by yuanlii@29bbs.net, 1/29/2005 17:06:11
//	GdFixCursor(psd);
//by yuanlii@29bbs.net
}

/* UTF-8 to UTF-16 conversion.  Surrogates are handeled properly, e.g.
 * a single 4-byte UTF-8 character is encoded into a surrogate pair.
 * On the other hand, if the UTF-8 string contains surrogate values, this
 * is considered an error and returned as such.
 *
 * The destination array must be able to hold as many Unicode-16 characters
 * as there are ASCII characters in the UTF-8 string.  This in case all UTF-8
 * characters are ASCII characters.  No more will be needed.
 *
 * Copyright (c) 2000 Morten Rolland, Screen Media
 */
static int
utf8_to_utf16(const unsigned char *utf8, int cc, unsigned short *unicode16)
{
	int count = 0;
	unsigned char c0, c1;
	unsigned long scalar;

	while(--cc >= 0) {
		c0 = *utf8++;
		/*DPRINTF("Trying: %02x\n",c0);*/

		if ( c0 < 0x80 ) {
			/* Plain ASCII character, simple translation :-) */
			*unicode16++ = c0;
			count++;
			continue;
		}

		if ( (c0 & 0xc0) == 0x80 )
			/* Illegal; starts with 10xxxxxx */
			return -1;

		/* c0 must be 11xxxxxx if we get here => at least 2 bytes */
		scalar = c0;
		if(--cc < 0)
			return -1;
		c1 = *utf8++;
		/*DPRINTF("c1=%02x\n",c1);*/
		if ( (c1 & 0xc0) != 0x80 )
			/* Bad byte */
			return -1;
		scalar <<= 6;
		scalar |= (c1 & 0x3f);

		if ( !(c0 & 0x20) ) {
			/* Two bytes UTF-8 */
			if ( scalar < 0x80 )
				return -1;	/* Overlong encoding */
			*unicode16++ = scalar & 0x7ff;
			count++;
			continue;
		}

		/* c0 must be 111xxxxx if we get here => at least 3 bytes */
		if(--cc < 0)
			return -1;
		c1 = *utf8++;
		/*DPRINTF("c1=%02x\n",c1);*/
		if ( (c1 & 0xc0) != 0x80 )
			/* Bad byte */
			return -1;
		scalar <<= 6;
		scalar |= (c1 & 0x3f);

		if ( !(c0 & 0x10) ) {
			/*DPRINTF("####\n");*/
			/* Three bytes UTF-8 */
			if ( scalar < 0x800 )
				return -1;	/* Overlong encoding */
			if ( scalar >= 0xd800 && scalar < 0xe000 )
				return -1;	/* UTF-16 high/low halfs */
			*unicode16++ = scalar & 0xffff;
			count++;
			continue;
		}

		/* c0 must be 1111xxxx if we get here => at least 4 bytes */
		c1 = *utf8++;
		if(--cc < 0)
			return -1;
		/*DPRINTF("c1=%02x\n",c1);*/
		if ( (c1 & 0xc0) != 0x80 )
			/* Bad byte */
			return -1;
		scalar <<= 6;
		scalar |= (c1 & 0x3f);

		if ( !(c0 & 0x08) ) {
			/* Four bytes UTF-8, needs encoding as surrogates */
			if ( scalar < 0x10000 )
				return -1;	/* Overlong encoding */
			scalar -= 0x10000;
			*unicode16++ = ((scalar >> 10) & 0x3ff) + 0xd800;
			*unicode16++ = (scalar & 0x3ff) + 0xdc00;
			count += 2;
			continue;
		}

		return -1;	/* No support for more than four byte UTF-8 */
	}
	return count;
}

#if HAVE_HZK_SUPPORT

/* UniCode-16 (MWTF_UC16) to GB(MWTF_ASCII) Chinese Characters conversion.
 * a single 2-byte UC16 character is encoded into a surrogate pair.
 * return -1 ,if error;
 * The destination array must be able to hold as many
 * as there are Unicode-16 characters.
 *
 * Copyright (c) 2000 Tang Hao (TownHall)(tang_hao@263.net).
 */
static int
UC16_to_GB(const unsigned char *uc16, int cc, unsigned char *ascii)
{
	FILE* fp;
	char buffer[256];
	unsigned char *uc16p;
	int i=0,j=0, k;
	unsigned char *filebuffer;
	unsigned short *uc16pp,*table;
	unsigned short uc16px;
	int length=31504;

	if (use_big5)
		length=54840;

    	uc16p=(unsigned char *) uc16;
	uc16pp=(unsigned short *) uc16;

	strcpy(buffer,HZK_FONT_DIR);
	if (use_big5)
    		strcat(buffer,"/BG2UBG.KU");
	else
    		strcat(buffer,"/UGB2GB.KU");
	if(!(fp = fopen(buffer, "rb"))) 
	{
   	  	 fprintf (stderr, "Error.\nThe %s file can not be found!\n",buffer);
   		 return -1;
    	}

	filebuffer= (unsigned char *)malloc ( length);

	if(fread(filebuffer, sizeof(char),length, fp) < length) {
	   	  fprintf (stderr, "Error in reading ugb2gb.ku file!\n");
	   	  fclose(fp);
 	     	  return -1;
	}
    	fclose(fp);

	if (use_big5)
	{
		table=(unsigned short *)filebuffer;
		while(1)
		{
			if(j>=cc)
			{
				ascii[i]=0;
				break;
			}
			uc16px=*uc16pp;
			if((uc16px)<=0x00ff)
			{
				ascii[i]=(char)(*uc16pp);
				i++;
			}
			else
			{
				ascii[i]=0xa1; ascii[i+1]=0x40;
				for (k=0; k<13710; k++)
				{
					if (*(table+(k*2+1))==(uc16px))
					{
						ascii[i]=(char)((*(table+(k*2)) & 0xff00) >> 8);
						ascii[i+1]=(char)(*(table+(k*2)) & 0x00ff);
						break;
					}
				}
				i+=2;
			}
			uc16pp++; j++;
		}
	}
	else
	{
	while(1)
	{
		if(j>=cc)
		{
			ascii[i]=0;
			break;
		}
		if((*((uc16p)+j)==0)&&(*((uc16p)+j+1)==0))
		{
			ascii[i]=0;
			break;
		}
		else
		{
			if(*((uc16p)+j+1)==0)
			{
				ascii[i]=*((uc16p)+j);
				i++;
				j+=2;
			}
			else
			{
			/* to find the place of unicode charater*/
            		{
				int p1=0,p2=length-4,p;
				unsigned int c1,c2,c,d;
				c1=((unsigned int )filebuffer[p1])*0x100+(filebuffer[p1+1]);
                		c2=((unsigned int )filebuffer[p2])*0x100+(filebuffer[p2+1]);
				d=((unsigned int )*((uc16p)+j))*0x100+*((uc16p)+j+1);
                		if(c1==d)
				{
					ascii[i]=filebuffer[p1+2];
					ascii[i+1]=filebuffer[p1+3];
					goto findit;
 	            		}
                		if(c2==d)
				{
					ascii[i]=filebuffer[p2+2];
					ascii[i+1]=filebuffer[p2+3];
					goto findit;
                		}
				while(1)
				{
					p=(((p2-p1)/2+p1)>>2)<<2;
					c=((unsigned int )filebuffer[p])*0x100+(filebuffer[p+1]);
					if(d==c)	/* find it*/
					{
						ascii[i]=filebuffer[p+2];
						ascii[i+1]=filebuffer[p+3];
						break;
          	   	   		}
					else if(p2<=p1+4)	/* can't find.*/
					{
						ascii[i]='.';	/* ((uc16p)+j);*/
						ascii[i+1]='.';	/* ((uc16p)+j+1);*/
						break;
					}
					else if(d<c)
					{
						p2=p;
						c2=c;										
                  			}
					else
					{
						p1=p;
						c1=c;														
					}
				}
	            	}
			findit:
  			i+=2;
			j+=2;
			}
		}
	}
	}
	free(filebuffer);

	return i;
}

/************************** functions definition ******************************/

static int hzk_id( PMWHZKFONT pf )
{
	switch(pf->font_height)
	{
	case 24:
		return 0;
	case 16: default:
		return 1;
	}
}

/* This function get Chinese font info from etc file.*/
static MWBOOL GetCFontInfo( PMWHZKFONT pf )
{
	int charset;

	if (use_big5)
		charset=(13094+408);
	else
		charset=8178;

    	CFont[hzk_id(pf)].width = pf->cfont_width;
    	pf->CFont.width = pf->cfont_width;

    	CFont[hzk_id(pf)].height = pf->font_height;
    	pf->CFont.height = pf->font_height;

#if	HZK24_SMOOTH
	if (24 == pf->cfont_width) {
		/* bi-char, 2 bits per pixel */
		CFont[hzk_id(pf)].size = ((pf->CFont.width + 7) / 8) *
			pf->CFont.height * charset * 2;
		pf->CFont.size = ((pf->CFont.width + 7) / 8) *
			pf->CFont.height * charset * 2;
	} else {
		CFont[hzk_id(pf)].size = ((pf->CFont.width + 7) / 8) *
			pf->CFont.height * charset;
		pf->CFont.size = ((pf->CFont.width + 7) / 8) *
		       	pf->CFont.height * charset;
	}
#else/*!HZK24_SMOOTH*/
    	CFont[hzk_id(pf)].size = ((pf->CFont.width + 7) / 8) *
		pf->CFont.height * charset;
    	pf->CFont.size = ((pf->CFont.width + 7) / 8) * pf->CFont.height * charset;
#endif/*HZK24_SMOOTH*/

    	if(pf->CFont.size < charset * 8)
        	return FALSE;

	strcpy(CFont[hzk_id(pf)].file,HZK_FONT_DIR);
	strcpy(pf->CFont.file,HZK_FONT_DIR);

	if(pf->font_height==16) {
		strcat(CFont[hzk_id(pf)].file,"/hzk16.gz");
		strcat(pf->CFont.file,"/hzk16.gz");
	} else {
#if	HZK24_SMOOTH
		strcat(CFont[hzk_id(pf)].file,"/hzk24b.gz");
		strcat(pf->CFont.file,"/hzk24b.gz");
#else/*!HZK24_SMOOTH*/
		strcat(CFont[hzk_id(pf)].file,"/hzk24.gz");
		strcat(pf->CFont.file,"/hzk24.gz");
#endif/*HZK24_SMOOTH*/
	}

    	if (use_big5)
    	{
		CFont[hzk_id(pf)].file[strlen(pf->CFont.file)-3]+=use_big5;
		pf->CFont.file[strlen(pf->CFont.file)-3]+=use_big5;
    	}

    	return TRUE;
}

/* This function get ASCII font info from etc file.*/
static MWBOOL GetAFontInfo( PMWHZKFONT pf )
{
    	AFont[hzk_id(pf)].width = pf->afont_width;
    	pf->AFont.width = pf->afont_width;

    	AFont[hzk_id(pf)].height = pf->font_height;
    	pf->AFont.height = pf->font_height;

    	AFont[hzk_id(pf)].size = ((pf->AFont.width + 7) / 8) *
		pf->AFont.height * 255;
    	pf->AFont.size = ((pf->AFont.width + 7) / 8) * pf->AFont.height * 255;
    
	if(pf->AFont.size < 255 * 8)
        	return FALSE;

	strcpy(AFont[hzk_id(pf)].file,HZK_FONT_DIR);
	strcpy(pf->AFont.file,HZK_FONT_DIR);
	
	if(pf->font_height==16) {
	    	strcat(AFont[hzk_id(pf)].file,"/asc16.gz");
	    	strcat(pf->AFont.file,"/asc16.gz");
	} else {
	    	strcat(AFont[hzk_id(pf)].file,"/asc24.gz");
	    	strcat(pf->AFont.file,"/asc24.gz");
	}
    	return TRUE;
}

/* This function load system font into memory.*/
static MWBOOL LoadFont( PMWHZKFONT pf )
{
	gzFile	file;
	int	size[2];
	uLong	uncomprLen;

	if(!GetCFontInfo(pf)) {
		fprintf (stderr, "Get Chinese HZK font info failure!\n");
		return FALSE;
	}
    	if(CFont[hzk_id(pf)].pFont == NULL) {	/* check font cache*/
		/* Open font file and read information to the system memory.*/
		fprintf (stderr, "Loading Chinese HZK font from file(%s)..." ,pf->CFont.file );

		file = gzopen(CFont[hzk_id(pf)].file, "rb");
		if (file == NULL) {
			fprintf(stderr, "gzopen error\n");
			return FALSE;
		}

		gzread(file, &size[0], 4);
		gzread(file, &size[1], 4);
		if (size[0] == size[1]) {
			uncomprLen = size[0];
		} else {
			fprintf(stderr, "undecompress Chinese HZK file error\n");
			gzclose(file);
			return FALSE;
		}

 	   	/* Allocate system memory for Chinese font.*/
		size[1] = (uncomprLen > pf->CFont.size) ? uncomprLen : pf->CFont.size;
 		if( !(CFont[hzk_id(pf)].pFont = (char *)malloc(size[1])) )
 		{
	 	       	fprintf (stderr, "Allocate memory for Chinese HZK font failure.\n");
			gzclose(file);
		        return FALSE;
	 	}
 	
		size[0] = gzread(file, CFont[hzk_id(pf)].pFont, (unsigned)uncomprLen);
		if (uncomprLen != size[0]) {
	      	  	fprintf (stderr, "Error in reading Chinese HZK font file!\n");
			gzclose(file);
			return FALSE;
		}

		gzclose(file);
		CFont[hzk_id(pf)].use_count=0;

		fprintf (stderr, "done.\n" );
	}
	cfont_address = CFont[hzk_id(pf)].pFont;
	pf->cfont_address = CFont[hzk_id(pf)].pFont;
	pf->CFont.pFont = CFont[hzk_id(pf)].pFont;

	CFont[hzk_id(pf)].use_count++;

	if(!GetAFontInfo(pf))
	{
	       fprintf (stderr, "Get ASCII HZK font info failure!\n");
	       return FALSE;
	}
    	if(AFont[hzk_id(pf)].pFont == NULL)	/* check font cache*/
	{
		/* Open font file and read information to the system memory.*/
		fprintf (stderr, "Loading ASCII HZK font from file(%s)..." ,pf->AFont.file );

		file = gzopen(AFont[hzk_id(pf)].file, "rb");
		if (file == NULL) {
			fprintf(stderr, "gzopen error\n");
			return FALSE;
		}

		gzread(file, &size[0], 4);
		gzread(file, &size[1], 4);
		if (size[0] == size[1]) {
			uncomprLen = size[0];
		} else {
			fprintf(stderr, "undecompress HZKfile error\n");
			gzclose(file);
			return FALSE;
		}
		
 		/* Allocate system memory for ASCII font.*/
 		if( !(AFont[hzk_id(pf)].pFont = (char *)malloc(pf->AFont.size)) )
 		{
 		       	fprintf (stderr, "Allocate memory for ASCII HZK font failure.\n");
 		       	free(CFont[hzk_id(pf)].pFont);
 		       	CFont[hzk_id(pf)].pFont = NULL;
			gzclose(file);
			return FALSE;
 		}
 	
		size[0] = gzread(file, AFont[hzk_id(pf)].pFont, (unsigned)uncomprLen);
		if (uncomprLen != size[0]) {
	      	  	fprintf (stderr, "Error in reading Chinese HZK font file!\n");
			gzclose(file);
			return FALSE;
		}

		gzclose(file);
		AFont[hzk_id(pf)].use_count=0;
 	
		fprintf (stderr, "done.\n" );

  	}
	afont_address = AFont[hzk_id(pf)].pFont;
	pf->afont_address = AFont[hzk_id(pf)].pFont;
	pf->AFont.pFont = AFont[hzk_id(pf)].pFont;

	AFont[hzk_id(pf)].use_count++;

  	return TRUE;
}

/* This function unload system font from memory.*/
static void UnloadFont( PMWHZKFONT pf )
{
	CFont[hzk_id(pf)].use_count--;
	AFont[hzk_id(pf)].use_count--;

	if (!CFont[hzk_id(pf)].use_count) {	
		fprintf(stderr, "Unloading hzk%d...", CFont[hzk_id(pf)].height);
		free(pf->CFont.pFont);
		free(pf->AFont.pFont);
		CFont[hzk_id(pf)].pFont = NULL;
		AFont[hzk_id(pf)].pFont = NULL;

		fprintf(stderr, "done.\n");
	}
}

static int
hzk_init(PSD psd)
{
	/* FIXME: *.KU file should be opened and
	 * read in here...*/
	return 1;
}

static PMWHZKFONT
hzk_createfont(const char *name, MWCOORD height, int attr)
{
	PMWHZKFONT	pf;

	if(strcmp(name,"HZKFONT")!=0 && strcmp(name,"HZXFONT")!=0)
		return FALSE;

	/*printf("hzk_createfont(%s,%d)\n",name,height);*/

	use_big5=name[2]-'K';

	/* allocate font structure*/
	pf = (PMWHZKFONT)calloc(sizeof(MWHZKFONT), 1);
	if (!pf)
		return NULL;
	pf->fontprocs = &hzk_procs;

	pf->fontsize=height;
#if 0
	GdSetFontSize((PMWFONT)pf, height);
#endif
/* by yuanlii@29bbs.net, 07/16/2005 10:06:02 */
/*	GdSetFontRotation((PMWFONT)pf, 0);	*/
/* by yuanlii@29bbs.net */
	GdSetFontAttr((PMWFONT)pf, attr, 0);

	if(height==24) {		
		afont_width = 12;
		cfont_width = 24;
		font_height = 24;

		pf->afont_width = 12;
		pf->cfont_width = 24;
		pf->font_height = 24;
	} else {		
		afont_width = 8;
		cfont_width = 16;
		font_height = 16;

		pf->afont_width = 8;
		pf->cfont_width = 16;
		pf->font_height = 16;
	}

    	/* Load the font library to the system memory.*/
	if(!LoadFont(pf))
  	      	return FALSE;

	return pf;
}

int IsBig5(int i)
{
	if ((i>=0xa140 && i<=0xa3bf) || /* a140-a3bf(!a3e0) */
	    (i>=0xa440 && i<=0xc67e) || /* a440-c67e        */
	    (i>=0xc6a1 && i<=0xc8d3) || /* c6a1-c8d3(!c8fe) */
	    (i>=0xc940 && i<=0xf9fe))   /* c940-f9fe        */
		return 1;
	else
		return 0;
}

/*
 * following several function is used in hzk_drawtext
 */
static int getnextchar(char* s, unsigned char* cc)
{
    	if( s[0] == '\0') return 0;

    	cc[0] = (unsigned char)(*s);
    	cc[1] = (unsigned char)(*(s + 1));

    	if (use_big5) {
		if( IsBig5( (int) ( (cc[0] << 8) + cc[1]) ) )
			return 1;
	} else {
    		if( ((unsigned char)cc[0] > 0xa0) &&
		    ((unsigned char)cc[1] > 0xa0) )
        		return 1;
	}

	cc[1] = '\0';

    	return 1;
}

/***
 * @ingroup CDlgView
 * 	Get2Bits get 2 bits from a char buffer
 * @param pBuf the pointer to a byte of the char
 * @param nPos which 2 bits in the byte
 */
static int
Get2Bits(MWUCHAR *pBuf, int nPos)
{
	int nResult;
	int nBitInByte = (3 - nPos) * 2;

	assert((nPos >= 0) && (nPos <= 3));

	nResult = *pBuf >> nBitInByte;
	nResult &= 0x03;		/* only get the lowest 2 bits */
	assert((nResult >= 0) && (nResult <= 3));
	return nResult;
}

#if	HZK24_SMOOTH | HAVE_SHADOW_SUPPORT
static MWPIXELVAL
getdarkfg(MWPIXELVAL fg, MWPIXELVAL *bgbmp, int posi)
{
	int fgRed, fgGreen, fgBlue;		/* RGB from the foreground */
	int bgRed, bgGreen, bgBlue;		/* RGB from the badkground */
	int R,     G,       B;			/* RGB of light&dark */

	/* separate fgRGB&bgRGB from foreground and background */
	fgRed = gr_palette[fg].r;
	fgGreen = gr_palette[fg].g;
	fgBlue = gr_palette[fg].b;

	bgRed = gr_palette[bgbmp[posi]].r;
	bgGreen = gr_palette[bgbmp[posi]].g;
	bgBlue = gr_palette[bgbmp[posi]].b;

	/* get text_dark, (fg + bg)/2 */
	R = (fgRed + bgRed) >> 1;
	G = (fgGreen + bgGreen) >> 1;
	B = (fgBlue + bgGreen) >> 1;
	return GdFindColor(MWRGB(R, G, B));
}

static MWPIXELVAL
getlightfg(MWPIXELVAL fg, MWPIXELVAL *bgbmp, int posi)
{
	int fgRed, fgGreen, fgBlue;		/* RGB from the foreground */
	int bgRed, bgGreen, bgBlue;		/* RGB from the badkground */
	int R,     G,       B;			/* RGB of light&dark */

	/* separate fgRGB&bgRGB from foreground and background */
	fgRed = gr_palette[fg].r;
	fgGreen = gr_palette[fg].g;
	fgBlue = gr_palette[fg].b;

	bgRed = gr_palette[bgbmp[posi]].r;
	bgGreen = gr_palette[bgbmp[posi]].g;
	bgBlue = gr_palette[bgbmp[posi]].b;

	/* get text_light, fg/4 + bg*3/4 */
	R = ((fgRed + bgRed) >> 2) + (bgRed >> 1);
	G = ((fgGreen + bgGreen) >> 2) + (bgGreen >> 1);
	B = ((fgBlue + bgBlue) >> 2) + (bgBlue >> 1);
	return GdFindColor(MWRGB(R, G, B));
}
#endif/*HZK24_SMOOTH | HAVE_SHADOW_SUPPORT*/

#if	HZK24_SMOOTH
static void
expandcchar(PMWHZKFONT pf, int bg, int fg, unsigned char* c,
		MWPIXELVAL* bitmap, MWPIXELVAL* bgcbmp, MWULONG *pLUTable)
#else/*!HZK24_SMOOTH*/
static void
expandcchar(PMWHZKFONT pf, int bg, int fg, unsigned char* c,
	       	MWPIXELVAL* bitmap)
#endif
{
	int posi = 0;    /* the position in bitmap */
    	int c1, c2, seq;
	int x,y;
    	unsigned char *font;
    	int b = 0;		/* keep gcc happy with b = 0 - MW */
	int pitch;
	MWULONG	*bitmap32;

   	c1 = c[0];
    	c2 = c[1];
	if (use_big5) {
		seq=0;
		/* ladd=loby-(if(loby<127)?64:98)*/
		c2-=(c2<127?64:98);   

		/* hadd=(hiby-164)*157*/
		if (c1>=0xa4)	/* standard font*/
		{
			seq=(((c1-164)*157)+c2);
			if (seq>=5809) seq-=408;
		}

		/* hadd=(hiby-161)*157*/
		if (c1<=0xa3)	/* special font*/
			seq=(((c1-161)*157)+c2)+13094;
	} else
	       	seq=((c1 - 161)*94 + c2 - 161); 

#if	HZK24_SMOOTH
	if (24 == pf->font_height)
		font = pf->cfont_address + (seq * 2 *
			  (pf->font_height * ((pf->cfont_width + 7) / 8)));
	else
		font = pf->cfont_address + (seq *
			  (pf->font_height * ((pf->cfont_width + 7) / 8)));
#else/*!HZK24_SMOOTH*/
	font = pf->cfont_address + (seq *
		  (pf->font_height * ((pf->cfont_width + 7) / 8)));
#endif/*HZK24_SMOOTH*/

#if	HZK24_SMOOTH
	if (pf->font_height != 24) {
#endif/*HZK24_SMOOTH*/
		/* don't need smooth, output it directly */
		for (y = 0; y < pf->font_height; y++)
			for (x = 0; x < pf->cfont_width; x++) 
			{
				if (0 == (x % 8))
					b = *font++;
				if (b & (128 >> (x % 8)))
					bitmap[posi++] = fg;
				else
					bitmap[posi++] = bg;
			}		
#if	HZK24_SMOOTH
		return;
	}

	/* must be hzk24 here */

	
	if (NULL == bgcbmp) {/* OPAQUE, gr_usebg is true, use pLUTable */
		bitmap32 = (MWULONG *)bitmap;
		pitch = (pf->cfont_width + 7) / 8 * 2;
		for (y = 0; y < pf->font_height; y++)
			for (x = 0; x < pitch; x++)
				*(bitmap32 + y * pitch + x) = pLUTable[(font[y * pitch + x])];
	} else {	    /* TRANSPARENT, use bgcbmp */
		/* smooth the bitmap, modify it in the original bitmap */
		for (y = 0; y < pf->font_height; y++)
			for (x = 0; x < pf->cfont_width; x++)  {
				posi = y * pf->cfont_width + x;
				switch (Get2Bits(font + posi / 4, posi % 4)) {
				case 0:
					bitmap[posi] = bgcbmp[posi];
					break;
				case 1:
					bitmap[posi] = getlightfg(fg, bgcbmp, posi);
					break;
				case 2:
					bitmap[posi] = getdarkfg(fg, bgcbmp, posi);
					break;
				case 3:
					bitmap[posi] = fg;
					break;
				default:
					fprintf(stderr, "Wrong Get2Bits value\n");
				}
			}
	}
#endif/*HZK24_SMOOTH*/
}

#if	HZK24_SMOOTH
static void
expandchar(PMWHZKFONT pf, int bg, int fg, int c,
	       	MWPIXELVAL* bitmap, MWPIXELVAL* bgbmp)
#else /*HZK24_SMOOTH*/
static void expandchar(PMWHZKFONT pf, int bg, int fg, int c, MWPIXELVAL* bitmap)
#endif/*HZK24_SMOOTH*/
{
	int i=0;
	int x,y;
    	unsigned char *font;
    	int b = 0;		/* keep gcc happy with b = 0 - MW */

    	font = pf->afont_address + c * (pf->font_height *
		((pf->afont_width + 7) / 8));

  	for (y = 0; y < pf->font_height; y++)
		for (x = 0; x < pf->afont_width; x++) 
		{
	    		if (x % 8 == 0)
				b = *font++;
	    		if (b & (128 >> (x % 8)))	/* pixel */
				bitmap[i++]=fg;
			else
#if	HZK24_SMOOTH
				if (NULL != bgbmp)
					bitmap[i] = bgbmp[i++];
				else
					bitmap[i++]=bg;
#else /*HZK24_SMOOTH*/
				bitmap[i++]=bg;
#endif/*HZK24_SMOOTH*/
  		}
}

/***
 * generate a look-up table
 */
void
generate_table(MWPIXELVAL gr_foreground, MWPIXELVAL gr_background, MWULONG *pLUTable)
{
	int	index;
	MWULONG	entry;
	MWUCHAR	color[4];
	int i;

	color[0] = gr_background;
	color[1] = getlightfg(gr_foreground, &gr_background, 0);
	color[2] = getdarkfg(gr_foreground, &gr_background, 0);
	color[3] = gr_foreground;

	for (index = 0; index <= 255; index++) {
		entry = 0;

		/* process the 1st pixel */
		i = index >> 6;
		entry |= color[i];
		/* process the 2st pixel */
		i = (index & 0x30) >> 4;
		entry |= color[i] << 8;
		/* process the 3rd pixel */
		i = (index & 0x0c) >> 2;
		entry |= color[i] << 16;
		/* process the 4th pixel */
		i = (index & 0x03);
		entry |= color[i] << 24;

		pLUTable[index] = entry;
	}
}

/*
 * Draw ASCII text string using HZK type font
 */
static void
hzk_drawtext(PMWFONT pfont, PSD psd, MWCOORD ax, MWCOORD ay,
	const void *text, int cc, int flags)
{
	PMWHZKFONT pf=(PMWHZKFONT)pfont;

    	unsigned char c[2];
    	unsigned char s1[3];
 	char *s,*sbegin;
	MWPIXELVAL *bitmap;
#if	HZK24_SMOOTH
	MWPIXELVAL *bgcbmp = NULL, *bgbmp = NULL;
	MWULONG    *pLUTable = NULL;		/* the lookup table */
#endif/*HZK24_SMOOTH*/

	s=(char *)text;

	if(cc==1)
	{
		s1[0]=*((unsigned char*)text);
		s1[1]=0x0;
		s1[2]=0x0;
		s=s1;
    	}

	sbegin=s;
    	bitmap = (MWPIXELVAL *)ALLOCA(pf->cfont_width * pf->font_height *
			sizeof(MWPIXELVAL));
	if (NULL == bitmap)
		return;

#if	HZK24_SMOOTH
	if (gr_usebg) {
		pLUTable = (MWULONG *)ALLOCA(256 * sizeof(MWULONG));
		if (NULL == pLUTable) {
			FREEA(bitmap);
			return;
		}
		generate_table(gr_foreground, gr_background, pLUTable);
	} else {
		bgcbmp = (MWPIXELVAL *)ALLOCA(pf->cfont_width * pf->font_height *
				sizeof(MWPIXELVAL));
		if (NULL == bgcbmp) {
			FREEA(bitmap);
			return; 
		}

		bgbmp = (MWPIXELVAL *)ALLOCA(pf->afont_width * pf->font_height * 
				sizeof(MWPIXELVAL));
		if (NULL == bgbmp) {
			FREEA(bgcbmp);
			FREEA(bitmap);
			return;
		}
	}
#endif/*HZK24_SMOOTH*/

    	while( getnextchar(s, c) )
	{
              	if( c[1] != '\0') 
		{
#if	HZK24_SMOOTH
			/* read the background to bgcbmp */
			if (!gr_usebg)
				psd->ReadBlock(psd, ax, ay, bgcbmp, pf->cfont_width, pf->font_height);
			expandcchar(pf, gr_background, gr_foreground, c, bitmap, bgcbmp, pLUTable);
#else/*!HZK24_SMOOTH*/
                	expandcchar(pf, gr_background, gr_foreground,
				       	c, bitmap);
#endif/*HZK24_SMOOTH*/
			/* Now draw the bitmap ... */
			
			if (flags&MWTF_TOP)
#if	SPEEDUP_DEVICE
				psd->DrawBlock(psd, ax, ay, bitmap, pf->cfont_width, pf->font_height);
#else/*!SPEEDUP_DEVICE*/
				GdArea(psd,ax, ay, pf->cfont_width,
					pf->font_height, bitmap, MWPF_PIXELVAL);
#endif/*SPEEDUP_DEVICE*/
			else
#if	SPEEDUP_DEVICE
				psd->DrawBlock(psd, ax, ay-pf->font_height+2, bitmap,
					       	pf->cfont_width, pf->font_height);
#else/*!SPEEDUP_DEVICE*/
				GdArea(psd,ax, ay-pf->font_height+2,
					pf->cfont_width, pf->font_height,
					bitmap, MWPF_PIXELVAL);
#endif/*SPEEDUP_DEVICE*/

                	s += 2;
                	ax += pf->cfont_width;
            	}
            	else 
		{
#if	HZK24_SMOOTH
			if (!gr_usebg)
				psd->ReadBlock(psd, ax, ay, bgbmp, pf->afont_width, pf->font_height);
			expandchar(pf, gr_background, gr_foreground, c[0], bitmap, bgbmp);
#else /*HZK24_SMOOTH*/
                	expandchar(pf, gr_background,gr_foreground,
                           c[0], bitmap);
#endif/*HZK24_SMOOTH*/
			/* Now draw the bitmap ... */

			if (flags&MWTF_TOP) 
#if	SPEEDUP_DEVICE
				psd->DrawBlock(psd, ax, ay, bitmap, pf->afont_width, pf->font_height);
#else/*!SPEEDUP_DEVICE*/
				GdArea(psd,ax, ay, pf->afont_width,
					pf->font_height, bitmap, MWPF_PIXELVAL);
#endif/*SPEEDUP_DEVICE*/
			else
#if	SPEEDUP_DEVICE
				psd->DrawBlock(psd, ax, ay-pf->font_height+2, bitmap,
					       	pf->afont_width, pf->font_height);
#else/*!SPEEDUP_DEVICE*/
				GdArea(psd,ax, ay-pf->font_height+2,
					pf->afont_width, pf->font_height,
					bitmap, MWPF_PIXELVAL);
#endif/*SPEEDUP_DEVICE*/

                	s += 1;
                	ax += pf->afont_width;
            	}
						
		if(s>=sbegin+cc)break;
    	}

#if	HZK24_SMOOTH
	if (gr_usebg) {
		FREEA(pLUTable);
	} else {
		FREEA(bgbmp);
		FREEA(bgcbmp);
	}
#endif/*HZK24_SMOOTH*/
	FREEA(bitmap);
}

#if	HAVE_SHADOW_SUPPORT
static void
expandcchar_s(PMWHZKFONT pf, int bg, int light, int dark, int fg, int border,
		unsigned char *c, MWPIXELVAL *bitmap)
{
	int posi;    /* the position in bitmap */
    	int c1, c2, seq;
	int x,y;
	int nBorder;
	MWUCHAR *font;

   	c1 = c[0];
    	c2 = c[1];
	seq=((c1 - 161)*94 + c2 - 161); 

	font = pf->cfont_address + (seq * 2 *
			  (pf->font_height * ((pf->cfont_width + 7) / 8)));

	for (y = 0; y < pf->font_height; y++)
		for (x = 0; x < pf->cfont_width; x++) {
			/* draw the background first */
			posi = y * pf->cfont_width + x;
			if (0 == Get2Bits(font + posi / 4, posi % 4))
				bitmap[posi] = bg;

			/* give the font border, if matches 1, border it */
			/* 1 pixel border */
			if (x >= 1) {
				nBorder = y * pf->cfont_width + (x - 1);
				if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
					goto cborder;

				if (y >= 1) {
					nBorder = (y - 1) * pf->cfont_width + (x - 1);
					if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
						goto cborder;
				} 
				if (y < (pf->font_height - 1)) {
					nBorder = (y + 1) * pf->cfont_width + x - 1;
					if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
						goto cborder;
				}
			}

			if (y >= 1) {
				nBorder = (y - 1) * pf->cfont_width + x;
				if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
					goto cborder;
			}

			if (y < (pf->font_height - 1)) {
				nBorder = (y + 1) * pf->cfont_width + x;
				if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
					goto cborder;
			}

			if (x < (pf->cfont_width - 1)) {
				nBorder = y * pf->cfont_width + x + 1;
				if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
					goto cborder;

				if (y >= 1) {
					nBorder = (y - 1) * pf->cfont_width + x + 1;
					if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
						goto cborder;
				}
				if (y < (pf->font_height - 1)) {
					nBorder = (y + 1) * pf->cfont_width + x + 1;
					if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
						goto cborder;
				}
			}

			/* 2 pixel border */
			if (x >= 2) {
				nBorder = y * pf->cfont_width + x - 2;
				if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
					goto cborder;

				if (y >= 2) {
					nBorder = (y - 2) * pf->cfont_width + x - 2;
					if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
						goto cborder;
				}
				if (y < (pf->font_height - 2)) {
					nBorder = (y + 2) * pf->cfont_width + x - 2;
					if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
						goto cborder;
				}
			}

			if (y >= 2) {
				nBorder = (y - 2) * pf->cfont_width + x;
				if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
					goto cborder;
			}

			if (y < (pf->font_height - 2)) {
				nBorder = (y + 2) * pf->cfont_width + x;
				if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
					goto cborder;
			}

			if (x < (pf->cfont_width - 2)) {
				nBorder = y * pf->cfont_width + x + 2;
				if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
					goto cborder;

				if (y >= 2) {
					nBorder = (y - 2) * pf->cfont_width + x + 2;
					if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
						goto cborder;
				}
				if (y < (pf->font_height - 2)) {
					nBorder = (y + 2) * pf->cfont_width + x + 2;
					if (Get2Bits(font + nBorder / 4, nBorder % 4) != 0)
						goto cborder;
				}
			}

			/* no matches occuer, no border */
			goto cnoborder;

cborder:		bitmap[y * pf->cfont_width + x] = border;

			/* draw foreground */
cnoborder:		switch (Get2Bits(font + posi / 4, posi % 4)) {
			case 1:
				bitmap[posi] = light;
				break;
			case 2:
				bitmap[posi] = dark;
				break;
			case 3:
				bitmap[posi] = fg;
				break;
			}
		}
}

static MWBOOL
GetBit(MWUCHAR *pBuf, int x, int y, int nWidth)
{
	int	nBit, nByte, nBytesInLine = (nWidth + 7) / 8;

	nBit = x % 8;
	nByte = y * nBytesInLine + x / 8;

	return (pBuf[nByte] & (128 >> nBit));
}

static void
expandchar_s(PMWHZKFONT pf, int bg, int fg, int border, int c, MWPIXELVAL *bitmap)
{
	int x,y;
    	MWUCHAR *font = pf->afont_address + c * (pf->font_height *
					((pf->afont_width + 7) / 8));

	for (y = 0; y < pf->font_height; y++)
		for (x = 0; x < pf->afont_width; x++) {
			/* set the background first */
			if (!GetBit(font, x, y, pf->afont_width))
				bitmap[y * pf->afont_width + x] = bg;

			/* give border to font, 1 matches will border it */
			if (x >= 1) {
				if (GetBit(font, x - 1, y, pf->afont_width))
					goto aborder;
				if (y >= 1) {
					if (GetBit(font, x - 1, y - 1, pf->afont_width))
						goto aborder;
				}
				if (y < (pf->font_height - 1)) {
					if (GetBit(font, x - 1, y + 1, pf->afont_width))
						goto aborder;
				}
			}

			if (y >= 1)
				if (GetBit(font, x, y - 1, pf->afont_width))
					goto aborder;

			if (y < (pf->font_height - 1))
				if (GetBit(font, x, y + 1, pf->afont_width))
					goto aborder;

			if (x < (pf->afont_width - 1)) {
				if (GetBit(font, x + 1, y, pf->afont_width))
					goto aborder;
				if (y >= 1) {
					if (GetBit(font, x + 1, y - 1, pf->afont_width))
						goto aborder;
				}
				if (y < (pf->font_height - 1)) {
					if (GetBit(font, x + 1, y + 1, pf->afont_width))
						goto aborder;
				}
			}

			/* 2 pixel border */
			if (x >= 2) {
				if (GetBit(font, x - 2, y, pf->afont_width))
					goto aborder;
				if (y >= 2) {
					if (GetBit(font, x - 2, y - 2, pf->afont_width))
						goto aborder;
				}
				if (y < (pf->font_height - 2)) {
					if (GetBit(font, x - 2, y + 2, pf->afont_width))
						goto aborder;
				}
			}

			if (y >= 2)
				if (GetBit(font, x, y - 2, pf->afont_width))
					goto aborder;

			if (y < (pf->font_height - 2))
				if (GetBit(font, x, y + 2, pf->afont_width))
					goto aborder;

			if (x < (pf->afont_width - 2)) {
				if (GetBit(font, x + 2, y, pf->afont_width))
					goto aborder;
				if (y >= 2) {
					if (GetBit(font, x + 2, y - 2, pf->afont_width))
						goto aborder;
				}
				if (y < (pf->font_height - 2)) {
					if (GetBit(font, x + 2, y + 2, pf->afont_width))
						goto aborder;
				}
			}

			goto anoborder;

aborder:		bitmap[y * pf->afont_width + x] = border;

			/* draw the foreground */
anoborder:		if (GetBit(font, x, y, pf->afont_width))
				bitmap[y * pf->afont_width + x] = fg;
  		}
}

static void
hzk_drawtext_s(PMWFONT pfont, PSD psd, MWCOORD ax, MWCOORD ay,
	       	char *text, int cc)
{
	PMWHZKFONT pf=(PMWHZKFONT)pfont;

    	unsigned char c[2];
    	unsigned char s1[3];
 	char *s,*sbegin;
	MWPIXELVAL *bitmap;
	MWPIXELVAL	bg, fg;
	MWPIXELVAL	light, dark;
	MWPIXELVAL	border;		/* black border around green bg */

	if (pf->font_height != 24) {
#ifdef	_DEBUG
		printf("16x16 doesn't support shadow text\n");
#endif/*_DEBUG*/
		return;
	}

	fg = GdFindColor(MWRGB(0x00, 0xff, 0x00));
	bg = GdFindColor(TRANS_COLOR);
	light = getlightfg(fg, &bg, 0);
	dark = getdarkfg(fg, &bg, 0);
	border = GdFindColor(MWRGB(0x00, 0x00, 0x00));

	s=(char *)text;

	if(cc==1) {
		s1[0]=*((unsigned char*)text);
		s1[1]=0x0;
		s1[2]=0x0;
		s=s1;
    	}

	sbegin=s;
    	bitmap = (MWPIXELVAL *)ALLOCA(pf->cfont_width * pf->font_height *
			sizeof(MWPIXELVAL));
	if (NULL == bitmap)
		return;

    	while( getnextchar(s, c) ) {
              	if( c[1] != '\0') {
			/* draw a CHI char */
                	expandcchar_s(pf, bg, light, dark, fg, border, c, bitmap);
#if	SPEEDUP_DEVICE
			psd->DrawBlock(psd, ax, ay, bitmap, pf->cfont_width, pf->font_height);
#else/*!SPEEDUP_DEVICE*/
			GdArea(psd,ax, ay, pf->cfont_width,
				pf->font_height, bitmap, MWPF_PIXELVAL);
#endif/*SPEEDUP_DEVICE*/
                	s += 2;
                	ax += pf->cfont_width;
            	} else {
			/* draw a ASC char */
                	expandchar_s(pf, bg, fg, border, c[0], bitmap);
#if	SPEEDUP_DEVICE
			psd->DrawBlock(psd, ax, ay, bitmap, pf->afont_width, pf->font_height);
#else/*!SPEEDUP_DEVICE*/
			GdArea(psd,ax, ay, pf->afont_width,
				pf->font_height, bitmap, MWPF_PIXELVAL);
#endif/*SPEEDUP_DEVICE*/
                	s += 1;
                	ax += pf->afont_width;
            	}
						
		if(s>=sbegin+cc)break;
    	}

	FREEA(bitmap);
}
#endif/*HAVE_SHADOW_SUPPORT*/

/*
 * Return information about a specified font.
 */
static MWBOOL
hzk_getfontinfo(PMWFONT pfont, PMWFONTINFO pfontinfo)
{
	PMWHZKFONT pf=(PMWHZKFONT)pfont;

	int i;

	pfontinfo->height = pf->font_height;
	pfontinfo->maxwidth = pf->cfont_width;
	pfontinfo->baseline = pf->font_height-2;
	pfontinfo->firstchar = 0;
	pfontinfo->lastchar = 0;
	pfontinfo->fixed = TRUE;
		
	for(i=0; i<=256; i++)
		pfontinfo->widths[i] = pf->afont_width;

	return TRUE;
}

static void
hzk_gettextsize(PMWFONT pfont, const void *text, int cc,
	MWCOORD *pwidth, MWCOORD *pheight, MWCOORD *pbase)
{
	PMWHZKFONT pf=(PMWHZKFONT)pfont;

   	unsigned char c[2];
 	char *s,*sbegin;
    	unsigned char s1[3];

	int ax=0;
	s=(char *)text;
	if(cc==0)
	{
		*pwidth = 0;
		*pheight = pf->font_height;
		*pbase = pf->font_height-2;

	}
	if(cc==1)
	{
		s1[0]=*((unsigned char*)text);
		s1[1]=0x0;
		s1[2]=0x0;
		s=s1;
    	}
	sbegin=s;
    	while( getnextchar(s, c) )
	{
		if( c[1] != '\0') 
		{
           		s += 2;
           		ax += pf->cfont_width;
        	}
        	else 
		{
           		s += 1;
           		ax += pf->afont_width;
        	}
		if(s>=sbegin+cc) {
			/*fprintf(stderr,"s=%x,sbegin=%x,cc=%x\n",s,sbegin,cc);*/
			break;
		}

    	}
	/*fprintf(stderr,"ax=%d,\n",ax);*/

	*pwidth = ax;
	*pheight = pf->font_height;
	*pbase = pf->font_height-2;
}

static void
hzk_destroyfont(PMWFONT pfont)
{
	PMWHZKFONT pf=(PMWHZKFONT)pfont;
	if( pf )
		UnloadFont(pf);
	free(pf);
}

static void
hzk_setfontsize(PMWFONT pfont, MWCOORD fontsize)
{
	PMWHZKFONT pf=(PMWHZKFONT)pfont;
	/* jmt: hzk_setfontsize not supported*/
	/* & pf->fontsize can't be changed*/
	/* because of hzk_id() :p*/
	pf->fontsize=pf->font_height;
}

#endif /* HAVE_HZK_SUPPORT*/

/* FIXME: this routine should work for all font renderers...*/
int
GdGetTextSizeEx(PMWFONT pfont, const void *str, int cc,int nMaxExtent,
	int* lpnFit, int* alpDx,MWCOORD *pwidth,MWCOORD *pheight,
	MWCOORD *pbase, int flags)
{
	*pwidth = *pheight = *pbase = 0;
	return 0;
}

/* by yuanlii@29bbs.net, 07/11/2005 14:03:58 */
#if	HAVE_HUGENUM_SUPPORT
typedef struct {
	int	width;
	int	height;
	int	size;		/* buffer size in bytes */
	unsigned long use_count;
	MWUCHAR *pFont;		/* pointer to the buffer */
	char	file[MAX_PATH + 1];
} HUGENUMBER;

static HUGENUMBER hugeNumber;
/***
 * @ingroup GdDrawHugeNumber
 * 	LoadHugeNumber load huge number for display from file to buffer
 * @param nHugeSize the size of the huge number
 * @return 1 success or
 * 	   0 failed
 */
MWBOOL
GdLoadHugeNumber(int nHugeSize)
{
	if (NULL == hugeNumber.pFont) {
		int		fd;
		char		szTmp[32];
		struct stat	st;

		/* init the hugeNumber struct */
		hugeNumber.height = nHugeSize;
		hugeNumber.width = nHugeSize / 2;
		hugeNumber.size = ((hugeNumber.width + 7) / 8) * 2 /* bi */
					* hugeNumber.height * 10;  /* only load 0 ~ 9 */
		strcpy(hugeNumber.file, HZK_FONT_DIR);
		sprintf(szTmp, "%s%db", "/huge", nHugeSize);
		strcat(hugeNumber.file, szTmp);

		fprintf(stderr, "Loading huge number font from file(%s)...", hugeNumber.file);
		/* reading the font file */
		fd = open(hugeNumber.file, O_RDONLY);
		if (fd < 0 || fstat(fd, &st) < 0) {
			fprintf(stderr, "%s open error\n", hugeNumber.file);
			close(fd);
			return FALSE;
		}
		/* check the file size */
		if (st.st_size != hugeNumber.size){
			fprintf(stderr, "%s has wrong size\n", hugeNumber.file);
			close(fd);
			return FALSE;
		}

		/* alloc memory for the font */
		hugeNumber.pFont = (MWUCHAR *)malloc(hugeNumber.size * sizeof(MWUCHAR));
		if (NULL == hugeNumber.pFont) {
	 	       	fprintf (stderr, "Allocate memory for Huge Number font failure.\n");
			close(fd);
		        return FALSE;
		}

		if (hugeNumber.size != read(fd, hugeNumber.pFont, hugeNumber.size)) {
			fprintf(stderr, "%s read error\n", hugeNumber.file);
			free(hugeNumber.pFont);
			hugeNumber.pFont = NULL;
			close(fd);
			return FALSE;
		}

		close(fd);
		hugeNumber.use_count = 0;
		fprintf(stderr, "done.\n" );
	}

	hugeNumber.use_count++;
	return TRUE;
}

/***
 * @ingroup GdDrawHugeNumber
 * 	FreeHugeNumber free the font buffer
 * @no param
 */
void
GdFreeHugeNumber(void)
{
	hugeNumber.use_count--;
	if (0 == hugeNumber.use_count) {
		fprintf(stderr, "Unloading huge number huge%db...",
			       	hugeNumber.height);
		free(hugeNumber.pFont);
		hugeNumber.pFont = NULL;
		fprintf(stderr, "done.\n" );
	}
}

/***
 * @ingroup GdDrawHugeNumber
 * 	DrawHugeNumber draw huge number
 * @param szNumber the number string to display
 * @param nCount how many numbers to display
 * @param x
 * 	  y the position to display the number string
 */
static void expand_hugenum(MWUCHAR c, MWPIXELVAL bg, MWPIXELVAL fg,
	       	MWPIXELVAL border, MWPIXELVAL *bitmap);

void
GdDrawHugeNumber(PSD psd, MWCOORD x, MWCOORD y, char *szNumber, int nCount)
{
    	unsigned char	c[2];
    	unsigned char	s1[3];
 	unsigned char	*s,*sbegin;
	MWPIXELVAL	*bitmap;
	MWPIXELVAL	bg, fg;
	MWPIXELVAL	border;		/* black border around green bg */

	fg = GdFindColor(MWRGB(0x00, 0xff, 0x00));
	bg = GdFindColor(TRANS_COLOR);
	border = GdFindColor(MWRGB(0x00, 0x00, 0x00));

	/* get ready for the string */
	s=(char *)szNumber;

	if(nCount==1)
	{
		s1[0]=*((unsigned char*)szNumber);
		s1[1]=0x0;
		s1[2]=0x0;
		s=s1;
    	}

	sbegin=s;
    	bitmap = (MWPIXELVAL *)ALLOCA(hugeNumber.width * hugeNumber.height *
			sizeof(MWPIXELVAL));
	if (NULL == bitmap)
		return;

    	while( getnextchar(s, c) ) {
              	if( c[1] != '\0') {
			fprintf(stderr, "Not a ascii char\n");
			break;
		} else if (c[0] < '0' || c[0] > '9') {
			fprintf(stderr, "Not a numeric\n");
			break;
		} else {
			expand_hugenum(c[0], bg, fg, border, bitmap);


#if	SPEEDUP_DEVICE
			psd->DrawBlock(psd, x, y, bitmap, hugeNumber.width, hugeNumber.height);
#else/*!SPEEDUP_DEVICE*/
			GdArea(psd, x, y, hugeNumber.width,
				hugeNumber.height, bitmap, MWPF_PIXELVAL);
#endif/*SPEEDUP_DEVICE*/

                	s += 1;
                	x += hugeNumber.width;
		}
		if(s>=sbegin+nCount)
			break;
	}
	FREEA(bitmap);
}

static void
expand_hugenum(MWUCHAR c, MWPIXELVAL bg, MWPIXELVAL fg, MWPIXELVAL border,
			  MWPIXELVAL *bitmap)
{
	int	x,y;
	int	posi;
    	MWUCHAR	*font;

	font = hugeNumber.pFont + (c - '0') * (hugeNumber.height *
		((hugeNumber.width + 7) / 8)) * 2;

	for (y = 0; y < hugeNumber.height; y++)
		for (x = 0; x < hugeNumber.width; x++) {
			posi = y * hugeNumber.width + x;
			switch (Get2Bits(font + posi / 4, posi % 4)) {
			case 0:
				bitmap[posi] = bg;
				break;
			case 2:
				bitmap[posi] = border;
				break;
			case 3:
				bitmap[posi] = fg;
				break;
			default:
#ifdef	_DEBUG
				fprintf(stderr, "error Get2Bits return value\n");
#endif/*_DEBUG*/
			}
		}
}
#endif/*HAVE_HUGENUM_SUPPORT*/
/* by yuanlii@29bbs.net */

