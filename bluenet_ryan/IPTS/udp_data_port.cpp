#include "udp_data_port.h"
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

CUDPDataPort::CUDPDataPort()
{
    m_hSocket = -1;
    memset( &m_mreq, 0, sizeof(m_mreq) );
    memset( &m_DstIP, 0, sizeof(m_DstIP) );
}

CUDPDataPort::~CUDPDataPort()
{
    Invalid();
}

///------------------------------------------
/// Function:
///		Initialize socket and join the multicast
/// Input Parameter:
///		lpszDstIP			destination IP address
///		nPort				data port
///		lpszLocalBindIP		local bind IP address
/// Output Parameter:
///		true				succ
///		false				failed
bool CUDPDataPort::Initialize( const char * lpszDstIP, int nPort, const char* lpszLocalBindIP, CSubNetworkEncapsulation * pSubNetworkEncapsulation )
{
    Invalid();
    memset( &m_mreq, 0, sizeof(m_mreq) );
    m_pSubNetworkEncapsulation = pSubNetworkEncapsulation;

#ifdef _DEBUG
    assert( lpszDstIP && nPort>1024 );
#endif //_DEBUG
    if( NULL == lpszDstIP || nPort < 1024 )
        return false;

    m_hSocket = socket( AF_INET, SOCK_DGRAM, 0 );
    if( m_hSocket < 0 )
    {
#ifdef _DEBUG
        printf("Create multicast UDP socket failed.\n");
#endif //_DEBUG
        return false;
    }

    bool bReusable = true;
    setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&bReusable, sizeof(bReusable) );

    int on = 1;
    setsockopt( m_hSocket, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on) );

    EnableAllMulti( lpszLocalBindIP );

    struct sockaddr_in tmp;
    memset( &tmp, 0, sizeof(tmp) );
    tmp.sin_family = AF_INET;
    tmp.sin_port = htons( nPort );
    tmp.sin_addr.s_addr = inet_addr( lpszDstIP );

    if( bind( m_hSocket, (struct sockaddr*)&tmp, sizeof(tmp) ) < 0 )
    {
#ifdef _DEBUG
        printf("Bind to %s failed. Multicast IP=%s:%d\n", lpszLocalBindIP, lpszDstIP, nPort );
#endif //_DEBUG
        Invalid();
        return false;
    }

    m_DstIP.sin_addr.s_addr = inet_addr( lpszDstIP );
    m_DstIP.sin_port = htons( nPort );
    m_DstIP.sin_family = AF_INET;

    memset( &m_mreq, 0, sizeof(m_mreq) );
    // 2016.4.23 CYJ Add, add membership only multicast case
    int nFirstAddr = m_DstIP.sin_addr.s_addr&0xff;
    if( nFirstAddr >= 224  && nFirstAddr <= 239 )
    {
        m_mreq.imr_multiaddr.s_addr = inet_addr( lpszDstIP );
        if( lpszLocalBindIP && *lpszLocalBindIP )
            m_mreq.imr_interface.s_addr = inet_addr( lpszLocalBindIP );
        else
            m_mreq.imr_interface.s_addr = htonl( INADDR_ANY );

        if( setsockopt( m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&m_mreq, sizeof(m_mreq) ) < 0 )
        {
#ifdef _DEBUG
            printf("IP Add member ship failed.\n");
#endif //_DEBUG
            Invalid();
            return false;
        }
    }

    AdjustRcvBufferSize();

    return true;
}
///------------------------------------------
/// Function:
///		adjust rcv buffer size
/// Input Parameter:
///		None
/// Output Parameter:
///		try to set the rcv buffer = 512KB
void CUDPDataPort::AdjustRcvBufferSize()
{
    int nBufSize = 2048L*1024L;

    // adjust the rmem_max
    int anName[] = { CTL_NET, NET_CORE, NET_CORE_RMEM_MAX };
    int nNewValue = nBufSize;
    int nNameSize = sizeof(anName)/sizeof(int);
    sysctl(anName, nNameSize, NULL, NULL, (void*)&nNewValue, sizeof(int));

    setsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&nBufSize, sizeof(nBufSize) );

#ifdef _DEBUG
    nBufSize = 0;
    socklen_t len = sizeof(int);
    getsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&nBufSize, &len );
#endif //_DEBUG
}

///------------------------------------------
/// Function:
///		close and release the resource
/// Input Parameter:
///		None
/// Output Parameter:
///		None
void CUDPDataPort::Invalid()
{
    if( m_hSocket > 0 )
    {
        if( m_mreq.imr_multiaddr.s_addr )
        {
            setsockopt( m_hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&m_mreq, sizeof(m_mreq) );
            memset( &m_mreq, 0, sizeof(m_mreq) );
        }

        close( m_hSocket );
    }
    m_hSocket = -1;
}

//---------------------------------------
//	2004.5.31
//	Let netcard Enable allmulti
//	Input parameter:
//		pszLocalBindIP			local bind IP
bool CUDPDataPort::EnableAllMulti(const char * pszLocalBindIP)
{
    char szNetAdapter[10];
    struct ifreq ifr;

    assert( pszLocalBindIP );
    if( NULL == pszLocalBindIP )
        pszLocalBindIP = "";

    sockaddr_in LocalBindAddr;
    LocalBindAddr.sin_family = AF_INET;
    LocalBindAddr.sin_port = 0;
    if( 0 == pszLocalBindIP[0] )
        strcpy( szNetAdapter, "eth0" );		//	set default eth to eth0
    else
    {						//	auto match the eth by local bind
        LocalBindAddr.sin_addr.s_addr = inet_addr( pszLocalBindIP );
        for(int i=0; i<4; i++)		// max support 4 network adapters
        {
            sprintf( szNetAdapter, "eth%d", i );
            bzero( &ifr, sizeof(ifr) );
            strncpy( ifr.ifr_name, szNetAdapter, sizeof(ifr.ifr_name)-1 );
            ifr.ifr_addr.sa_family = AF_INET;
            if( ioctl( m_hSocket, SIOCGIFADDR, &ifr ) < 0 )
            {
                strcpy( szNetAdapter, "eth0" );
                break;
            }
            sockaddr_in * pSrcIP = (sockaddr_in * )&ifr.ifr_addr;
            if( pSrcIP->sin_addr.s_addr == LocalBindAddr.sin_addr.s_addr )
                break;						//	match the eth
        }
    }

    bzero( &ifr, sizeof(ifr) );
    strncpy( ifr.ifr_name, szNetAdapter, sizeof(ifr.ifr_name)-1 );
    if( ioctl( m_hSocket, SIOCGIFFLAGS, &ifr ) < 0 )
        return false;
    ifr.ifr_flags |= IFF_ALLMULTI;
    if( ioctl( m_hSocket, SIOCSIFFLAGS, &ifr ) < 0 )
        return false;

    return true;
}
