#include "raw_udp.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int FAKEHEADERSIZE = 12;				// 计算UDP校验和的伪报头字节数

CRawUDP::CRawUDP()
{
    memset(&m_IPHdr,0,sizeof(m_IPHdr));
    memset(&m_UDPHdr,0, sizeof(m_UDPHdr));
    memset(&m_EthernetHeader,0,sizeof(m_EthernetHeader));
    m_pbyBuffer = NULL;
}

CRawUDP::~CRawUDP()
{
    if( NULL != m_pbyBuffer )
        delete []m_pbyBuffer;
}

///-------------------------------------------------------
/// 苗卫斌,2008-5-19
/// 函数功能:
///		设置创建虚拟IP地址，构造UDP包参数
/// 输入参数:
///     strSourceIP: 源IP
///     nSourcePort: 源Port
///     strDestIP: 目标IP
///     nDestPort: 目标Port
/// 返回参数:
///		TRUE: 成功 FALSE:失败
/// 说明：
///    为了把IP多投点地址映射到以太网的多投点地址，需要将IP多投点地址的低序23位放入特别的
///    以太网多投点地址01.00.5e.00.00.00(十六进制)的低序23位；
///    例如：224.0.0.1变成以太网多投点地址是：01.00.5e.00.00.01
bool CRawUDP::SetCreateRawIPParam( string strSourceIP,int nSourcePort, string strDestIP, int nDestPort)
{
    m_strSourceIP = strSourceIP;
    m_strDestIP = strDestIP;
    m_nSourcePort = nSourcePort;
    m_nDestPort = nDestPort;

    memset(m_abyMultiCastMAC,0,sizeof(m_abyMultiCastMAC));
    DWORD dwSwapMultiCastIP = SWAP_DWORD( inet_addr(m_strDestIP.data()))&0x7FFFFF;
    for(int i=2; i>=0; i-- )
    {
        m_abyMultiCastMAC[i] = (BYTE)(dwSwapMultiCastIP & 0xff);
        dwSwapMultiCastIP >>= 8;
    }
    return true;
}

///-------------------------------------------------------
/// 苗卫斌,2008-5-19
/// 函数功能:
///		计算校验和
/// 输入参数:
///		pBuffer: 需要计算校验和缓存地址
///     nSize：	缓存字节数
/// 返回参数:
///		校验和
/// 说明：计算IP头和UDP头校验和统一使用该函数
unsigned short CRawUDP::CheckSum( unsigned short* pBuffer, int nSize)
{
    unsigned long lcksum = 0;
    while( nSize > 1 )
    {
        lcksum += *pBuffer++;
        nSize -= sizeof(unsigned short);
    }

    if( nSize )
    {
        lcksum += *(unsigned char*)pBuffer;
    }

    lcksum = (lcksum >> 16) +(lcksum & 0xffff);
    lcksum += (lcksum >> 16);
    return (unsigned short)(~lcksum);
}

///-------------------------------------------------------
/// 苗卫斌,2008-5-19
/// 函数功能:
///		填充IP头
/// 输入参数:
///		无
/// 返回参数:
///		无
void CRawUDP::FillIPHeader(WORD wIdent )
{
    memset(&m_IPHdr,0,sizeof(m_IPHdr));
    m_IPHdr.m_cLenver = ( (4<<4)|sizeof(m_IPHdr)/ 4 );
    m_IPHdr.m_cTos = 0;
    // 需要转化为网络序字节
    m_IPHdr.m_sTotal_len = htons( sizeof(m_IPHdr) + sizeof(m_UDPHdr) + m_nPayLoadLen );
    m_IPHdr.m_sIdent = htons(wIdent);
    m_IPHdr.m_sFrag_and_Flags = 0;      // 在传输时，也需要转化为网络字节序
    m_IPHdr.m_cTTL = 8;
    m_IPHdr.m_cProto = IPPROTO_UDP;
    m_IPHdr.m_nSourceIP = inet_addr(m_strSourceIP.data());
    m_IPHdr.m_nDestIP = inet_addr(m_strDestIP.data());
    // 计算IP头校验和
    m_IPHdr.m_sCheckSum = CheckSum((unsigned short*)&m_IPHdr, sizeof(IPHDR));
}

