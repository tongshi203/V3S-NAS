///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2006-9-21
///
///		用途：
///			IP Over DVB MPE decoder
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

// DVBPSITable_IP_MPE.cpp: implementation of the CDVBPSITable_IP_MPE class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "dvbpsitable_ip_mpe.h"
#include "dvbpsitablesdefine.h"
#include "dvb_crc.h"

#if defined(_DEBUG) && defined(_WIN32)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
#define __ENABLE_TRACE__
#endif //_DEBUG


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDVBPSITable_IP_MPE::CDVBPSITable_IP_MPE()
    :CDVBPSITablesBase(MPE_TABLE_MAX_SIZE),
      m_EthFrame( MPE_TABLE_MAX_SIZE )
{
    m_nDataHeaderLen = 20;		// 目前只考虑 IP 协议封装，一个IP头至少20字节
}

CDVBPSITable_IP_MPE::~CDVBPSITable_IP_MPE()
{

}

///-------------------------------------------------------
/// CYJ,2006-9-21
/// 函数功能:
///		接收到一个子表
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_IP_MPE::OnTableReceived()
{
    int nByteReceived = GetMemoryAllocated();
    ASSERT( nByteReceived );
    PBYTE pSrcBuf = GetHeapBuf();
    ASSERT( pSrcBuf );
    if( NULL == pSrcBuf || nByteReceived <= 12 )	// 一个PAT表至少需要12字节
        return;

    ASSERT( *pSrcBuf == DVBPSI_TBLID_DATABROCAST_MPE );	// 0x3E
    if( *pSrcBuf != DVBPSI_TBLID_DATABROCAST_MPE )
    {
#ifdef __ENABLE_TRACE__
        TRACE("CDVBPSITable_IP_MPE: Not a MPE private data table, tableId=%d != 0\n", *pSrcBuf );
#endif // __ENABLE_TRACE__
        return;						//	不是 MPE private data 表
    }
    WORD wSectionLen = ((pSrcBuf[1]&0xF) << 8) | pSrcBuf[2];
    if( nByteReceived < wSectionLen+3 )
    {
#ifdef __ENABLE_TRACE__
        TRACE("CDVBPSITable_IP_MPE: Bytes received %d < %d needed.\n", nByteReceived, wSectionLen );
#endif // __ENABLE_TRACE__
        return;
    }

    if( 0x30 != (pSrcBuf[1] & 0x30 ) )
    {		// reserved, should be '11'
#ifdef __ENABLE_TRACE__
        TRACE("CDVBPSITable_IP_MPE: section_syntax_indicator and next should be 2,but actual=%d\n", (pSrcBuf[1] & 0xC0 ) >> 6 );
#endif // __ENABLE_TRACE__
        return;
    }

    if( ( pSrcBuf[5] & 1 ) == 0 )
    {
#ifdef __ENABLE_TRACE__
        TRACE("CDVBPSITable_IP_MPE, current_next_indicator should be 1.\n");
#endif //__ENABLE_TRACE__
        return;
    }

    unsigned char bSectionSyntaxIndicator = ( pSrcBuf[1] & 0x80 );
    //	unsigned char bPrivateIndicator = ( pSrcBuf[1] & 0x40 );
    //	unsigned char byPayloadScramblingCtrl = ( pSrcBuf[5] >> 4 ) & 3;
    //	unsigned char byAddrScramblingCtrl = (pSrcBuf[5] >> 2 ) & 3;
    unsigned char bLLC_SNAP_Flag = (pSrcBuf[5] & 2);

    if( bLLC_SNAP_Flag )
    {
#ifdef __ENABLE_TRACE__
        TRACE("IP_MPE: This is a LLC_SNAP protocol, abort\n");
#endif  // __ENABLE_TRACE__
        return;
    }

    if( bSectionSyntaxIndicator )
    {
        if( DVB_GetCRC32( pSrcBuf, wSectionLen+3 ) )
        {							//	包括CRC32在内，整个结构大小＝wSectionLen＋3。
#ifdef __ENABLE_TRACE__
            TRACE("CDVBPSITable_IP_MPE: Bad CRC32\n");
#endif // __ENABLE_TRACE__
            return;
        }
    }
    else
    {					// check sum
        // not implement
    }

    m_abyEthAddr[5] = pSrcBuf[3];
    m_abyEthAddr[4] = pSrcBuf[4];
    for(int i=0; i<4; i++)
    {
        m_abyEthAddr[i] = pSrcBuf[11-i];
    }

    PBYTE pDataPtr = pSrcBuf + 12;
    WORD wDataLen = wSectionLen - 9 - 4;	// 4 bytes CRC32 or checksum, but 3 bytes header(table_id and senction len)

    if( 0 == pSrcBuf[6] )		// only one section
        OnEthernetFrame( m_abyEthAddr, pDataPtr, wDataLen );
    else
    {					// multiple section
        // 未经过调试，未知正确与否？故，设一个 ASSERT( FALSE );
        ASSERT( FALSE );
        if( 0 == m_EthFrame.GetMemoryAllocated() )
            SetSectionCount( pSrcBuf[7]+1 );	// 设置section个数
        SetSectionNoStatus( pSrcBuf[6] );
        if( false == m_EthFrame.Write( pDataPtr, wDataLen ) )
            Reset( true );			// write data failed
        if( m_EthFrame.GetMemoryAllocated() && IsAllSectionReceived() && m_EthFrame.GetHeapBuf() )
            OnEthernetFrame( m_abyEthAddr, m_EthFrame.GetHeapBuf(), m_EthFrame.GetMemoryAllocated() );
    }

    Reset( true );
}

