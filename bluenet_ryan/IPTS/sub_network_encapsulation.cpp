#include "sub_network_encapsulation.h"
#include "bit_stream.h"
#include "raw_udp.h"
#include "dvb_crc.h"

#include <assert.h>

enum
{
    TSDVB_SNDU_SECTION_MAX_SIZE = 1600,		// 最大的长度
};

CSubNetworkEncapsulation::CSubNetworkEncapsulation(CUDPSend* pUDPSend):\
    m_pUDPSend(pUDPSend)
{
    memset( m_abyOutData, 0, sizeof(m_abyOutData) );
    m_nDataLen = 0;

    m_bChecksumIsCRC32 = true;
    m_wPID = INVALID_PID;
    m_byTSContinuity = 0;

    m_bIPv4 = true;
    m_bDestinationAddressAbentField = false;

    m_lpszRawUDPSrcIP = NULL;
    m_lpszRawUDPDstIP = NULL;
    m_wRawUDPSrcPort = 0;
    m_wRawUDPDstPort = 0;
}

CSubNetworkEncapsulation::~CSubNetworkEncapsulation()
{
}
///-------------------------------------------------------
/// wcl,2019-03-07
/// 函数功能:
///		设置PID
/// 输入参数:
///		wPID		PID 数值 for ts packet
/// 返回参数:
///		无
void CSubNetworkEncapsulation::SetPID( WORD wPID )
{
    m_wPID = wPID;
}
///-------------------------------------------------------
/// wcl,2019-07-03
/// 函数功能:
///		 初始化以太帧封装时的源端和目的端的ip port
/// 输入参数:
///		lpszSrcIP：源端ip
///     wSrcPort：源端port
///     lpszDstIP：目的端ip
///     wDstPort：目的端port
/// 输出参数:
///		无
void CSubNetworkEncapsulation::SetRawUDPIPPort(const char* lpszSrcIP, WORD wSrcPort, const char* lpszDstIP, WORD wDstPort)
{
    if( NULL == lpszSrcIP || 0 == *lpszSrcIP )
        return;
    if( NULL == lpszDstIP || 0 == *lpszDstIP )
        return;
    if( wSrcPort < 1024 || wDstPort < 1024 )
        return;

    m_lpszRawUDPSrcIP = lpszSrcIP;
    m_lpszRawUDPDstIP = lpszDstIP;
    m_wRawUDPSrcPort = wSrcPort;
    m_wRawUDPDstPort = wDstPort;
}