///-------------------------------------------------------
/// 苗卫斌,2008-5-21
/// 函数功能:
///		拷贝IP头到缓存
/// 输入参数:
///		无
/// 返回参数:
///		无
///  注：由于在计算UDP头的校验和时要使用伪数据包头，使用了UDP头前面
///      12个字节作为计算UDP头校验和使用的伪数据包头的缓存，所以在计算
///      完UDP头的信息后，需要再将计算的IP头拷贝到指定的位置。
void CRawUDP::CopyIPHeader()
{
    // 将已生成的IPHeader填充到申请的缓存中
    memcpy( m_pbyBuffer + sizeof(ETHERNETHEADER), &m_IPHdr, sizeof(IPHDR));
}


///-------------------------------------------------------
/// 苗卫斌,2008-5-20
/// 函数功能:
///		填充UDP头
/// 输入参数:
///		无
/// 返回参数:
///		无
void CRawUDP::FillUDPHeader()
{
    memset(&m_UDPHdr,0,sizeof(m_UDPHdr));
    m_UDPHdr.m_sSourcePort = htons(m_nSourcePort);
    m_UDPHdr.m_sDestPort = htons(m_nDestPort);
    m_UDPHdr.m_sLength = htons( sizeof(m_UDPHdr) + m_nPayLoadLen );

    // 计算UDP校验和的方法：
    // 1.生成12个字节的伪报头；
    // 2.12个字节的伪报头后为真正的UDP头；
    // 3.真正UDP头的后面为实际有效的数据；

    // 根据上面的三项来计算UDP头的校验和！！！！！！
    // 计算UDP头校验和数值需要以上三项的内容。

    // 由于在调用GetRawIP函数时，首先会将有效数据拷贝到sizeof(ETHERNETHEADER) + sizeof(IPHDR) + sizeof(UDPHDR)
    // 后，即预先保留出以太网帧头及IP头和UDP头。
    PBYTE pTempBuf = m_pbyBuffer + sizeof(ETHERNETHEADER) + sizeof(IPHDR) - FAKEHEADERSIZE;
    PBYTE pbyCalCheckSum = pTempBuf;
    int nUDPCheckSumSize = 0;
    //////////////////////////////////////////////////////////////////////////
    // 以下为计算UDP校验和时使用的伪数据包头信息
    // 发送方IP地址
    int nTempSize = sizeof(m_IPHdr.m_nSourceIP);
    memcpy(pTempBuf,&m_IPHdr.m_nSourceIP,nTempSize);
    pTempBuf += nTempSize;
    nUDPCheckSumSize += nTempSize;

    // 接收方IP地址
    nTempSize = sizeof(m_IPHdr.m_nDestIP);
    memcpy(pTempBuf,&m_IPHdr.m_nDestIP,nTempSize);
    pTempBuf += nTempSize;
    nUDPCheckSumSize += nTempSize;

    // 伪数据包头中数据为0的字段
    pTempBuf++;
    nUDPCheckSumSize ++;

    // 协议标识符
    nTempSize = sizeof(m_IPHdr.m_cProto);
    memcpy( pTempBuf, &m_IPHdr.m_cProto, nTempSize );
    pTempBuf += nTempSize;
    nUDPCheckSumSize += nTempSize;

    // UDP长度
    nTempSize = sizeof( m_UDPHdr.m_sLength );
    memcpy( pTempBuf, &m_UDPHdr.m_sLength, nTempSize );
    pTempBuf += nTempSize;
    nUDPCheckSumSize += nTempSize;
    // 以上为计算UDP校验和时使用的伪数据包头信息
    //////////////////////////////////////////////////////////////////////////

    // UDP数据包头
    memcpy(pTempBuf, &m_UDPHdr, sizeof(m_UDPHdr));
    nUDPCheckSumSize += sizeof(m_UDPHdr);

    // 计算UDP校验和字节数 +　有效数据字节数
    nUDPCheckSumSize += m_nPayLoadLen;

    // 计算UDP校验和
    m_UDPHdr.m_sCheckSum = CheckSum((unsigned short*)pbyCalCheckSum,nUDPCheckSumSize);

    // 将已生成的UDPHeader填充到申请的缓存中
    memcpy( m_pbyBuffer + sizeof(ETHERNETHEADER) + sizeof(IPHDR), &m_UDPHdr, sizeof(m_UDPHdr));
}

