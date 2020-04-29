///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2006-10-9
///
///		用途：
///			UDP/IP Over DVB 编码
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

// DVBPSI_IP_MPE_Generator.h: interface for the CDVBPSI_IP_MPE_Generator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DVBPSI_IP_MPE_GENERATOR_H__DBCF0883_B984_4176_A0FA_D6D547F7D778__INCLUDED_)
#define AFX_DVBPSI_IP_MPE_GENERATOR_H__DBCF0883_B984_4176_A0FA_D6D547F7D778__INCLUDED_

#include "psitablegenerator.h"

#pragma pack(push,4)

class CDVBPSI_IP_MPE_Generator : public CDVBPSI_TableGeneratorBase  
{
public:
	int Build( PBYTE pBuf, int nLen );
	CDVBPSI_IP_MPE_Generator( bool bChecksumIsCRC32 = false );
	virtual ~CDVBPSI_IP_MPE_Generator();

	enum
	{
		TSDVB_IP_MPE_SECTION_MAX_SIZE = 1600,		// 最大的长度
	};

	BYTE	m_abySectionBuf[TSDVB_IP_MPE_SECTION_MAX_SIZE];
	bool	m_bChecksumIsCRC32;					// true - CRC32 ; false - check sum
};

#pragma pack(pop)

#endif // !defined(AFX_DVBPSI_IP_MPE_GENERATOR_H__DBCF0883_B984_4176_A0FA_D6D547F7D778__INCLUDED_)
