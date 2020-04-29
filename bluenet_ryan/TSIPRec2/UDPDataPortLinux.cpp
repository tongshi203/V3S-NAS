///---------------------------------------------------------
///
///      Chen Yongjian @ Xi'an Tongshi Technology Limited
///				2004.4.5
///      This file is implemented:
///				Multicast UDP Data port
///-----------------------------------------------------------

#include "stdafx.h"

#ifndef _WIN32
  #define __signed__

  #include <unistd.h>
  #include <stdio.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <string.h>
  #include <net/if.h>
  #include <sys/ioctl.h>
  #include <linux/unistd.h>
  #include <sys/sysctl.h>
  #include <sys/time.h>

#endif //_WIN32

#include <string.h>

#include "UDPDataPortLinux.h"

//---------------------------------------------------------------------------


CUDPDataPort::CUDPDataPort()
{
	m_hSocket = -1;
    memset( &m_mreq, 0, sizeof(m_mreq) );
    memset( &m_DstIP, 0, sizeof(m_DstIP) );
    m_pDataPortItem = NULL;
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
///		pDataPortItem		associated data port item
/// Output Parameter:
///		true				succ
///		false				failed
bool CUDPDataPort::Initialize( const char * lpszDstIP, int nPort, const char* lpszLocalBindIP,COneDataPortItem * pDataPortItem )
{
	Invalid();
    memset( &m_mreq, 0, sizeof(m_mreq) );
	m_pDataPortItem = pDataPortItem;

#ifdef _DEBUG
	ASSERT( lpszDstIP && nPort>1024 );
#endif //_DEBUG
	if( NULL == lpszDstIP || nPort < 1024 )
    	return false;

    m_hSocket = socket( AF_INET, SOCK_DGRAM, 0 );
    if( m_hSocket < 0 )
    {
#ifdef _DEBUG
		TRACE("Create multicast UDP socket failed.\n");
#endif //_DEBUG
    	return false;
    }

    bool bReusable = true;
    setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&bReusable, sizeof(bReusable) );

#ifndef _WIN32
    EnableAllMulti( lpszLocalBindIP );
#endif //_WIN32

    struct sockaddr_in tmp;
	memset( &tmp, 0, sizeof(tmp) );
    tmp.sin_family = AF_INET;
    tmp.sin_port = htons( nPort );

#ifdef _WIN32
	if( lpszLocalBindIP && *lpszLocalBindIP )
		tmp.sin_addr.s_addr = inet_addr( lpszLocalBindIP );
    else
    	tmp.sin_addr.s_addr = htonl( INADDR_ANY );
#else
    tmp.sin_addr.s_addr = inet_addr( lpszDstIP );

#endif //_WIN32
	if( bind( m_hSocket, (struct sockaddr*)&tmp, sizeof(tmp) ) < 0 )
    {
#ifdef _DEBUG
		TRACE("Bind to %s failed. Multicast IP=%s:%d\n", lpszLocalBindIP, lpszDstIP, nPort );
#endif //_DEBUG
    	Invalid();
        return false;
    }

	m_DstIP.sin_addr.s_addr = inet_addr( lpszDstIP );
    m_DstIP.sin_port = htons( nPort );
	m_DstIP.sin_family = AF_INET;

    memset( &m_mreq, 0, sizeof(m_mreq) );
    // 2016.4.23 CYJ Add, add membership only multicast case
	int nFirstAddr = m_DstIP.sin_addr.s_addr >> 24;
	if( nFirstAddr >= 224  && nFirstAddr <= 239 )
	{
		m_mreq.imr_multiaddr.s_addr = inet_addr( lpszDstIP );
		if( lpszLocalBindIP && *lpszLocalBindIP )
			m_mreq.imr_interface.s_addr = inet_addr( lpszLocalBindIP );
		else
			m_mreq.imr_interface.s_addr = htonl( INADDR_ANY );

	#ifdef _WIN32
		if( setsockopt( m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char FAR*)&m_mreq, sizeof(m_mreq) ) < 0 )
	#else
		if( setsockopt( m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&m_mreq, sizeof(m_mreq) ) < 0 )
	#endif
		{
	#ifdef _DEBUG
	#ifdef _WIN32
			int nLastError = WSAGetLastError();
	#endif //_WIN32
			TRACE("IP Add member ship failed.\n");
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

#ifndef _WIN32			// adjust the rmem_max
	int anName[] = { CTL_NET, NET_CORE, NET_CORE_RMEM_MAX };
    int nNewValue = nBufSize;
    int nNameSize = sizeof(anName)/sizeof(int);
	sysctl(anName, nNameSize, NULL, NULL, (void*)&nNewValue, sizeof(int));
#endif //_WIN32

    setsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&nBufSize, sizeof(nBufSize) );

#ifdef _DEBUG
	nBufSize = 0;
#ifdef _WIN32
    int len = sizeof(int);
#else
	socklen_t len = sizeof(int);
#endif //_WIN32
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
#ifdef _WIN32
			setsockopt( m_hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char*)&m_mreq, sizeof(m_mreq) );
#else
        	setsockopt( m_hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&m_mreq, sizeof(m_mreq) );
#endif //_WIN32
            memset( &m_mreq, 0, sizeof(m_mreq) );
        }

#ifdef _WIN32
    	closesocket( m_hSocket );
#else
		close( m_hSocket );
#endif //_WIN32
    }
    m_hSocket = -1;
    m_pDataPortItem = NULL;
}

#ifndef _WIN32
//---------------------------------------
//	2004.5.31
//	Let netcard Enable allmulti
//	Input parameter:
//		pszLocalBindIP			local bind IP
bool CUDPDataPort::EnableAllMulti(const char * pszLocalBindIP)
{
    char szNetAdapter[10];
    struct ifreq ifr;

    ASSERT( pszLocalBindIP );
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
#endif //_WIN32
