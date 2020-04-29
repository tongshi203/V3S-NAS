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

// DVBPSI_IP_MPE_Generator.cpp: implementation of the CDVBPSI_IP_MPE_Generator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "dvbpsi_ip_mpe_generator.h"
#include "bitstream.h"
#include "dvbpsitablesdefine.h"
#include "dvb_crc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDVBPSI_IP_MPE_Generator::CDVBPSI_IP_MPE_Generator( bool bChecksumIsCRC32 )
{
	m_bChecksumIsCRC32 = bChecksumIsCRC32;
}

CDVBPSI_IP_MPE_Generator::~CDVBPSI_IP_MPE_Generator()
{

}

///-------------------------------------------------------
/// CYJ,2006-10-9
/// 函数功能:
///		编码
/// 输入参数:
///		pBuf				以太帧数据
///		nLen				长度
/// 返回参数:
///		>0					成功，分配的TS分组数
///		<0					失败
int CDVBPSI_IP_MPE_Generator::Build(PBYTE pBuf, int nLen)
{
	ASSERT( m_wPID != INVALID_PID );
	ASSERT( pBuf && nLen >= 14+20 );		// 14 bytes eth frame, 20 IP header

	if( m_wPID == INVALID_PID || NULL == pBuf || nLen < 14+20 )
		return false;

	PBYTE pEthHeader = pBuf;
	PBYTE pIPData = pBuf + 14;				// 跳过 EtherNet
	nLen -= 14;								// 除去 EtherNet

	int nSectionLen = nLen + 13;			// 会话层占用16字节，包括后面的4字节校验和；但头3个字节不算在内
	
	CMyBitStream bs( m_abySectionBuf, sizeof(m_abySectionBuf) );
	bs.PutBits8( DVBPSI_TBLID_DATABROCAST_MPE );

	bs.PutBit( m_bChecksumIsCRC32 ? 1 : 0 );	// '1' => CRC32, '0' => Checksum
	bs.PutBit( 1 );
	bs.PutBits( 3, 2 );						// '11', reserved
	bs.PutBits( nSectionLen, 12 );

	bs.PutBits8( pEthHeader[5] );
	bs.PutBits8( pEthHeader[4] );

	bs.PutBits( 3, 2 );						//	'11', reserved
	bs.PutBits( 0, 2 );						// Not Scrambled
	bs.PutBits( 0, 2 );						// Not Scrambled
	bs.PutBit( 0 );							// Not a LLC_SNAP
	bs.PutBit( 1 );							// Current Next indicator, should be '1'

	bs.PutBits8( 0 );						// section Number, 0 ==> only one section
	bs.PutBits8( 0 );						// last section number

	bs.PutBits8( pEthHeader[3] );
	bs.PutBits8( pEthHeader[2] );
	bs.PutBits8( pEthHeader[1] );
	bs.PutBits8( pEthHeader[0] );

	bs.FinishWrite();
	ASSERT( bs.GetTotalWriteBits() == 12*8 );

	memcpy( m_abySectionBuf+12, pIPData, nLen );

	PDWORD pdwCheckSum = (PDWORD)( m_abySectionBuf + 12 + nLen );

	*pdwCheckSum = 0;

	if( m_bChecksumIsCRC32 )		
	{
		DWORD dwCRC32 = DVB_GetCRC32( m_abySectionBuf, nSectionLen-4+3 );	// 3 bytes header, 4 bytes crc32
		*pdwCheckSum = SWAP_DWORD( dwCRC32 );
	}
	else
	{
		*pdwCheckSum = 0;			// 不知如何计算
	}

	return Encapsulate( m_abySectionBuf, nSectionLen+3 );
}


