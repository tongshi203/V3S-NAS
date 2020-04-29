///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2006-6-22
///
///		用途：
///			DVB 图文页解释
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

// mydvbteletextpage.cpp: implementation of the CMyDVBTeletextPage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mydvbteletextpage.h"
#include <stdio.h>

#if defined(_DEBUG) && defined(_MSC_VER)
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

// extern unsigned char g_ttfont_8x10[];
extern unsigned char g_abyTeletextFont_24x20[];
extern int g_nTag;
const int TELETEXT_FONT_BYTE_PER_FONTS = 20*3;

#define EA_HDOUBLE		1	// 倍高
#define EA_BOX			2	// 加框
#define EA_CONCEALED	4	// 隐匿
#define EA_SEMIGRAY		8	// 半灰度
#define EA_INVERSE		16	// 反转
#define EA_UNDERLINE    32	// 下划线
#define EA_WDOUBLE		64	// 倍宽

#define EA_GRAPHIC		128	// graphic symbol
#define EA_SEPARATED    256	// use separated graphic symbol
#define	EA_BLINK		512	// blink

#define E_DEF_FG	7
#define E_DEF_BG	0
#define E_DEF_ATTR	0

static unsigned char invtab[256] =
{
  0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
  0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
  0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
  0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
  0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
  0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
  0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
  0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
  0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
  0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
  0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
  0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
  0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
  0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
  0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
  0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
  0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
  0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
  0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
  0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
  0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
  0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
  0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
  0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
  0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
  0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
  0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
  0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
  0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
  0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
  0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
  0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

static unsigned char unhamtab[256] =
{
  0x01, 0xff, 0x81, 0x01, 0xff, 0x00, 0x01, 0xff,
  0xff, 0x02, 0x01, 0xff, 0x0a, 0xff, 0xff, 0x07,
  0xff, 0x00, 0x01, 0xff, 0x00, 0x80, 0xff, 0x00,
  0x06, 0xff, 0xff, 0x0b, 0xff, 0x00, 0x03, 0xff,
  0xff, 0x0c, 0x01, 0xff, 0x04, 0xff, 0xff, 0x07,
  0x06, 0xff, 0xff, 0x07, 0xff, 0x07, 0x07, 0x87,
  0x06, 0xff, 0xff, 0x05, 0xff, 0x00, 0x0d, 0xff,
  0x86, 0x06, 0x06, 0xff, 0x06, 0xff, 0xff, 0x07,
  0xff, 0x02, 0x01, 0xff, 0x04, 0xff, 0xff, 0x09,
  0x02, 0x82, 0xff, 0x02, 0xff, 0x02, 0x03, 0xff,
  0x08, 0xff, 0xff, 0x05, 0xff, 0x00, 0x03, 0xff,
  0xff, 0x02, 0x03, 0xff, 0x03, 0xff, 0x83, 0x03,
  0x04, 0xff, 0xff, 0x05, 0x84, 0x04, 0x04, 0xff,
  0xff, 0x02, 0x0f, 0xff, 0x04, 0xff, 0xff, 0x07,
  0xff, 0x05, 0x05, 0x85, 0x04, 0xff, 0xff, 0x05,
  0x06, 0xff, 0xff, 0x05, 0xff, 0x0e, 0x03, 0xff,
  0xff, 0x0c, 0x01, 0xff, 0x0a, 0xff, 0xff, 0x09,
  0x0a, 0xff, 0xff, 0x0b, 0x8a, 0x0a, 0x0a, 0xff,
  0x08, 0xff, 0xff, 0x0b, 0xff, 0x00, 0x0d, 0xff,
  0xff, 0x0b, 0x0b, 0x8b, 0x0a, 0xff, 0xff, 0x0b,
  0x0c, 0x8c, 0xff, 0x0c, 0xff, 0x0c, 0x0d, 0xff,
  0xff, 0x0c, 0x0f, 0xff, 0x0a, 0xff, 0xff, 0x07,
  0xff, 0x0c, 0x0d, 0xff, 0x0d, 0xff, 0x8d, 0x0d,
  0x06, 0xff, 0xff, 0x0b, 0xff, 0x0e, 0x0d, 0xff,
  0x08, 0xff, 0xff, 0x09, 0xff, 0x09, 0x09, 0x89,
  0xff, 0x02, 0x0f, 0xff, 0x0a, 0xff, 0xff, 0x09,
  0x88, 0x08, 0x08, 0xff, 0x08, 0xff, 0xff, 0x09,
  0x08, 0xff, 0xff, 0x0b, 0xff, 0x0e, 0x03, 0xff,
  0xff, 0x0c, 0x0f, 0xff, 0x04, 0xff, 0xff, 0x09,
  0x0f, 0xff, 0x8f, 0x0f, 0xff, 0x0e, 0x0f, 0xff,
  0x08, 0xff, 0xff, 0x05, 0xff, 0x0e, 0x0d, 0xff,
  0xff, 0x0e, 0x0f, 0xff, 0x0e, 0x8e, 0xff, 0x0e,
};

// this table generates the parity checks for hamm24/18 decoding.
// bit 0 is for test A, 1 for B, ...
// thanks to R. Gancarz for this fine table *g*

static char hamm24par[3][256] =
{
    { // parities of first byte
		0, 33, 34,  3, 35,  2,  1, 32, 36,  5,  6, 39,  7, 38, 37,  4,
		37,  4,  7, 38,  6, 39, 36,  5,  1, 32, 35,  2, 34,  3,  0, 33,
		38,  7,  4, 37,  5, 36, 39,  6,  2, 35, 32,  1, 33,  0,  3, 34,
		3, 34, 33,  0, 32,  1,  2, 35, 39,  6,  5, 36,  4, 37, 38,  7,
		39,  6,  5, 36,  4, 37, 38,  7,  3, 34, 33,  0, 32,  1,  2, 35,
		2, 35, 32,  1, 33,  0,  3, 34, 38,  7,  4, 37,  5, 36, 39,  6,
		1, 32, 35,  2, 34,  3,  0, 33, 37,  4,  7, 38,  6, 39, 36,  5,
		36,  5,  6, 39,  7, 38, 37,  4,  0, 33, 34,  3, 35,  2,  1, 32,
		40,  9, 10, 43, 11, 42, 41,  8, 12, 45, 46, 15, 47, 14, 13, 44,
		13, 44, 47, 14, 46, 15, 12, 45, 41,  8, 11, 42, 10, 43, 40,  9,
		14, 47, 44, 13, 45, 12, 15, 46, 42, 11,  8, 41,  9, 40, 43, 10,
		43, 10,  9, 40,  8, 41, 42, 11, 15, 46, 45, 12, 44, 13, 14, 47,
		15, 46, 45, 12, 44, 13, 14, 47, 43, 10,  9, 40,  8, 41, 42, 11,
		42, 11,  8, 41,  9, 40, 43, 10, 14, 47, 44, 13, 45, 12, 15, 46,
		41,  8, 11, 42, 10, 43, 40,  9, 13, 44, 47, 14, 46, 15, 12, 45,
		12, 45, 46, 15, 47, 14, 13, 44, 40,  9, 10, 43, 11, 42, 41,  8
    },
	{ // parities of second byte
		0, 41, 42,  3, 43,  2,  1, 40, 44,  5,  6, 47,  7, 46, 45,  4,
		45,  4,  7, 46,  6, 47, 44,  5,  1, 40, 43,  2, 42,  3,  0, 41,
		46,  7,  4, 45,  5, 44, 47,  6,  2, 43, 40,  1, 41,  0,  3, 42,
		3, 42, 41,  0, 40,  1,  2, 43, 47,  6,  5, 44,  4, 45, 46,  7,
		47,  6,  5, 44,  4, 45, 46,  7,  3, 42, 41,  0, 40,  1,  2, 43,
		2, 43, 40,  1, 41,  0,  3, 42, 46,  7,  4, 45,  5, 44, 47,  6,
		1, 40, 43,  2, 42,  3,  0, 41, 45,  4,  7, 46,  6, 47, 44,  5,
		44,  5,  6, 47,  7, 46, 45,  4,  0, 41, 42,  3, 43,  2,  1, 40,
		48, 25, 26, 51, 27, 50, 49, 24, 28, 53, 54, 31, 55, 30, 29, 52,
		29, 52, 55, 30, 54, 31, 28, 53, 49, 24, 27, 50, 26, 51, 48, 25,
		30, 55, 52, 29, 53, 28, 31, 54, 50, 27, 24, 49, 25, 48, 51, 26,
		51, 26, 25, 48, 24, 49, 50, 27, 31, 54, 53, 28, 52, 29, 30, 55,
		31, 54, 53, 28, 52, 29, 30, 55, 51, 26, 25, 48, 24, 49, 50, 27,
		50, 27, 24, 49, 25, 48, 51, 26, 30, 55, 52, 29, 53, 28, 31, 54,
		49, 24, 27, 50, 26, 51, 48, 25, 29, 52, 55, 30, 54, 31, 28, 53,
		28, 53, 54, 31, 55, 30, 29, 52, 48, 25, 26, 51, 27, 50, 49, 24
	},
	{ // parities of third byte
		63, 14, 13, 60, 12, 61, 62, 15, 11, 58, 57,  8, 56,  9, 10, 59,
		10, 59, 56,  9, 57,  8, 11, 58, 62, 15, 12, 61, 13, 60, 63, 14,
		9, 56, 59, 10, 58, 11,  8, 57, 61, 12, 15, 62, 14, 63, 60, 13,
		60, 13, 14, 63, 15, 62, 61, 12,  8, 57, 58, 11, 59, 10,  9, 56,
		8, 57, 58, 11, 59, 10,  9, 56, 60, 13, 14, 63, 15, 62, 61, 12,
		61, 12, 15, 62, 14, 63, 60, 13,  9, 56, 59, 10, 58, 11,  8, 57,
		62, 15, 12, 61, 13, 60, 63, 14, 10, 59, 56,  9, 57,  8, 11, 58,
		11, 58, 57,  8, 56,  9, 10, 59, 63, 14, 13, 60, 12, 61, 62, 15,
		31, 46, 45, 28, 44, 29, 30, 47, 43, 26, 25, 40, 24, 41, 42, 27,
		42, 27, 24, 41, 25, 40, 43, 26, 30, 47, 44, 29, 45, 28, 31, 46,
		41, 24, 27, 42, 26, 43, 40, 25, 29, 44, 47, 30, 46, 31, 28, 45,
		28, 45, 46, 31, 47, 30, 29, 44, 40, 25, 26, 43, 27, 42, 41, 24,
		40, 25, 26, 43, 27, 42, 41, 24, 28, 45, 46, 31, 47, 30, 29, 44,
		29, 44, 47, 30, 46, 31, 28, 45, 41, 24, 27, 42, 26, 43, 40, 25,
		30, 47, 44, 29, 45, 28, 31, 46, 42, 27, 24, 41, 25, 40, 43, 26,
		43, 26, 25, 40, 24, 41, 42, 27, 31, 46, 45, 28, 44, 29, 30, 47
	}
};



// table to extract the lower 4 bit from hamm24/18 encoded bytes

static char hamm24val[256] =
{
      0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,
      2,  2,  2,  2,  3,  3,  3,  3,  2,  2,  2,  2,  3,  3,  3,  3,
      4,  4,  4,  4,  5,  5,  5,  5,  4,  4,  4,  4,  5,  5,  5,  5,
      6,  6,  6,  6,  7,  7,  7,  7,  6,  6,  6,  6,  7,  7,  7,  7,
      8,  8,  8,  8,  9,  9,  9,  9,  8,  8,  8,  8,  9,  9,  9,  9,
     10, 10, 10, 10, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11, 11, 11,
     12, 12, 12, 12, 13, 13, 13, 13, 12, 12, 12, 12, 13, 13, 13, 13,
     14, 14, 14, 14, 15, 15, 15, 15, 14, 14, 14, 14, 15, 15, 15, 15,
      0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,
      2,  2,  2,  2,  3,  3,  3,  3,  2,  2,  2,  2,  3,  3,  3,  3,
      4,  4,  4,  4,  5,  5,  5,  5,  4,  4,  4,  4,  5,  5,  5,  5,
      6,  6,  6,  6,  7,  7,  7,  7,  6,  6,  6,  6,  7,  7,  7,  7,
      8,  8,  8,  8,  9,  9,  9,  9,  8,  8,  8,  8,  9,  9,  9,  9,
     10, 10, 10, 10, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11, 11, 11,
     12, 12, 12, 12, 13, 13, 13, 13, 12, 12, 12, 12, 13, 13, 13, 13,
     14, 14, 14, 14, 15, 15, 15, 15, 14, 14, 14, 14, 15, 15, 15, 15
};



// mapping from parity checks made by table hamm24par to error
// results return by hamm24.
// (0 = no error, 0x0100 = single bit error, 0x1000 = double error)

static short hamm24err[64] =
{
    0x0000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
};



// mapping from parity checks made by table hamm24par to faulty bit
// in the decoded 18 bit word.

static int hamm24cor[64] =
{
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00001, 0x00000, 0x00002, 0x00004, 0x00008,
    0x00000, 0x00010, 0x00020, 0x00040, 0x00080, 0x00100, 0x00200, 0x00400,
    0x00000, 0x00800, 0x01000, 0x02000, 0x04000, 0x08000, 0x10000, 0x20000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
};

/*
*  Teletext character set
*
*  (Similar to ISO 6937 -
*   ftp://dkuug.dk/i18n/charmaps/ISO_6937)
*/

/*
*  ETS 300 706 Table 36: Latin National Option Sub-sets
*
*  Latin G0 character code to Unicode mapping per national subset,
*  unmodified codes (NO_SUBSET) in row zero.
*
*  [13][0] Turkish currency symbol not in Unicode, use private code U+E800
*/
static const unsigned short national_subset[14][13] =
{
	{ 0x0023u, 0x0024u, 0x0040u, 0x005Bu, 0x005Cu, 0x005Du, 0x005Eu, 0x005Fu, 0x0060u, 0x007Bu, 0x007Cu, 0x007Du, 0x007Eu },
	{ 0x0023u, 0x016Fu, 0x010Du, 0x0165u, 0x017Eu, 0x00FDu, 0x00EDu, 0x0159u, 0x00E9u, 0x00E1u, 0x011Bu, 0x00FAu, 0x0161u },
	{ 0x00A3u, 0x0024u, 0x0040u, 0x2190u, 0x00BDu, 0x2192u, 0x2191u, 0x0023u, 0x2014u, 0x00BCu, 0x2016u, 0x00BEu, 0x00F7u },
	{ 0x0023u, 0x00F5u, 0x0160u, 0x00C4u, 0x00D6u, 0x017Du, 0x00DCu, 0x00D5u, 0x0161u, 0x00E4u, 0x00F6u, 0x017Eu, 0x00FCu },
	{ 0x00E9u, 0x00EFu, 0x00E0u, 0x00EBu, 0x00EAu, 0x00F9u, 0x00EEu, 0x0023u, 0x00E8u, 0x00E2u, 0x00F4u, 0x00FBu, 0x00E7u },
	{ 0x0023u, 0x0024u, 0x00A7u, 0x00C4u, 0x00D6u, 0x00DCu, 0x005Eu, 0x005Fu, 0x00B0u, 0x00E4u, 0x00F6u, 0x00FCu, 0x00DFu },
	{ 0x00A3u, 0x0024u, 0x00E9u, 0x00B0u, 0x00E7u, 0x2192u, 0x2191u, 0x0023u, 0x00F9u, 0x00E0u, 0x00F2u, 0x00E8u, 0x00ECu },
	{ 0x0023u, 0x0024u, 0x0160u, 0x0117u, 0x0229u, 0x017Du, 0x010Du, 0x016Bu, 0x0161u, 0x0105u, 0x0173u, 0x017Eu, 0x012Fu },
	{ 0x0023u, 0x0144u, 0x0105u, 0x01B5u, 0x015Au, 0x0141u, 0x0107u, 0x00F3u, 0x0119u, 0x017Cu, 0x015Bu, 0x0142u, 0x017Au },
	{ 0x00E7u, 0x0024u, 0x00A1u, 0x00E1u, 0x00E9u, 0x00EDu, 0x00F3u, 0x00FAu, 0x00BFu, 0x00FCu, 0x00F1u, 0x00E8u, 0x00E0u },
	{ 0x0023u, 0x00A4u, 0x0162u, 0x00C2u, 0x015Eu, 0x01CDu, 0x00CDu, 0x0131u, 0x0163u, 0x00E2u, 0x015Fu, 0X01CEu, 0x00EEu },
	{ 0x0023u, 0x00CBu, 0x010Cu, 0x0106u, 0x017Du, 0x00D0u, 0x0160u, 0x00EBu, 0x010Du, 0x0107u, 0x017Eu, 0x00F0u, 0x0161u },
	{ 0x0023u, 0x00A4u, 0x00C9u, 0x00C4u, 0x00D6u, 0x00C5u, 0x00DCu, 0x005Fu, 0x00E9u, 0x00E4u, 0x00F6u, 0x00E5u, 0x00FCu },
	{ 0xE800u, 0x011Fu, 0x0130u, 0x015Eu, 0x00D6u, 0x00C7u, 0x00DCu, 0x011Eu, 0x0131u, 0x015Fu, 0x00F6u, 0x00E7u, 0x00FCu }
};

/*
*  ETS 300 706 Table 37: Latin G2 Supplementary Set
*
*  0x49 seems to be dot below; not in Unicode (except combining), use U+002E.
*/
static const unsigned short latin_g2[96] =
{
	0x00A0u, 0x00A1u, 0x00A2u, 0x00A3u, 0x0024u, 0x00A5u, 0x0023u, 0x00A7u, 0x00A4u, 0x2018u, 0x201Cu, 0x00ABu, 0x2190u, 0x2191u, 0x2192u, 0x2193u,
	0x00B0u, 0x00B1u, 0x00B2u, 0x00B3u, 0x00D7u, 0x00B5u, 0x00B6u, 0x00B7u, 0x00F7u, 0x2019u, 0x201Du, 0x00BBu, 0x00BCu, 0x00BDu, 0x00BEu, 0x00BFu,
	0x0020u, 0x02CBu, 0x02CAu, 0x02C6u, 0x02DCu, 0x02C9u, 0x02D8u, 0x02D9u, 0x00A8u, 0x002Eu, 0x02DAu, 0x02CFu, 0x02CDu, 0x02DDu, 0x02DBu, 0x02C7u,
	0x2014u, 0x00B9u, 0x00AEu, 0x00A9u, 0x2122u, 0x266Au, 0x20A0u, 0x2030u, 0x0251u, 0x0020u, 0x0020u, 0x0020u, 0x215Bu, 0x215Cu, 0x215Du, 0x215Eu,
	0x2126u, 0x00C6u, 0x00D0u, 0x00AAu, 0x0126u, 0x0020u, 0x0132u, 0x013Fu, 0x0141u, 0x00D8u, 0x0152u, 0x00BAu, 0x00DEu, 0x0166u, 0x014Au, 0x0149u,
	0x0138u, 0x00E6u, 0x0111u, 0x00F0u, 0x0127u, 0x0131u, 0x0133u, 0x0140u, 0x0142u, 0x00F8u, 0x0153u, 0x00DFu, 0x00FEu, 0x0167u, 0x014Bu, 0x25A0u
};

/*
*  ETS 300 706 Table 38: Cyrillic G0 Primary Set - Option 1 - Serbian/Croatian
*/
static const unsigned short cyrillic_1_g0[64] =
{
	0x0427u, 0x0410u, 0x0411u, 0x0426u, 0x0414u, 0x0415u, 0x0424u, 0x0413u, 0x0425u, 0x0418u, 0x0408u, 0x041Au, 0x041Bu, 0x041Cu, 0x041Du, 0x041Eu,
	0x041Fu, 0x040Cu, 0x0420u, 0x0421u, 0x0422u, 0x0423u, 0x0412u, 0x0403u, 0x0409u, 0x040Au, 0x0417u, 0x040Bu, 0x0416u, 0x0402u, 0x0428u, 0x040Fu,
	0x0447u, 0x0430u, 0x0431u, 0x0446u, 0x0434u, 0x0435u, 0x0444u, 0x0433u, 0x0445u, 0x0438u, 0x0458u, 0x043Au, 0x043Bu, 0x043Cu, 0x043Du, 0x043Eu,
	0x043Fu, 0x045Cu, 0x0440u, 0x0441u, 0x0442u, 0x0443u, 0x0432u, 0x0453u, 0x0459u, 0x045Au, 0x0437u, 0x045Bu, 0x0436u, 0x0452u, 0x0448u, 0x25A0u
};

/*
*  ETS 300 706 Table 39: Cyrillic G0 Primary Set - Option 2 - Russian/Bulgarian
*/
static const unsigned short cyrillic_2_g0[64] =
{
	0x042Eu, 0x0410u, 0x0411u, 0x0426u, 0x0414u, 0x0415u, 0x0424u, 0x0413u, 0x0425u, 0x0418u, 0x040Du, 0x041Au, 0x041Bu, 0x041Cu, 0x041Du, 0x041Eu,
	0x041Fu, 0x042Fu, 0x0420u, 0x0421u, 0x0422u, 0x0423u, 0x0416u, 0x0412u, 0x042Cu, 0x042Au, 0x0417u, 0x0428u, 0x042Du, 0x0429u, 0x0427u, 0x042Bu,
	0x044Eu, 0x0430u, 0x0431u, 0x0446u, 0x0434u, 0x0435u, 0x0444u, 0x0433u, 0x0445u, 0x0438u, 0x045Du, 0x043Au, 0x043Bu, 0x043Cu, 0x043Du, 0x043Eu,
	0x043Fu, 0x044Fu, 0x0440u, 0x0441u, 0x0442u, 0x0443u, 0x0436u, 0x0432u, 0x044Cu, 0x044Au, 0x0437u, 0x0448u, 0x044Du, 0x0449u, 0x0447u, 0x25A0u
};

/*
*  ETS 300 706 Table 40: Cyrillic G0 Primary Set - Option 3 - Ukrainian
*/
static const unsigned short cyrillic_3_g0[64] =
{
	0x042Eu, 0x0410u, 0x0411u, 0x0426u, 0x0414u, 0x0415u, 0x0424u, 0x0413u, 0x0425u, 0x0418u, 0x040Du, 0x041Au, 0x041Bu, 0x041Cu, 0x041Du, 0x041Eu,
	0x041Fu, 0x042Fu, 0x0420u, 0x0421u, 0x0422u, 0x0423u, 0x0416u, 0x0412u, 0x042Cu, 0x0406u, 0x0417u, 0x0428u, 0x0404u, 0x0429u, 0x0427u, 0x0407u,
	0x044Eu, 0x0430u, 0x0431u, 0x0446u, 0x0434u, 0x0435u, 0x0444u, 0x0433u, 0x0445u, 0x0438u, 0x045Du, 0x043Au, 0x043Bu, 0x043Cu, 0x043Du, 0x043Eu,
	0x043Fu, 0x044Fu, 0x0440u, 0x0441u, 0x0442u, 0x0443u, 0x0436u, 0x0432u, 0x044Cu, 0x0456u, 0x0437u, 0x0448u, 0x0454u, 0x0449u, 0x0447u, 0x25A0u
};

/*
*  ETS 300 706 Table 41: Cyrillic G2 Supplementary Set
*/
static const unsigned short cyrillic_g2[96] =
{
	0x00A0u, 0x00A1u, 0x00A2u, 0x00A3u, 0x0020u, 0x00A5u, 0x0023u, 0x00A7u, 0x0020u, 0x2018u, 0x201Cu, 0x00ABu, 0x2190u, 0x2191u, 0x2192u, 0x2193u,
	0x00B0u, 0x00B1u, 0x00B2u, 0x00B3u, 0x00D7u, 0x00B5u, 0x00B6u, 0x00B7u, 0x00F7u, 0x2019u, 0x201Du, 0x00BBu, 0x00BCu, 0x00BDu, 0x00BEu, 0x00BFu,
	0x0020u, 0x02CBu, 0x02CAu, 0x02C6u, 0x02DCu, 0x02C9u, 0x02D8u, 0x02D9u, 0x00A8u, 0x002Eu, 0x02DAu, 0x02CFu, 0x02CDu, 0x02DDu, 0x02DBu, 0x02C7u,
	0x2014u, 0x00B9u, 0x00AEu, 0x00A9u, 0x2122u, 0x266Au, 0x20A0u, 0x2030u, 0x0251u, 0x0141u, 0x0142u, 0x00DFu, 0x215Bu, 0x215Cu, 0x215Du, 0x215Eu,
	0x0044u, 0x0045u, 0x0046u, 0x0047u, 0x0049u, 0x004Au, 0x004Bu, 0x004Cu, 0x004Eu, 0x0051u, 0x0052u, 0x0053u, 0x0055u, 0x0056u, 0x0057u, 0x005Au,
	0x0064u, 0x0065u, 0x0066u, 0x0067u, 0x0069u, 0x006Au, 0x006Bu, 0x006Cu, 0x006Eu, 0x0071u, 0x0072u, 0x0073u, 0x0075u, 0x0076u, 0x0077u, 0x007Au
};

/*
*  ETS 300 706 Table 42: Greek G0 Primary Set
*/
static const unsigned short greek_g0[64] =
{
	0x0390u, 0x0391u, 0x0392u, 0x0393u, 0x0394u, 0x0395u, 0x0396u, 0x0397u, 0x0398u, 0x0399u, 0x039Au, 0x039Bu, 0x039Cu, 0x039Du, 0x039Eu, 0x039Fu,
	0x03A0u, 0x03A1u, 0x0374u, 0x03A3u, 0x03A4u, 0x03A5u, 0x03A6u, 0x03A7u, 0x03A8u, 0x03A9u, 0x03AAu, 0x03ABu, 0x03ACu, 0x03ADu, 0x03AEu, 0x03AFu,
	0x03B0u, 0x03B1u, 0x03B2u, 0x03B3u, 0x03B4u, 0x03B5u, 0x03B6u, 0x03B7u, 0x03B8u, 0x03B9u, 0x03BAu, 0x03BBu, 0x03BCu, 0x03BDu, 0x03BEu, 0x03BFu,
	0x03C0u, 0x03C1u, 0x03C2u, 0x03C3u, 0x03C4u, 0x03C5u, 0x03C6u, 0x03C7u, 0x03C8u, 0x03C9u, 0x03CAu, 0x03CBu, 0x03CCu, 0x03CDu, 0x03CEu, 0x25A0u
};

/*
*  ETS 300 706 Table 43: Greek G2 Supplementary Set
*/
static const unsigned short greek_g2[96] =
{
	0x00A0u, 0x0061u, 0x0062u, 0x00A3u, 0x0065u, 0x0068u, 0x0069u, 0x00A7u, 0x003Au, 0x2018u, 0x201Cu, 0x006Bu, 0x2190u, 0x2191u, 0x2192u, 0x2193u,
	0x00B0u, 0x00B1u, 0x00B2u, 0x00B3u, 0x0078u, 0x006Du, 0x006Eu, 0x0070u, 0x00F7u, 0x2019u, 0x201Du, 0x0074u, 0x00BCu, 0x00BDu, 0x00BEu, 0x0078u,
	0x0020u, 0x02CBu, 0x02CAu, 0x02C6u, 0x02DCu, 0x02C9u, 0x02D8u, 0x02D9u, 0x00A8u, 0x002Eu, 0x02DAu, 0x02CFu, 0x02CDu, 0x02DDu, 0x02DBu, 0x02C7u,
	0x003Fu, 0x00B9u, 0x00AEu, 0x00A9u, 0x2122u, 0x266Au, 0x20A0u, 0x2030u, 0x0251u, 0x038Au, 0x038Eu, 0x038Fu, 0x215Bu, 0x215Cu, 0x215Du, 0x215Eu,
	0x0043u, 0x0044u, 0x0046u, 0x0047u, 0x004Au, 0x004Cu, 0x0051u, 0x0052u, 0x0053u, 0x0055u, 0x0056u, 0x0057u, 0x0059u, 0x005Au, 0x0386u, 0x0389u,
	0x0063u, 0x0064u, 0x0066u, 0x0067u, 0x006Au, 0x006Cu, 0x0071u, 0x0072u, 0x0073u, 0x0075u, 0x0076u, 0x0077u, 0x0079u, 0x007Au, 0x0388u, 0x25A0u
};

/*
*  ETS 300 706 Table 44: Arabic G0 Primary Set
*
*  XXX 0X0000 is what?
*  Until these tables are finished use private codes U+E6xx.
*/
static const unsigned short arabic_g0[96] =
{
	0x0020u, 0x0021u, 0x0022u, 0x00A3u, 0x0024u, 0x0025u, 0xE606u, 0xE607u, 0x0029u, 0x0028u, 0x002Au, 0x002Bu, 0x060Cu, 0x002Du, 0x002Eu, 0x002Fu,
	0x0030u, 0x0031u, 0x0032u, 0x0033u, 0x0034u, 0x0035u, 0x0036u, 0x0037u, 0x0038u, 0x0039u, 0x003Au, 0x061Bu, 0x003Eu, 0x003Du, 0x003Cu, 0x061Fu,
	0xE620u, 0xE621u, 0xE622u, 0xE623u, 0xE624u, 0xE625u, 0xE626u, 0xE627u, 0xE628u, 0xE629u, 0xE62Au, 0xE62Bu, 0xE62Cu, 0xE62Du, 0xE62Eu, 0xE62Fu,
	0xE630u, 0xE631u, 0xE632u, 0xE633u, 0xE634u, 0xE635u, 0xE636u, 0xE637u, 0xE638u, 0xE639u, 0xE63Au, 0xE63Bu, 0xE63Cu, 0xE63Du, 0xE63Eu, 0x0023u,
	0xE640u, 0xE641u, 0xE642u, 0xE643u, 0xE644u, 0xE645u, 0xE646u, 0xE647u, 0xE648u, 0xE649u, 0xE64Au, 0xE64Bu, 0xE64Cu, 0xE64Du, 0xE64Eu, 0xE64Fu,
	0xE650u, 0xE651u, 0xE652u, 0xE653u, 0xE654u, 0xE655u, 0xE656u, 0xE657u, 0xE658u, 0xE659u, 0xE65Au, 0xE65Bu, 0xE65Cu, 0xE65Du, 0xE65Eu, 0x25A0u
};

/*
*  ETS 300 706 Table 45: Arabic G2 Supplementary Set
*
*  XXX 0X0000 is what?
*  Until these tables are finished use private codes U+E7xx.
*/
static const unsigned short arabic_g2[96] =
 {
	0xE660u, 0xE661u, 0xE662u, 0xE663u, 0xE664u, 0xE665u, 0xE666u, 0xE667u, 0xE668u, 0xE669u, 0xE66Au, 0xE66Bu, 0xE66Cu, 0xE66Du, 0xE66Eu, 0xE66Fu,
	0xE670u, 0xE671u, 0xE672u, 0xE673u, 0xE674u, 0xE675u, 0xE676u, 0xE677u, 0xE678u, 0xE679u, 0xE67Au, 0xE67Bu, 0xE67Cu, 0xE67Du, 0xE67Eu, 0xE67Fu,
	0x00E0u, 0x0041u, 0x0042u, 0x0043u, 0x0044u, 0x0045u, 0x0046u, 0x0047u, 0x0048u, 0x0049u, 0x004Au, 0x004Bu, 0x004Cu, 0x004Du, 0x004Eu, 0x004Fu,
	0x0050u, 0x0051u, 0x0052u, 0x0053u, 0x0054u, 0x0055u, 0x0056u, 0x0057u, 0x0058u, 0x0059u, 0x005Au, 0x00EBu, 0x00EAu, 0x00F9u, 0x00EEu, 0xE75Fu,
	0x00E9u, 0x0061u, 0x0062u, 0x0063u, 0x0064u, 0x0065u, 0x0066u, 0x0067u, 0x0068u, 0x0069u, 0x006Au, 0x006Bu, 0x006Cu, 0x006Du, 0x006Eu, 0x006Fu,
	0x0070u, 0x0071u, 0x0072u, 0x0073u, 0x0074u, 0x0075u, 0x0076u, 0x0077u, 0x0078u, 0x0079u, 0x007Au, 0x00E2u, 0x00F4u, 0x00FBu, 0x00E7u, 0x0020u
};

/*
*  ETS 300 706 Table 46: Hebrew G0 Primary Set
*/
static const unsigned short hebrew_g0[37] =
 {
	0x2190u, 0x00BDu, 0x2192u, 0x2191u, 0x0023u,
	0x05D0u, 0x05D1u, 0x05D2u, 0x05D3u, 0x05D4u, 0x05D5u, 0x05D6u, 0x05D7u, 0x05D8u, 0x05D9u, 0x05DAu, 0x05DBu, 0x05DCu, 0x05DDu, 0x05DEu, 0x05DFu,
	0x05E0u, 0x05E1u, 0x05E2u, 0x05E3u, 0x05E4u, 0x05E5u, 0x05E6u, 0x05E7u, 0x05E8u, 0x05E9u, 0x05EAu, 0x20AAu, 0x2016u, 0x00BEu, 0x00F7u, 0x25A0u
};

// unham 2 bytes into 1
__inline unsigned char unham(unsigned char low,unsigned char high) { return (unhamtab[high] << 4) | (unhamtab[low] & 0x0F); }

// unham 2 nibbles into 1 nibble
__inline unsigned char unham(unsigned char low) { return (unhamtab[low] & 0x0F); }

static int hamm24(BYTE *p, int *err)
{
    int e = hamm24par[0][p[0]] ^ hamm24par[1][p[1]] ^ hamm24par[2][p[2]];
    int x = hamm24val[p[0]] + p[1] % 128 * 16 + p[2] % 128 * 2048;

    *err |= hamm24err[e];
    return x ^ hamm24cor[e];
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyDVBTeletextPage::CMyDVBTeletextPage()
{
	Initial();
}

CMyDVBTeletextPage::~CMyDVBTeletextPage()
{
#ifdef __TELETEXT_HAS_CDC__
	for(int i=0; i<OUTPUT_IMAGE_HEIGH;i++)
	{
		if( m_apImageBufferForCDC[i] )
			delete m_apImageBufferForCDC[i];
	}


#endif // #ifdef __TELETEXT_HAS_CDC__

	Invalidate();
}

///-------------------------------------------------------
/// CYJ,2010-3-30
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyDVBTeletextPage::Invalidate()
{
	int nCount = m_aPageInfo.GetSize();
	for(int i=0; i<nCount; i++ )
	{
		PAGE_INFO & PageInfo = m_aPageInfo[i];
		if( PageInfo.m_pX26Data )
			delete PageInfo.m_pX26Data;
		if( PageInfo.m_pRawData )
			delete PageInfo.m_pRawData;
	}
	m_aPageInfo.RemoveAll();
}

///-------------------------------------------------------
/// CYJ,2010-3-30
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyDVBTeletextPage::Initial()
{
	Invalidate();

	memset( &m_FormattedPage,0,sizeof(m_FormattedPage) );	// 格式化过的页面

	m_bValid = false;
	m_bDoReceiveData = false;

	m_nLanguage = 0;

	m_byFastLinkDisplayCtrl = 0;	//  CYJ,2010-3-5 显示FastLink
	memset( m_abyFastLinkPageNo, 0, sizeof(m_abyFastLinkPageNo) );

	m_bShow = false;

#ifdef __TELETEXT_HAS_CDC__
	memset( m_apImageBufferForCDC, 0, sizeof(m_apImageBufferForCDC));
#endif // #ifdef __TELETEXT_HAS_CDC__
}

///-------------------------------------------------------
/// CYJ,2006-6-22
/// 函数功能:
///		接收到数据
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyDVBTeletextPage::OnPESReceived(PBYTE pData, int nDataLen, int nOffset, int nErrorTimes )
{
	if( *pData < 0x10 || *pData > 0x1F )
		return;				// 不是 EBU data，不处理
	pData ++;
	nDataLen --;

#ifdef _DEBUG
	ASSERT( (nDataLen % 46) == 0 );
#endif // _DEBUG

	int nCount = nDataLen/46;
	for(int i=0; i<nCount; i++)
	{
		SetOneLine( pData );

		pData += 46;
#ifdef _DEBUG
		nDataLen -= 46;
#endif //_DEBUG
	}

#ifdef _DEBUG
	ASSERT( 0 == nDataLen );
#endif //_DEBUG
}

///-------------------------------------------------------
/// CYJ,2006-6-22
/// 函数功能:
///		翻转比特顺序
/// 输入参数:
///		无
/// 返回参数:
///		无
inline void DVB_TT_InvBytes( PBYTE pSrc, PBYTE pDst, int nBytes )
{
	for(int i=0; i<nBytes; i++)
	{
		*pDst ++ = invtab[ *pSrc ++ ];
	}
}

inline void DVB_TT_InvBytesAnd0x7F( PBYTE pSrc, PBYTE pDst, int nBytes )
{
	for(int i=0; i<nBytes; i++)
	{
		*pDst ++ = invtab[ *pSrc ++ ] & 0x7F;
	}
}

///-------------------------------------------------------
/// ZDD,2010-3-27
/// 函数功能:
///		获取National sub set
/// 输入参数:
///		控制字符C14C13C12
/// 返回参数:
///		National sub-set
int	CMyDVBTeletextPage::GetNationSubSet( BYTE byControl )
{
	int nNation = NO_SUBSET;
	switch( byControl )
	{
	case 0x00:
		nNation = ENGLISH;
		break;
	case 0x01:
		nNation = FRENCH;
		break;
	case 0x02:
		nNation = SWE_FIN_HUN;
		break;
	case 0x03:
		nNation = CZECH_SLOVAK;
		break;
	case 0x04:
		nNation = GERMAN;
		break;
	case 0x05:
		nNation = PORTUG_SPANISH;
		break;
	case 0x06:
		nNation = ITALIAN;
		break;
	case 0x07:
	default:
		break;
	}
	return nNation;
}

///-------------------------------------------------------
/// CYJ,2006-6-22
/// 函数功能:
///		设置一行数据
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyDVBTeletextPage::SetOneLine(PBYTE pData)
{
	if( *pData < 2 || *pData > 3 )
		return;			// 目前只支持 2、3，即 EBU Teletext non-subtitle data, EBU teletext subtitle data

#ifdef _DEBUG
	ASSERT( pData[1] == 0x2C );
	ASSERT( pData[3] == 0xE4 );		// 0x27
#endif //_DEBUG

	unsigned char mpag = unham( invtab[ pData[4] ], invtab[ pData[5] ]);
	int nMagNo = mpag & 7;
	if( 0 == nMagNo )
		nMagNo = 8;

	unsigned char nPacketNo = (mpag >> 3) & 0x1f;

	pData += 6;
	BYTE bufTmp[VBI_WIDTH];
	if( 0 == nPacketNo )
	{
		DVB_TT_InvBytes( pData, bufTmp, 40 );
		m_nMagNo = nMagNo;
		DecodePacket_0( bufTmp );	//	解析X0包
		BYTE byControl = ( unham( bufTmp[7] ) >> 1 ) & 0x07;//控制位C14C13C12

		PPAGE_INFO pPage = GetPageInfoPtr( m_nPageNo );
		if( NULL == pPage )
			return;

		pPage->m_nNationSubSet = GetNationSubSet( byControl );
		memcpy( pPage->m_pRawData->m_abyRawData, bufTmp, VBI_WIDTH );
	}
	else if( m_nMagNo == nMagNo && m_bDoReceiveData )
	{
		PPAGE_INFO pPage = GetPageInfoPtr( m_nPageNo );
		if( NULL == pPage )
			return;

		if( nPacketNo <= 24 )
		{
			DVB_TT_InvBytesAnd0x7F( pData, bufTmp, 40 );
			memcpy( pPage->m_pRawData->m_abyRawData[nPacketNo], bufTmp, VBI_WIDTH );
		}
		else if( nPacketNo == 26 )
		{
			DVB_TT_InvBytes( pData, bufTmp, 40 );
			Decode_X26( bufTmp );
		}
	}
}

///-------------------------------------------------------
/// CYJ,2006-6-22
/// 函数功能:
///		解 0 包
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyDVBTeletextPage::DecodePacket_0(PBYTE data)
{
   // Using this buffer to start a brand new page.
	BYTE byPageNo =  unham(data[0], data[1]); // The lower two (hex) numbers of page
	if( byPageNo > 0x99 )
	{
		m_bDoReceiveData = false;
		return;
	}

	if( m_bValid )
		OnTeletextPageReceived( m_nMagNo, m_nPageNo & 0xFF, 0 );

	m_byFastLinkDisplayCtrl = 0;	//  CYJ,2010-3-5 显示FastLink

	m_bDoReceiveData = true;
	m_nPageNo = (m_nMagNo << 8) | byPageNo;

	PPAGE_INFO pPageInfo = GetPageInfoPtr( m_nPageNo );
	if( NULL == pPageInfo )
	{	// not exist
		PAGE_INFO NewPage;
		memset( &NewPage, 0, sizeof(PAGE_INFO) );
		NewPage.m_nPageNo = m_nPageNo;
		NewPage.m_pRawData = new ONE_PAGE_RAWDATA;
		if( NULL == NewPage.m_pRawData )
		{
			fprintf( stderr, "Failed to allocate raw data memory for teletext page.\n" );
			return;		//
		}
		memset( NewPage.m_pRawData, 0, sizeof(ONE_PAGE_RAWDATA) );

		m_aPageInfo.Add( NewPage );
	}

    m_nSubPage = ((unham(data[4], data[5]) << 8) | unham(data[2], data[3])) & 0x3F7F;
	m_nLanguage = ((unham(data[6], data[7]) >> 5) & 0x07);

    m_bErasePage = (data[3] & 0x80) == 0x80;		// Byte 9,  bit 8
    m_bNewsflash = (data[5] & 0x20) == 0x20;		// Byte 11, bit 6
    m_bSubtitle = (data[5] & 0x80) == 0x80;			// Byte 11, bit 8
    m_bSupressHeader = (data[6] & 0x02) == 0x02;	// Byte 12, bit 2
    m_bUpdateIndicator = (data[6] & 0x08) == 0x08;	// Byte 12, bit 4
    m_bInterruptedSequence = (data[6] & 0x20) == 0x20; // Byte 12, bit 6
    m_bInhibitDisplay = (data[6] & 0x80) == 0x80;	// Byte 12, bit 8
    m_bMagazineSerial = (data[7] & 0x02) == 0x02;	// Byte 13, bit 2

	m_bValid = true;
}

///-------------------------------------------------------
/// ZDD,2010-3-17
/// 函数功能:
///		Translate Unicode character to glyph number in wstfont2 image.
/// 输入参数:
///		c:	Unicode
///		italic:	TRUE to switch to slanted character set (doesn't affect
///          Hebrew and Arabic). If this is a G1 block graphic character
///          switch to separated block mosaic set.
/// 返回参数:
///		Glyph number
unsigned int CMyDVBTeletextPage::unicode_wstfont2(unsigned int c, int italic)
{
	static const unsigned short specials[] =
	{
		0x01B5, 0x2016, 0x01CD, 0x01CE, 0x0229, 0x0251, 0x02DD, 0x02C6,
		0x02C7, 0x02C9, 0x02CA, 0x02CB, 0x02CD, 0x02CF, 0x02D8, 0x02D9,
		0x02DA, 0x02DB, 0x02DC, 0x2014, 0x2018, 0x2019, 0x201C,	0x201D,
		0x20A0, 0x2030, 0x20AA, 0x2122, 0x2126, 0x215B, 0x215C, 0x215D,
		0x215E, 0x2190, 0x2191, 0x2192, 0x2193, 0x25A0, 0x266A, 0xE800,
		0xE75F
	};

	const unsigned int invalid = 357;
	unsigned int i;

	if( c < 0x0180 )
	{
		if( c < 0x0080 )
		{
			if( c < 0x0020 )
				return invalid;
			else /* %3 Basic Latin (ASCII) 0x0020 ... 0x007F */
				c = c - 0x0020 + 0 * 32;
		}
		else if( c < 0x00A0 )
			return invalid;
		else /* %3 Latin-1 Supplement, Latin Extended-A 0x00A0 ... 0x017F */
			c = c - 0x00A0 + 3 * 32;
	}
	else if( c < 0xEE00 )
	{
		if (c < 0x0460)
		{
			if (c < 0x03D0)
			{
				if (c < 0x0370)
					goto special;
				else /* %5 Greek 0x0370 ... 0x03CF */
					c = c - 0x0370 + 12 * 32;
			}
			else if (c < 0x0400)
				return invalid;
			else /* %5 Cyrillic 0x0400 ... 0x045F */
				c = c - 0x0400 + 15 * 32;
		}
		else if (c < 0x0620)
		{
			if (c < 0x05F0)
			{
				if (c < 0x05D0)
					return invalid;
				else /* %6 Hebrew 0x05D0 ... 0x05EF */
					return c - 0x05D0 + 18 * 32;
			} else if (c < 0x0600)
				return invalid;
			else /* %6 Arabic 0x0600 ... 0x061F */
				return c - 0x0600 + 19 * 32;
		}
		else if (c >= 0xE600 && c < 0xE740)
			return c - 0xE600 + 19 * 32; /* %6 Arabic (TTX) */
		else
			goto special;
	}
	else if (c < 0xEF00)
	{ /* %3 G1 Graphics */
		return (c ^ 0x20) - 0xEE00 + 23 * 32;
	}
	else if (c < 0xF000)
	{ /* %4 G3 Graphics */
		return c - 0xEF20 + 27 * 32;
	}
	else /* 0xF000 ... 0xF7FF reserved for DRCS */
		return invalid;

	if (italic)
		return c + 31 * 32;
	else
		return c;
special:
	for (i = 0; i < sizeof(specials) / sizeof(specials[0]); i++)
	{
		if (specials[i] == c)
		{
			if (italic)
				return i + 41 * 32;
			else
				return i + 10 * 32;
		}
	}
	return invalid;
}

///-------------------------------------------------------
/// ZDD,2010-3-17
/// 函数功能:
///		Translate Teletext character code to Unicode.
/// 输入参数:
///     s:	Teletext character set as listed in ETS 300 706 section 15
///		n:	National character subset as listed in section 15, only
///			applicable to character set LATIN_G0, ignored otherwise.
///		c:	Character code in range 0x20 ... 0x7F
/// 返回参数:
///		unicode值
unsigned int CMyDVBTeletextPage::vbi_teletext_unicode(int s, int n, unsigned int c)
{
	int i;

//	ASSERT(c >= 0x20 && c <= 0x7F);
	switch (s)
	{
	case LATIN_G0:
		/* shortcut */
		if (0xF8000019UL & (1 << (c & 31)))
		{
			if (n > 0)
			{
			#ifdef _DEBUG
				ASSERT(n < 14);
			#endif // #ifdef _DEBUG

				for (i = 0; i < 13; i++)
					if (c == national_subset[0][i])
						return national_subset[n][i];
			}

			if (c == 0x24)
				return 0x00A4u;
			else if (c == 0x7C)
				return 0x00A6u;
			else if (c == 0x7F)
				return 0x25A0u;
		}

		return c;

	case LATIN_G2:
		return latin_g2[c - 0x20];

	case CYRILLIC_1_G0:
		if (c < 0x40)
			return c;
		else
			return cyrillic_1_g0[c - 0x40];

	case CYRILLIC_2_G0:
		if (c == 0x26)
			return 0x044Bu;
		else if (c < 0x40)
			return c;
		else
			return cyrillic_2_g0[c - 0x40];

	case CYRILLIC_3_G0:
		if (c == 0x26)
			return 0x00EFu;
		else if (c < 0x40)
			return c;
		else
			return cyrillic_3_g0[c - 0x40];

	case CYRILLIC_G2:
		return cyrillic_g2[c - 0x20];

	case GREEK_G0:
		if (c == 0x3C)
			return 0x00ABu;
		else if (c == 0x3E)
			return 0x00BBu;
		else if (c < 0x40)
			return c;
		else
			return greek_g0[c - 0x40];

	case GREEK_G2:
		return greek_g2[c - 0x20];

	case ARABIC_G0:
		return arabic_g0[c - 0x20];

	case ARABIC_G2:
		return arabic_g2[c - 0x20];

	case HEBREW_G0:
		if (c < 0x5B)
			return c;
		else
			return hebrew_g0[c - 0x5B];

	case BLOCK_MOSAIC_G1:
		/* 0x20 ... 0x3F -> 0xEE00 ... 0xEE1F separated */
		/*                  0xEE20 ... 0xEE3F contiguous */
		/* 0x60 ... 0x7F -> 0xEE40 ... 0xEE5F separated */
		/*                  0xEE60 ... 0xEE7F contiguous */
	#ifdef _DEBUG
		ASSERT(c < 0x40 || c >= 0x60);
	#endif // #ifdef _DEBUG
		return 0xEE00u + c;

	case SMOOTH_MOSAIC_G3:
		return 0xEF00u + c;

	default:
		fprintf( stderr, "vbi_teletext_unicode, failed.\n" );
		return 0;
	}
}


///-------------------------------------------------------
/// ZDD,2010-3-25
/// 函数功能:
///		格式化页面
/// 输入参数:
///		nPageNo			the page No.  low 16 bits is the page No.
///						if bit30 = 1 then bit29,28 is the digital count
///										  bit16 ~ bit27 is the page no to show
///										  Bit16 ~ Bit23 is the PageNo
///										  Bit24 ~ Bit27 is the MageNo
///		nHeight		 format height
/// 返回参数:
///		无
CMyDVBTeletextPage::PPAGE_INFO CMyDVBTeletextPage::FormatPage( int nPageNo, int nHeight )
{
	int nDigitalCount = 0;
	int nPageNoInput;
	if( nPageNo & (1<<30) )
	{
		nDigitalCount = (nPageNo >> 28) & 3;
		nPageNoInput = ( nPageNo >> 16 ) & 0xFFF;
	}
	else
		nPageNoInput = nPageNo & 0xFFF;
	nPageNo &= 0xFFF;
#ifdef _DEBUG
	fprintf( stderr, "FormatPage: PageNo=%x, Digital=%d, PageNoInput=%x\n",\
		nPageNo, nDigitalCount, nPageNoInput );
#endif //_DEBUG

	PPAGE_INFO pPage = GetPageInfoPtr( nPageNo );
	if( NULL == pPage )
	{
		pPage = GetPageInfoPtr( m_nPageNo );
		if( NULL == pPage )
			return NULL;
		nHeight = 1;
	}
	PPAGE_INFO pCurPage = GetPageInfoPtr( m_nPageNo );
	if( pCurPage && pCurPage->m_pRawData )	//  CYJ,2010-3-30 总是显示最新的时间
		memcpy( pPage->m_pRawData->m_abyRawData[0]+32, pCurPage->m_pRawData->m_abyRawData[0]+32, 8 );

	char buf[16];
    int x, y;

	memset( &m_FormattedPage, 0, sizeof(m_FormattedPage) );
	if( 0 == nDigitalCount || nDigitalCount >=3 )
		sprintf(buf, "\2P%03X\7", nPageNoInput );
	else if( 1 == nDigitalCount )
		sprintf(buf, "\2P%01X  \7", nPageNoInput );
	else if( 2 == nDigitalCount )
		sprintf(buf, "\2P%02X \7", nPageNoInput );

	for(y = 0; y < nHeight; y++)
    {
		BYTE *p = pPage->m_pRawData->m_abyRawData[y];
		VBI_FORMATTED_CHAR c;
		int last_ch = ' ';
		int hold = 0;

		c.fg = 7;
		c.bg = 0;
		c.attr = 0;
		for(x = 0; x < VBI_WIDTH; x++)
		{
			c.ch = *p++;
			if (y == 0 && x < 7 )		// 7 => strlen("\2P%03X\7")
				c.ch = buf[x];

			switch (c.ch)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:/* alpha + fg color */
				c.fg = c.ch & 7;
				c.attr &= ~(EA_GRAPHIC | EA_SEPARATED | EA_CONCEALED);
				goto ctrl;
			case 0x08:		/* flash */
				c.attr |= EA_BLINK;
				goto ctrl;
			case 0x09:		/* steady */
				c.attr &= ~EA_BLINK;
				goto ctrl;
			case 0x0a:		/* end box */
				c.attr &= (~EA_BOX);
			case 0x0b:		/* start box */
				c.attr |= EA_BOX;
				goto ctrl;
			case 0x0c:		/* normal height */
				c.attr &= (~EA_HDOUBLE);
				goto ctrl;
			case 0x0d:		/* double height */
				c.attr |= EA_HDOUBLE;
				//dbl = 1;
				goto ctrl;
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
			case 0x14:
			case 0x15:
			case 0x16:
			case 0x17:
				/* gfx + fg color */
				c.fg = c.ch & 7;
				c.attr |= EA_GRAPHIC;
				c.attr &= ~EA_CONCEALED;
				goto ctrl;
			case 0x18:		/* conceal */
				c.attr |= EA_CONCEALED;
				goto ctrl;
			case 0x19:		/* contiguous gfx */
				c.attr &= ~EA_SEPARATED;
				goto ctrl;
			case 0x1a:		/* separate gfx */
				c.attr |= EA_SEPARATED;
				goto ctrl;
			case 0x1c:		/* black bf */
				c.bg = 0;
				goto ctrl;
			case 0x1d:		/* new bg */
				c.bg = c.fg;
				goto ctrl;
			case 0x1e:		/* hold gfx */
				hold = 1;
				goto ctrl;
			case 0x1f:		/* release gfx */
				hold = 0;
				goto ctrl;

			case 0x0e:		/* SO */
			case 0x0f:		/* SI */
			case 0x1b:		/* ESC */
				c.ch = ' ';
				break;

ctrl:
				c.ch = ' ';
				if (hold && (c.attr & EA_GRAPHIC))
					c.ch = last_ch;
				break;
			}

			if(c.attr & EA_CONCEALED)		//  && (!e->reveal) )
				c.ch = ' ';
			if( c.attr & EA_GRAPHIC )
			{
				last_ch = c.ch;
				if(c.ch & 0x20)	//	c.ch(32~63, 96~127)
					c.ch += 0xEE00;	//G1 contiguous
			}

			if( 0 == y )	//	页头排
				c.ch &= 0x7F;

			if( c.ch < 0xEE00  )
			{
				WORD unicode = vbi_teletext_unicode(1, pPage->m_nNationSubSet, c.ch);
				c.ch = unicode;
			}

			m_FormattedPage.m_abyData[y][x] = c;
		}
    }

	if( pPage->m_pX26Data )
		FormatPageByPacket26( pPage->m_pX26Data );

	return pPage;
}


///-------------------------------------------------------
/// ZDD,2010-3-19
/// 函数功能:
///		分析X26包中的并行属性
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyDVBTeletextPage::AnalyseParalAttr( BYTE byData, int x, int y )
{
	static int attrArray[7] =
	{
		EA_HDOUBLE, EA_BOX, EA_CONCEALED, EA_SEMIGRAY, EA_INVERSE,
		EA_UNDERLINE, EA_WDOUBLE
	};

	for( int i=0; i<7; i++ )
	{
		if( byData & attrArray[i] )
			m_FormattedPage.m_abyData[y][x].attr |= attrArray[i];
		else
			m_FormattedPage.m_abyData[y][x].attr &= (~attrArray[i]);
	}
}

///-------------------------------------------------------
/// CYJ,2010-3-30
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyDVBTeletextPage::FormatPageByPacket26( PX26_DATA pX26Data )
{
	int nDataUnhamm = 0;
	BYTE byAddress;
	BYTE byMode;
	BYTE byData;

	int nErr = 0;
	for( int i=0; i<15; i++ )
	{
		BYTE * p_i_X26 = pX26Data->m_abyX26Data[i];	//	X/i/26包数据
		if( 0 == *p_i_X26 )	//	X/i/26包未收到
			continue;

		p_i_X26++;	// 跳过标志字节

		int x = -1;
		int y = -1;
		for( int j=0; j<13; j++ )
		{
			nDataUnhamm = hamm24( p_i_X26, &nErr );
			byAddress  = nDataUnhamm & 0x3F;	//	6比特显示地址
			byMode = (nDataUnhamm >> 6) & 0x1F;	//	5比特模式描述
			byData = (nDataUnhamm >>11) & 0x7F;	//	7比特数据

			if( byAddress == 40 )
			{
				y = 24;
				x = -1;
			}
			else if( byAddress > 40 )
			{
				y = byAddress - 40;
				x= -1;
			}
			else if( byAddress < 40 )
			{
				if( -1 == y )
					continue;

				x = byAddress;
			}

			X26Attribute( byMode, byData, x, y );

			p_i_X26 += 3;
		}
	}
}

///-------------------------------------------------------
/// CYJ,2010-3-30
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyDVBTeletextPage::X26Attribute(BYTE byMode,BYTE byData, int x, int y)
{
	if( -1 == x )
	{	//	寻址到某一排
		switch( byMode )
		{
		case 0x00:
			{
				BYTE byTmp = (byData>>3) & 0x0f;
				if( 8 == byTmp )
				{//设定全页背景色
					for(int j=0; j<VBI_HEIGHT; j++)
					{
						for(int i=0; i<VBI_WIDTH; i++)
						{
							m_FormattedPage.m_abyData[j][i].bg = byData & 7;
						}
					}
				}
				else if( byTmp == 4 )
				{//设定全页前景色
					for(int j=0; j<VBI_HEIGHT; j++)
					{
						for(int i=0; i<VBI_WIDTH; i++)
						{
							m_FormattedPage.m_abyData[j][i].fg = byData & 7;
						}
					}
				}
				else if( byTmp == 0 )
				{//设定全屏色
					for(int j=0; j<VBI_HEIGHT; j++)
					{
						for(int i=0; i<VBI_WIDTH; i++)
						{
							m_FormattedPage.m_abyData[j][i].fg = byData & 7;
							m_FormattedPage.m_abyData[j][i].bg = byData & 7;
						}
					}
				}
			}
			break;

		case 0x01:	//	全排色
			{
				if( 0 != ((byData>>3) & 0x0f) )
					break;
				for(int i=0; i<VBI_WIDTH; i++)
				{
					m_FormattedPage.m_abyData[y][i].bg = byData & 7;
					m_FormattedPage.m_abyData[y][i].fg = byData & 7;
				}
			}
			break;

		case 0x1f:
		case 0x07:
		default:
			break;
		}
	}
	else
	{	//	寻址到某个字符
		switch( byMode )
		{
		case 0x00:	// 前景色
			if( 0 != byData>>3 )
				break;
			while( x < 40 )
			{
				m_FormattedPage.m_abyData[y][x].fg = byData & 7;
				x++;
			}
			break;

		case 0x01:	// 块状镶嵌字符集G1字符调用
			if( byData & 0x20 )	//	32~63, 96~127
				m_FormattedPage.m_abyData[y][x].ch = byData + 0xEE00;	//G1 contiguous
			else if( byData>=0x40 && byData<0x60 )
				m_FormattedPage.m_abyData[y][x].ch = vbi_teletext_unicode(1, NO_SUBSET, byData);
			break;

		case 0x02:	// 平滑镶嵌字符集G3字符调用
			if( byData >= 0x20 )
				m_FormattedPage.m_abyData[y][x].ch = byData + 0xEF00;
			break;

		case 0x03:	// 背景色
			if( 0 != byData>>3 )
				break;
			while( x < 40 )
			{
				m_FormattedPage.m_abyData[y][x].bg = byData & 7;
				x++;
			}
			break;

		case 0x04:	// 字符集调用，FIXME
			break;

		case 0x0c:	// 并行属性
			AnalyseParalAttr( byData, x, y );
			break;

		case 0x0f:	// 增补字符集，Ignore
			break;

		default:
			break;
		}
	}
}

void CMyDVBTeletextPage::Dump( int nPageNo, bool bDoFormat)
{
#ifdef _DEBUG
	if( bDoFormat )
		FormatPage( nPageNo );

	for(int y=0; y<VBI_HEIGHT; y++ )
	{
		char szTmp[VBI_WIDTH+2];
		for(int x=0; x<VBI_WIDTH; x ++ )
		{
			szTmp[x] = (BYTE)m_FormattedPage.m_abyData[y][x].ch;
		}
		szTmp[VBI_WIDTH] = '\n';
		szTmp[VBI_WIDTH+1] = 0;
		TRACE( szTmp );
	}
#else
	(void)nPageNo;
	(void)bDoFormat;
#endif //_DEBUG
}

///-------------------------------------------------------
/// CYJ,2006-6-23
/// 函数功能:
///		接收到一个页面
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyDVBTeletextPage::OnTeletextPageReceived(BYTE byMag, BYTE byPageNo,DWORD dwFlags)
{
}

///-------------------------------------------------------
/// CYJ,2006-6-23
/// 函数功能:
///		解释第26包
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyDVBTeletextPage::Decode_X26(PBYTE pData)
{
	PPAGE_INFO pPageInfo = GetPageInfoPtr( m_nPageNo );
	if( NULL == pPageInfo )
		return;

	if( NULL == pPageInfo->m_pX26Data )
	{
		pPageInfo->m_pX26Data = new X26_DATA;
		if( NULL == pPageInfo->m_pX26Data)
			return;
		memset( pPageInfo->m_pX26Data, 0, sizeof(X26_DATA) );
	}

	BYTE byDC = unham( *pData );
	byDC &= 0xF;
	if( 0x0F == byDC )
		return;			//	DC为1111的X/26包无定义

	pPageInfo->m_pX26Data->m_abyX26Data[byDC][0] = 1;
	memcpy( pPageInfo->m_pX26Data->m_abyX26Data[byDC] + 1, pData+1, 39 );
}

///-------------------------------------------------------
/// CYJ,2010-3-5
/// 函数功能:
///		解释第27行；只处理快选链接
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyDVBTeletextPage::Decode_27( PBYTE pData )
{
	BYTE byDC = unhamtab[ pData[2] ];
	if( byDC > 3 )
		return;			// 0 ~ 3 为快选链接

	m_byFastLinkDisplayCtrl = 0;
	for(int i=0; i<3; i++)
	{
		m_abyFastLinkPageNo[i] = unham( pData[3+i*6], pData[4+i*6] );	// The lower two (hex) numbers of page
		if( m_abyFastLinkPageNo[i] >= 0x99 )
			return;
	}

	m_byFastLinkDisplayCtrl = FASTLINK_CTRL_HAS_PAGE_NO;
	if( 0 == (unhamtab[ pData[39] ] & 8 ) )	// Bit3
		m_byFastLinkDisplayCtrl |= FASTLINK_CTRL_SHOW_PAGENO;			// 覆盖第24行，且改为显示页码

#ifdef _DEBUG
	TRACE( "CtrlFlag=0x%02X, PageNo=%d, %d, %d, %d\n",  m_byFastLinkDisplayCtrl,\
		m_abyFastLinkPageNo[0], m_abyFastLinkPageNo[1], m_abyFastLinkPageNo[2], m_abyFastLinkPageNo[3] );
#endif //_DEBUG
}

///-------------------------------------------------------
/// CYJ,2006-6-24
/// 函数功能:
///		输出一个字符
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyDVBTeletextPage::DrawChar( PBYTE pLines[], int y, int x, unsigned int unicode, BYTE byFontColor, BYTE byBgColor, WORD wAttr )
{
	int row = y/TT_FONT_HEIGHT;
	int column = x/TT_FONT_WIDTH;
	int i;

	WORD wTmpAttr = 0;
	if( row >= 1 )
		wTmpAttr = m_FormattedPage.m_abyData[row-1][column].attr;
	if( wTmpAttr & EA_HDOUBLE )
		return;

	wTmpAttr = 0;
	if( column >= 1 )
		wTmpAttr = m_FormattedPage.m_abyData[row][column-1].attr;
	if( wTmpAttr & EA_WDOUBLE )
	{
		m_bShow = !m_bShow;
		if( m_bShow == false )
		{
			if( wTmpAttr & EA_HDOUBLE )
			{
				for( int i=0; i<TT_FONT_HEIGHT; i++ )
					memcpy( pLines[y+i*2+1]+x, pLines[y+i*2]+x, TT_FONT_WIDTH );
				return;
			}
		}
	}

	if( unicode > 0xEF00 )
		unicode &= 0xEF7F;
	else if( unicode > 0xEE00 )
		unicode &= 0xEE7F;
// 	else if( unicode < 0xFF )
// 		unicode &= 0x7F;

	unsigned int nWstfontIndex = unicode_wstfont2( unicode, 0 );

	PBYTE pFontMask = g_abyTeletextFont_24x20 + nWstfontIndex * TELETEXT_FONT_BYTE_PER_FONTS;

	BYTE byTmpChar[TT_FONT_HEIGHT][TT_FONT_WIDTH];
	if( wAttr & EA_HDOUBLE )
	{
		for(i=0; i<TT_FONT_HEIGHT; i++)
		{
			memcpy( byTmpChar[i], pLines[y+i]+x, TT_FONT_WIDTH );
		}

		for( i=0; i<TT_FONT_HEIGHT; i++ )
		{
			memcpy( pLines[y+i*2], byTmpChar[i], TT_FONT_WIDTH  );
		}
	}

	for(i=0; i<TT_FONT_HEIGHT; i++)
	{
		PBYTE pImageBuf;
		if( wAttr & EA_HDOUBLE )			// 倍高
			pImageBuf = pLines[y+i*2]+x;
		else
			pImageBuf = pLines[y+i]+x;

		int xf = 0;
		for(int k=0; k<TT_FONT_WIDTH/8; k++ )
		{
			BYTE byMask = *pFontMask ++;
			for(int j=0; j<8; j++)
			{
				BYTE byColor;
				if( wAttr & EA_INVERSE )	//	反转
					byColor = (byMask & 0x80) ? byBgColor : byFontColor;
				else
					byColor = (byMask & 0x80) ? byFontColor : byBgColor;

				if( (i == TT_FONT_HEIGHT-1) && (wAttr & EA_UNDERLINE) ) //	下划线
					byColor = (wAttr & EA_INVERSE) ? byBgColor : byFontColor;

				if( (wAttr & EA_SEPARATED) && (wAttr & EA_GRAPHIC) )	//	分离镶嵌
				{
					if( (i==TT_FONT_HEIGHT-1) || (i==0) || (0==xf) || (TT_FONT_WIDTH-1==xf) )
						byColor = (wAttr & EA_INVERSE) ? byFontColor : byBgColor;
				}

				if( wAttr & EA_WDOUBLE )
				{//	倍宽
					pImageBuf[2*xf] = byColor;
					pImageBuf[2*xf+1] = byColor;
				}
				else
					pImageBuf[xf] = byColor;

				byMask <<= 1;
				xf ++;
			}
		}
		if( wAttr & EA_HDOUBLE )
		{	//	倍高
			int nBytes = TT_FONT_WIDTH;
			memcpy( pLines[y+i*2+1]+x, pLines[y+i*2]+x, nBytes );
		}
	}

}

///-------------------------------------------------------
/// CYJ,2006-6-24
/// 函数功能:
///		显示图像
/// 输入参数:
///		nPageNo			the page No.  low 16 bits is the page No.
///						if bit30 = 1 then bit29,28 is the digital count
///										  bit16 ~ bit27 is the page no to show
///										  Bit16 ~ Bit23 is the PageNo
///										  Bit24 ~ Bit27 is the MageNo
///		pLines			图像输出缓冲区, 缓冲区是 768 * 500
///		abyColors		颜色，分别为 背景色，黑，红，绿，黄，蓝，品红，青，白
///		bHeaderOnly		only draw header
/// 返回参数:
///		实际显示的字符行数
int CMyDVBTeletextPage::Draw(int nPageNo, PBYTE pLines[], BYTE abyColors[9], bool bHeaderOnly )
{
	PPAGE_INFO pPageInfo = FormatPage( nPageNo, bHeaderOnly ? 1 : VBI_HEIGHT );
	if( NULL == pPageInfo )
		return 0;
	nPageNo &= 0xFFF;

	int nHeight = ( bHeaderOnly || (pPageInfo->m_nPageNo != nPageNo) ) ? 1 : VBI_HEIGHT;
	for(int y=0; y<nHeight; y++ )
	{
		m_bShow = true;
		VBI_FORMATTED_CHAR * pFormatChar = m_FormattedPage.m_abyData[y];
		for(int x=0; x<VBI_WIDTH; x++ )
		{
			DrawChar( pLines, y*TT_FONT_HEIGHT, x*TT_FONT_WIDTH, \
				pFormatChar->ch, abyColors[pFormatChar->fg+1], abyColors[pFormatChar->bg+1],\
				pFormatChar->attr );
			pFormatChar ++;
		}
	}

	return nHeight;
}

#ifdef __TELETEXT_HAS_CDC__
void CMyDVBTeletextPage::Draw( int nPageNo, CDC * pDC )
{
	static BYTE abyColorIndex[]={0,1,2,3,4,5,6,7,8,9};
	static COLORREF aColorRef[]={
		RGB(0,0,0),		RGB(0,0,0),		RGB(255,0,0),	RGB(0,255,0),	RGB(255,255,0),
		RGB(0,0,255),	RGB(255,0,255),	RGB(0,255,255),	RGB(255,255,255)
	};
	if( NULL == m_apImageBufferForCDC[OUTPUT_IMAGE_HEIGH-1]  )
	{
		for(int i=0; i<OUTPUT_IMAGE_HEIGH;i++)
		{
			if( NULL == m_apImageBufferForCDC[i] )
				m_apImageBufferForCDC[i] = new BYTE[OUTPUT_IMGAGE_WIDTH];
			if( NULL == m_apImageBufferForCDC[i] )
				return;
		}
	}

	for(int i=0; i<OUTPUT_IMAGE_HEIGH;i++)
	{
		memset( m_apImageBufferForCDC[i], 0, OUTPUT_IMGAGE_WIDTH );
	}

	int nHeight = Draw( nPageNo, m_apImageBufferForCDC, abyColorIndex );
#ifdef _DEBUG
	ASSERT( nHeight >= 0 && nHeight <= VBI_HEIGHT );
#endif // #ifdef _DEBUG
	int nImageHeight = nHeight * TT_FONT_HEIGHT;
	for(int y=0; y<nImageHeight; y++)
	{
		PBYTE pImageBuf = m_apImageBufferForCDC[y];
		for( int x = 0; x<OUTPUT_IMGAGE_WIDTH; x++ )
		{
			pDC->SetPixel(x,y,aColorRef[ pImageBuf[x] ]);
		}
	}
}
#endif //_WIN32

///-------------------------------------------------------
/// CYJ,2010-3-30
/// 函数功能:
///		根据页面获取页面数据
/// 输入参数:
///		无
/// 返回参数:
///		无
CMyDVBTeletextPage::PPAGE_INFO CMyDVBTeletextPage::GetPageInfoPtr( int nPageNo )
{
	int nNo = m_aPageInfo.Find( nPageNo );
	if( nNo < 0 )
		return NULL;
	return &m_aPageInfo[nNo];
}