///-------------------------------------------------------
/// CYJ,2006-9-21
/// 函数功能:
///		接收到一个IP数据
/// 输入参数:
///		pEthernetAddr					目标以太网地址
///		pIPPacket						IP 数据包
///		nIPLen							IP 长度
/// 返回参数:
///		无
void CDVBPSITable_IP_MPE::OnEthernetFrame( PBYTE pEthernetAddr, PBYTE pIPPacket, int nIPLen )
{
#ifdef __ENABLE_TRACE__
    TRACE( "IP over DVB, MAC:%02X-%02X-%02X-%02X-%02X-%02X, DstIP=%d.%d.%d.%d, %d Bytes\n",
           pEthernetAddr[0], pEthernetAddr[1], pEthernetAddr[2],
            pEthernetAddr[3], pEthernetAddr[4], pEthernetAddr[5],
            pIPPacket[16], pIPPacket[17], pIPPacket[18], pIPPacket[19],
            nIPLen );
#endif // __ENABLE_TRACE__
}


void CDVBPSITable_IP_MPE::Dump(FILE*fOutput)
{
#ifdef _DEBUG
    if( NULL == fOutput )
        fOutput = stderr;
    PBYTE pEthernetAddr = m_abyEthAddr;
    PBYTE pIPPacket = m_EthFrame.GetHeapBuf() + 12;
    int nIPLen = m_EthFrame.GetMemoryAllocated() - 12 - 4;
    if( NULL == pIPPacket || nIPLen < 20 )
    {
        fprintf( fOutput, "CDVBPSITable_IP_MPE::Dump, error data.\n" );
        return;
    }
    fprintf( fOutput, "IP over DVB, MAC:%02X-%02X-%02X-%02X-%02X-%02X\n",
             pEthernetAddr[0], pEthernetAddr[1], pEthernetAddr[2],
            pEthernetAddr[3], pEthernetAddr[4], pEthernetAddr[5] );
#else
    (void)fOutput;
#endif //_DEBUG
}

///-------------------------------------------------------
/// CYJ,2006-9-21
/// 函数功能:
///		获取上个表
/// 输入参数:
///		无
/// 返回参数:
///		无
PDVB_PSI_TABLE_BASE CDVBPSITable_IP_MPE::GetTableLastReceived()
{
    if( m_EthFrame.GetMemoryAllocated() <= m_nDataHeaderLen )	// 一个IP包，至少包含20字节的IP头
        return NULL;			// 没有收到
    return (PDVB_PSI_TABLE_BASE)m_EthFrame.GetHeapBuf();
}

///-------------------------------------------------------
/// CYJ,2006-9-21
/// 函数功能:
///		复位
/// 输入参数:
///		bForce				是否强制复位，缺省为true
/// 返回参数:
///		无
void CDVBPSITable_IP_MPE::Reset(bool bForce)
{
    CDVBPSITablesBase::Reset(bForce);
    if( bForce )
        m_EthFrame.Reset();
}

///-------------------------------------------------------
/// CYJ,2006-9-21
/// 函数功能:
///		是否有效
/// 输入参数:
///		无
/// 返回参数:
///		true			有效
///		false			失败
bool CDVBPSITable_IP_MPE::IsValid()
{
    if( false == CDVBPSITablesBase::IsValid() )
        return false;
    return m_EthFrame.IsValid();
}

///-------------------------------------------------------
/// CYJ,2006-9-21
/// 函数功能:
///		数据包是否完整
/// 输入参数:
///		无
/// 返回参数:
///		true			接收完整
///		false			未完整
bool CDVBPSITable_IP_MPE::IsPSIPacketIntegral()
{
    int nByteReceived = GetMemoryAllocated();
    ASSERT( nByteReceived );
    PBYTE pSrcBuf = GetHeapBuf();
    ASSERT( pSrcBuf );
    if( NULL == pSrcBuf || nByteReceived <= 12+m_nDataHeaderLen )	// 一个MPE表至少需要12字节，再加20字节的IP头
        return false;
    WORD wSectionLen = ((pSrcBuf[1]&0xf) << 8) | pSrcBuf[2];
    return( nByteReceived >= wSectionLen+3 );
}