///-------------------------------------------------------
/// wcl,2019-07-03
/// 函数功能:
///		 设置ULE封装参数
/// 输入参数:
///		bIPv4： true IPv4 false：IPv6
///     bDestinationAddressAbentField：true/false
/// 输出参数:
///		无
void CSubNetworkEncapsulation::SetULEPars( bool bIPv4, bool bDestinationAddressAbentField )
{
    m_bIPv4 = bIPv4;
    m_bDestinationAddressAbentField = bDestinationAddressAbentField;
}
///-------------------------------------------------------
/// wcl,2019-03-07
/// 函数功能:
///		封装
/// 输入参数:
///		pBuf				数据
///		nLen				长度
/// 返回参数:
///		>0					成功，分配的TS分组数
///		<0					失败
int CSubNetworkEncapsulation::Encapsulation( PBYTE pBuf, int nLen )
{
    // 封装以太帧(14Bytes以太帧头 + 20BytesIP头 + 8BytesUDP头 + Data)
    CRawUDP rawUdp;
    rawUdp.SetCreateRawIPParam( m_lpszRawUDPSrcIP, m_wRawUDPSrcPort, m_lpszRawUDPDstIP, m_wRawUDPDstPort );

    WORD wIdent = 0;
    int nEtherFrameLen = 0;
    PBYTE pEtherFrameBuffer = rawUdp.GetRawIP( pBuf, nLen, nEtherFrameLen, wIdent );

#ifdef ULE
    return ULEEncapsulate( pEtherFrameBuffer, nEtherFrameLen );
#else
    return MPEEncapsulate( pEtherFrameBuffer, nEtherFrameLen );
#endif
}
///-------------------------------------------------------
/// wcl,2019-03-07
/// 函数功能:
///		MPE 封装
///     MPE header:      table id:                   8bits 固定为0x3e
///                      section_syntax_indicator:   1bit   '1' => CRC32, '0' => Checksum
///                      private_indicator:          1bit
///                      reserved:                   2bits  '11'
///                      length:                     12bits 以下的字节数（包括crc/checksum在内）
///                      mac6:                       8bits
///                      mac5:                       8bits
///                      reserved:                   2bits  '11
///                      payload_scrambling_control: 2bits
///                      address_scrambling_control: 2bits
///                      LCC_SNAP_Flag:              1bit
///                      Current_next_indicator:     1bit
///                      section_number:             8bits
///                      laset_section:              8bits
///                      mac4:                       8bits
///                      mac3:                       8bits
///                      mac2:                       8bits
///                      mac1:                       8bits
/// 输入参数:
///		pEthernetBuf		以太帧数据
///		nLen				长度
/// 返回参数:
///		>0					成功，分配的TS分组数
///		<0					失败
int CSubNetworkEncapsulation::MPEEncapsulate( PBYTE pEthernetBuf, int nLen )
{
    if( m_wPID == INVALID_PID || NULL == pEthernetBuf || nLen < 14+20+8 )
        return -1;

    PBYTE pEthHeader = pEthernetBuf;
    PBYTE pIPData = pEthernetBuf + 14;		// 跳过 EtherNet
    nLen -= 14;								// 除去 EtherNet

    int nSectionLen = nLen + 13;			// 会话层占用16字节，包括后面的4字节校验和；但头3个字节不算在内

    BYTE abySectionBuf[TSDVB_SNDU_SECTION_MAX_SIZE];
    CMyBitStream bs( abySectionBuf, sizeof(abySectionBuf) );
    bs.PutBits8( DVBPSI_TBLID_DATABROCAST_MPE );

    bs.PutBit( m_bChecksumIsCRC32 ? 1 : 0 );	// '1' => CRC32, '0' => Checksum
    bs.PutBit( 0 );
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
    assert( bs.GetTotalWriteBits() == 12*8 );

    memcpy( abySectionBuf+12, pIPData, nLen );

    PDWORD pdwCheckSum = (PDWORD)( abySectionBuf + 12 + nLen );

    *pdwCheckSum = 0;

    if( m_bChecksumIsCRC32 )
    {
        DWORD dwCRC32 = DVB_GetCRC32( abySectionBuf, nSectionLen-4+3 );	// 3 bytes header, 4 bytes crc32
        *pdwCheckSum = SWAP_DWORD( dwCRC32 );
    }
    else
    {
        *pdwCheckSum = 0;			// 不知如何计算
    }
    return TSEncapsulate( abySectionBuf, nSectionLen+3 );
}
///-------------------------------------------------------
/// wcl,2019-03-07
/// 函数功能:
///		ULE 封装
///     ULE header:      destionation(NPA) Address Absent:  1bit '1' => 无NPA address, '0' => 有NPA address
///                      length:                            15bits 以下的字节数（包括crc在内）
///                      type:                              16bits 0x0800 => IPv4 0x86dd => IPv6
///                if( destionation(NPA) Address Absent == '0' )
///                      NPA address:                       6Bytes
/// 输入参数:
///		pEthernetBuf		以太帧数据
///		nLen				长度
/// 返回参数:
///		>0					成功，分配的TS分组数
///		<0					失败
int CSubNetworkEncapsulation::ULEEncapsulate(PBYTE pEthernetBuf, int nLen)
{
    if( m_wPID == INVALID_PID || NULL == pEthernetBuf || nLen < 4+20+8 )
        return -1;

    PBYTE pEthHeader = pEthernetBuf;
    PBYTE pIPData = pEthernetBuf + 14;	// 跳过 EtherNet
    nLen -= 14;							// 除去 EtherNet
    int nSectionLen = nLen + 6;			// 会话层占用8字节，包括后面的4字节校验和；但头2个字节不算在内
    if( false == m_bDestinationAddressAbentField )
        nSectionLen += 6;               //  6Bytes Receiver Destination NPA Address

    BYTE abySectionBuf[TSDVB_SNDU_SECTION_MAX_SIZE];
    CMyBitStream bs( abySectionBuf, sizeof(abySectionBuf) );
    bs.PutBit( m_bDestinationAddressAbentField ? 1 : 0 );
    bs.PutBits( nSectionLen, 15 );
    bs.PutBits16( m_bIPv4? 0x0800:0x86dd );
    if( false == m_bDestinationAddressAbentField )
    {
        bs.PutBits8( pEthHeader[5] );
        bs.PutBits8( pEthHeader[4] );
        bs.PutBits8( pEthHeader[3] );
        bs.PutBits8( pEthHeader[2] );
        bs.PutBits8( pEthHeader[1] );
        bs.PutBits8( pEthHeader[0] );
    }
    bs.FinishWrite();
    int nULEHeaderLen = m_bDestinationAddressAbentField? 4: 10;
    assert( bs.GetTotalWriteBits() == nULEHeaderLen*8 );

    memcpy( abySectionBuf+nULEHeaderLen, pIPData, nLen );

    PDWORD pdwCheckSum = (PDWORD)( abySectionBuf + nULEHeaderLen + nLen );
    *pdwCheckSum = 0;
    DWORD dwCRC32 = DVB_GetCRC32( abySectionBuf, nSectionLen-4+2 );	// 2 bytes header, 4 bytes crc32
    *pdwCheckSum = SWAP_DWORD( dwCRC32 );

    return TSEncapsulate( abySectionBuf, nSectionLen+2 );
}

