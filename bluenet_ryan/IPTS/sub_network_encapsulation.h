#ifndef __SUB_NETWORK_ENCAPSULATION_H_20190703__
#define __SUB_NETWORK_ENCAPSULATION_H_20190703__

#include "value.h"
#include "udp_send.h"

class CSubNetworkEncapsulation
{
public:
    CSubNetworkEncapsulation( CUDPSend* pUDPSend );
    ~CSubNetworkEncapsulation();

    void SetPID( WORD wPID );
    void SetRawUDPIPPort( const char* lpszSrcIP, WORD wSrcPort, const char* lpszDstIP, WORD wDstPort );
    void SetULEPars( bool bIPv4 = true, bool bDestinationAddressAbentField = false );

    int Encapsulation( PBYTE pBuf, int nLen );

private:
    int MPEEncapsulate( PBYTE pEthernetBuf, int nLen );
    int ULEEncapsulate( PBYTE pEthernetBuf, int nLen );
    int TSEncapsulate( PBYTE pBuf, int nLen );
    void OnTSPacketReady( PBYTE pPacket );

private:
    enum
    {
        OUT_DATA_BUF_SIZE = TS_PACKET_LEN*7,
    };
    BYTE        m_abyOutData[ OUT_DATA_BUF_SIZE ];
    int         m_nDataLen;

    // for ts packet encapsulation
    WORD	    m_wPID;
    BYTE	    m_byTSContinuity;

    // for mpe
    bool	    m_bChecksumIsCRC32;	// true - CRC32 ; false - check sum

    // for ule
    bool         m_bIPv4;
    bool         m_bDestinationAddressAbentField;

    // for ethernet frame
    const char*   m_lpszRawUDPSrcIP;
    WORD          m_wRawUDPSrcPort;
    const char*   m_lpszRawUDPDstIP;
    WORD          m_wRawUDPDstPort;

    // for send
    CUDPSend*    m_pUDPSend;
};

#endif // __SUB_NETWORK_ENCAPSULATION_H_20190703__
