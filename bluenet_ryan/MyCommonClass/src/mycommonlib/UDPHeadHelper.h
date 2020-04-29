// UDPHeadHelper.h: interface for the CUDPHeadHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UDPHEADHELPER_H__602C791B_E893_4D09_B17C_0407FBC518CC__INCLUDED_)
#define AFX_UDPHEADHELPER_H__602C791B_E893_4D09_B17C_0407FBC518CC__INCLUDED_

#pragma pack(push,4)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define XCHG(x)         (WORD)( BYTE(x)*0x100 + BYTE(x>>8) )
#define DXCHG(x)        (DWORD)( XCHG(x>>16) | ( XCHG(WORD(x))<<16 ) )

#ifdef _WIN32
	#include <protocol.h>
#else

#pragma pack(push,1)
    typedef BYTE ETHERNET_ADDRESS [6];

    typedef struct ETHERNET_FRAME
    {
        ETHERNET_ADDRESS	Destination;
        ETHERNET_ADDRESS	Source;
        WORD				FrameType;			// in host-order
    } ETHERNET_FRAME;

    #define	ETHERNET_FRAME_TYPE_IP		0x0800

    typedef struct IP_HEADER {
        BYTE    x;
        BYTE    tos;
        WORD    length;
        WORD    identifier;
    #define IP_MF 0x2000
        WORD    fragment;
        BYTE    ttl;
        BYTE    protocol;
        WORD    cksum;
        DWORD   src;
        DWORD   dest;
    } IP_HEADER;
    typedef IP_HEADER * LPIPHEADER;

    #define	IP_HEADER_MINIMUM_LEN	20

    typedef struct UDP_HEADER {
        WORD	src_port;
        WORD	dest_port;
        WORD	length;			// including this header
        WORD	checksum;
    } UDP_HEADER;

    #define	UDP_HEADER_LEN			8
    
#pragma pack(pop)

#endif // _WIN32

class CUDPHeadHelper
{
public:
	static WORD CheckSum( LPIPHEADER lpIPHeader );
	CUDPHeadHelper();
	virtual ~CUDPHeadHelper();
	static WORD CheckSum( WORD * buffer, int size);
	static WORD CheckSum( UDP_HEADER * pUDPHeader, DWORD dwSrcIP, DWORD dwDstIP );

	typedef struct tagUDPFAKEHEADER
	{
		DWORD m_dwSrcIP;
		DWORD m_dwDstIP;
		BYTE  m_byReserved_0;
		BYTE  m_byProtocol;
		WORD  m_wUDPLen;
		WORD  m_wSrcPort;
		WORD  m_wDstPort;
		WORD  m_wUDPLen1;
		WORD  m_wUDPChkSum;
	}UDPFAKEHEADER,*PUDPFAKEHEADER;
};

#pragma pack(pop)

#endif // !defined(AFX_UDPHEADHELPER_H__602C791B_E893_4D09_B17C_0407FBC518CC__INCLUDED_)