///-------------------------------------------------------
/// wcl,2019-03-07
/// 函数功能:
///		将数据封装成TS分组
/// 输入参数:
///		pBuf				table 缓冲区
///		nLen				table 大小
/// 返回参数:
///		>0					分配的个数
///		<0					失败
int CSubNetworkEncapsulation::TSEncapsulate( PBYTE pBuf, int nLen )
{
    assert( pBuf && nLen > 0 );

    if( !pBuf || nLen <=0 )
        return -1;

    int nPointerField = 1;
    int nRetVal = 0;
    BYTE byTSPacket[TS_PACKET_LEN] = {'0'};
    while( nLen > 0 )
    {
        byTSPacket[0] = 0x47;		// 同步字
        byTSPacket[1] = ((m_wPID>>8)&0x1F );
        byTSPacket[2] = (m_wPID&0xFF);
        byTSPacket[3] = 0x10 | (m_byTSContinuity++&0xF);
        int nPayloadLen = 184;
        int pos = 4;
        if( nPointerField )
        {
            byTSPacket[1] |= 0x40;
            byTSPacket[pos] = 0; //  pointer field
            pos++;
            nPayloadLen --;
            nPointerField = 0;
        }
        if( nPayloadLen > nLen )
        {
            memset(&byTSPacket[pos+nLen],0xFF, nPayloadLen-nLen);
            nPayloadLen = nLen;
        }
        memcpy( &byTSPacket[pos], pBuf, nPayloadLen );
        nLen -= nPayloadLen;
        pBuf += nPayloadLen;

        OnTSPacketReady( byTSPacket );
        nRetVal ++;
    }
    assert( 0 == nLen );

    return nRetVal;
}

///-------------------------------------------------------
/// wcl,2019-03-07
/// 函数功能:
///		一个完整的TS包封装完成，每7个ts包发一次udp
/// 输入参数:
///		无
/// 返回参数:
///		无

void CSubNetworkEncapsulation::OnTSPacketReady( PBYTE pPacket )
{
    memcpy( &m_abyOutData[m_nDataLen], pPacket, TS_PACKET_LEN );
    m_nDataLen += TS_PACKET_LEN;
    if( m_nDataLen < OUT_DATA_BUF_SIZE )
        return;

    if( m_pUDPSend )
    {
        m_pUDPSend->Send( m_abyOutData, OUT_DATA_BUF_SIZE );
    }

    m_nDataLen = 0;
}
