#ifndef __RAW_UDP_H__
#define __RAW_UDP_H__

#include <iostream>
#include <string>
#include <string.h>
#include "value.h"
using namespace std;
//ip 定义
typedef struct tagIPHDR
{
    unsigned char	m_cLenver;			// 4位首部长度+4位IP版本号
    unsigned char	m_cTos;				// 8位服务类型TOS;
    unsigned short	m_sTotal_len;		// 16位总长度(字节)
    unsigned short  m_sIdent;			// 16位标识
    unsigned short  m_sFrag_and_Flags;	// 3位标志位和13位报片偏移
    unsigned char   m_cTTL;				// 8位生存时间TTL
    unsigned char   m_cProto;			// 8位协议
    unsigned short  m_sCheckSum;		// 16位IP首部校验和
    unsigned int    m_nSourceIP;		// 32位源IP地址
    unsigned int    m_nDestIP;			// 32位目标IP地址
}IPHDR,*PIPHDR;

typedef struct tagUDPHDR
{
    unsigned short m_sSourcePort;		// 16位源端口
    unsigned short m_sDestPort;			// 16位目标端口
    unsigned short m_sLength;			// 16位数据长度
    unsigned short m_sCheckSum;			// 16位数据校验和
}UDPHDR,*PUDPHDR;

typedef struct tagEthernetHeader
{
    BYTE m_abyDestMACAddr[6];			// 6位目标MAC地址
    BYTE m_abySourceMACAddr[6];			// 6位源MAC地址
    BYTE m_abyType[2];					// 2字节表示类型(0800:IP 0806:ARP 6003:DECnet)
}ETHERNETHEADER,*PETHERNETHEADER;

class CRawUDP
{
public:
    CRawUDP();
    virtual ~CRawUDP();
    bool SetCreateRawIPParam(string strSourceIP,int nSourcePort, string strDestIP, int nDestPort);
    PBYTE GetRawIP(PBYTE pSrc,int nPayLoadLen,int& nPacketLen,WORD wIdent);
    PBYTE	m_pbyBuffer;				// 保存以太网帧头，udp头和IP头及数据的缓存
private:
    unsigned short CheckSum( unsigned short* pBuffer, int nSize);
    void FillIPHeader(WORD wIdent);
    void FillUDPHeader();
    void FillEthernetHeader();
    void CopyIPHeader();

private:
    IPHDR			 m_IPHdr;			// IP头
    UDPHDR			 m_UDPHdr;			// UDP头
    ETHERNETHEADER	 m_EthernetHeader;	// 以太网帧头

    int		m_nPayLoadLen;				// 有效数据字节数
    string m_strSourceIP;				// 源IP
    string m_strDestIP;				// 目标IP
    int     m_nSourcePort;				// 源端口
    int     m_nDestPort;				// 目标端口

    BYTE    m_abyMultiCastMAC[3];		// 保存多播MAC地址的低序23位
};

#endif // __RAW_UDP_H__
