// mydvbteletextpage.h: interface for the CMyDVBTeletextPage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYDVBTELETEXTPAGE_H__E0EA1B6F_D5D4_44B2_8EEF_F989331C825A__INCLUDED_)
#define AFX_MYDVBTELETEXTPAGE_H__E0EA1B6F_D5D4_44B2_8EEF_F989331C825A__INCLUDED_

#include <stdio.h>
#include "pespacket.h"
#include <MySortArray.h>

#ifdef _WIN32
	#define __TELETEXT_HAS_CDC__
#endif //_WIN32

#define __TEST__BYZDD

#pragma pack(push,4)

class CMyDVBTeletextPage
{
public:
	enum
	{
		VBI_WIDTH = 40,
		VBI_HEIGHT = 25,

		TT_FONT_WIDTH = 24,
		TT_FONT_HEIGHT = 20,

		OUTPUT_IMGAGE_WIDTH = TT_FONT_WIDTH*VBI_WIDTH,
		OUTPUT_IMAGE_HEIGH = VBI_HEIGHT*TT_FONT_HEIGHT,
	};
	enum
	{
		FASTLINK_CTRL_HAS_PAGE_NO = 1,		// Bit0 表示是否有PageNo
		FASTLINK_CTRL_SHOW_PAGENO = 2,		// Bit1 表示是否覆盖24行并显示PageNo
	};

	enum
	{ //Teletext character set according to ETS 300 706, Section 15.
		LATIN_G0 = 1,
		LATIN_G2,
		CYRILLIC_1_G0,
		CYRILLIC_2_G0,
		CYRILLIC_3_G0,
		CYRILLIC_G2,
		GREEK_G0,
		GREEK_G2,
		ARABIC_G0,
		ARABIC_G2,
		HEBREW_G0,
		BLOCK_MOSAIC_G1,
		SMOOTH_MOSAIC_G3
	};

	enum
	{ //Teletext Latin G0 national option subsets according to
		//ETS 300 706, Section 15.2; Section 15.6.2 Table 36.
		NO_SUBSET,
		CZECH_SLOVAK,
		ENGLISH,
		ESTONIAN,
		FRENCH,
		GERMAN,
		ITALIAN,
		LETT_LITH,
		POLISH,
		PORTUG_SPANISH,
		RUMANIAN,
		SERB_CRO_SLO,
		SWE_FIN_HUN,
		TURKISH
	};

	typedef struct tagVBI_FORMATTED_CHAR
	{
		WORD	ch;			// ASCII Code, Unicode
		BYTE	fg;			// 前景色
		BYTE	bg;			// 背景色
		WORD	attr;		// 属性，参见 EA_XXXX
	}VBI_FORMATTED_CHAR,*PVBI_FORMATTED_CHAR;

	typedef struct tagVBI_FORMATTED_PAGE
	{
		DWORD	m_dwDouble;			// 记录哪些行是加高
		DWORD	m_dwHID;
		VBI_FORMATTED_CHAR	m_abyData[VBI_HEIGHT][VBI_WIDTH];
	}VBI_FORMATTED_PAGE,*PVBI_FORMATTED_PAGE;


	typedef struct tagX26_DATA
	{
		BYTE m_abyX26Data[15][VBI_WIDTH];	//	X26包,每一行的第一个字节标志是否收到该包（m_abyX26Data[i][0] = 1：收到X/i/26包）
											//	X/26包的第4~42字节分成13个3字节的数据群,
											//	在实际传输时，先传X/0/26包，再传X/1/26包，直到X/14/26包
	}X26_DATA, *PX26_DATA;

	typedef struct tagONE_PAGE_RAWDATA
	{
		BYTE m_abyRawData[VBI_HEIGHT][VBI_WIDTH];	//	X0~X24包
	}ONE_PAGE_RAWDATA,*PONE_PAGE_RAWDATA;

	typedef struct tagPAGE_INFO
	{
		int m_nPageNo;
		int m_nNationSubSet;
		PONE_PAGE_RAWDATA	m_pRawData;
		PX26_DATA			m_pX26Data;
	}PAGE_INFO, *PPAGE_INFO;

///		nPageNo			the page No.  low 16 bits is the page No.
///						if bit30 = 1 then bit29,28 is the digital count
///										  bit16 ~ bit27 is the page no to show
///										  Bit16 ~ Bit23 is the PageNo
///										  Bit24 ~ Bit27 is the MageNo
	int Draw( int nPageNo, PBYTE pLines[], BYTE abyColors[9], bool bHeaderOnly );
	void DrawChar( PBYTE pLines[], int y, int x, unsigned int unicode, BYTE byFontColor, BYTE byBgColor, WORD wAttr );

#ifdef __TELETEXT_HAS_CDC__
	void Draw( int nPageNo, CDC * pDC );
#endif

	virtual void OnTeletextPageReceived(BYTE byMag, BYTE byPageNo,DWORD dwFlags);
	CMyDVBTeletextPage();
	virtual ~CMyDVBTeletextPage();

	virtual void OnPESReceived(PBYTE pData, int nDataLen, int nOffset, int nErrorTimes );

	void Initial();

	void Dump(int nPageNo, bool bDoFormat = true);

protected:
	VBI_FORMATTED_PAGE	m_FormattedPage;			// 格式化过的页面
	int m_nMagNo;
	int m_nPageNo;
	int m_nSubPage;
	int m_nLanguage;

    BOOL m_bErasePage;
    BOOL m_bNewsflash;
    BOOL m_bSubtitle;
    BOOL m_bSupressHeader;
    BOOL m_bUpdateIndicator;
    BOOL m_bInterruptedSequence;
    BOOL m_bInhibitDisplay;
    BOOL m_bMagazineSerial;

	bool m_bValid;
	bool m_bDoReceiveData;
//	bool m_bPageIsFormatted;		// 当前页面已经进行格式化

	BYTE m_byFastLinkDisplayCtrl;	// CYJ,2010-3-5 显示FastLink
	BYTE m_abyFastLinkPageNo[4];	// CYJ,2010-3-5 FastLink 页码

#ifdef __TELETEXT_HAS_CDC__
	PBYTE m_apImageBufferForCDC[OUTPUT_IMAGE_HEIGH];
#endif //#ifdef __TELETEXT_HAS_CDC__

private:
	void Decode_X26( PBYTE pData );
	void Decode_27( PBYTE pData );
	PPAGE_INFO FormatPage( int nPageNo, int nHeight = VBI_HEIGHT );
	void FormatPageByPacket26( PX26_DATA pX26Data );
	void X26Attribute(BYTE byMode, BYTE byData, int x, int y);
	void DecodePacket_0( PBYTE data );
	void SetOneLine( PBYTE pData );

	unsigned int unicode_wstfont2(unsigned int c, int italic);
	unsigned int vbi_teletext_unicode(int s, int n, unsigned int c);

	void	AnalyseParalAttr( BYTE byData, int x, int y );	//	分析并行属性
	int		GetNationSubSet( BYTE byControl );
	void    Invalidate();
	PPAGE_INFO GetPageInfoPtr( int nPageNo );

private:
	BOOL	m_bShow;
	CMySortArray<PAGE_INFO, 0, int> m_aPageInfo;
};

#pragma pack(pop)

#endif // !defined(AFX_MYDVBTELETEXTPAGE_H__E0EA1B6F_D5D4_44B2_8EEF_F989331C825A__INCLUDED_)