///-------------------------------------------------------
/// 苗卫斌,2008-5-20
/// 函数功能:
///		填充以太网帧头
/// 输入参数:
///		无
/// 返回参数:
///		无
void CRawUDP::FillEthernetHeader( )
{
    // 目的计算机网卡MAC地址
    m_EthernetHeader.m_abyDestMACAddr[0] = 0x1;
    m_EthernetHeader.m_abyDestMACAddr[1] = 0x0;
    m_EthernetHeader.m_abyDestMACAddr[2] = 0x5E;
    m_EthernetHeader.m_abyDestMACAddr[3] = m_abyMultiCastMAC[0];
    m_EthernetHeader.m_abyDestMACAddr[4] = m_abyMultiCastMAC[1];
    m_EthernetHeader.m_abyDestMACAddr[5] = m_abyMultiCastMAC[2];

    // 源计算机网卡MAC地址
    m_EthernetHeader.m_abySourceMACAddr[0] = 0x0;
    m_EthernetHeader.m_abySourceMACAddr[1] = 0x50;
    m_EthernetHeader.m_abySourceMACAddr[2] = 0x18;
    m_EthernetHeader.m_abySourceMACAddr[3] = 0x23;
    m_EthernetHeader.m_abySourceMACAddr[4] = 0xC5;
    m_EthernetHeader.m_abySourceMACAddr[5] = 0x88;

    // 表示类型
    m_EthernetHeader.m_abyType[0] = 0x8;
    m_EthernetHeader.m_abyType[1] = 0x0;

    memcpy( m_pbyBuffer, &m_EthernetHeader, sizeof(ETHERNETHEADER));
}

///-------------------------------------------------------
/// 苗卫斌,2008-5-20
/// 函数功能:
///		获取已创建成功的缓存
/// 输入参数:
///     pSrc: 原始数据地址
///     nPayLoadLen: 原始数据字节数
///		nPakcetLen: 输出包括Ethernet头，IP头，UDP头及全部数据的总字节数
/// 返回参数:
///		返回已创建的缓存地址
PBYTE CRawUDP::GetRawIP(PBYTE pSrc,int nPayLoadLen,int& nPacketLen,WORD wIdent)
{
    int nHeaderLen = sizeof(ETHERNETHEADER) + sizeof(IPHDR) + sizeof(UDPHDR);
    m_nPayLoadLen = nPayLoadLen;

    // 返回总字节数
    nPacketLen = nHeaderLen + m_nPayLoadLen;

    if(  m_pbyBuffer != NULL )
    {
        delete []m_pbyBuffer;
        m_pbyBuffer = NULL;
    }

    m_pbyBuffer = new BYTE[ nHeaderLen + m_nPayLoadLen  ];
    if( NULL == m_pbyBuffer )
        return NULL;

    memset( m_pbyBuffer, 0, nHeaderLen + m_nPayLoadLen);
    // 将有效数据拷贝到申请成功的缓存中
    memcpy( m_pbyBuffer + nHeaderLen, pSrc, m_nPayLoadLen);

    FillIPHeader(wIdent);
    FillUDPHeader();
    CopyIPHeader();
    FillEthernetHeader();
    return m_pbyBuffer;
}
