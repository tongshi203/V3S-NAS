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

#include "stdafx.h"
#include "UDPCapture.h"


CUDPCaptureThread::CUDPCaptureThread()
 : CLookaheadPacketMgr< CIP_Packet >( 16*1024, 128 ) //Max UDP Count is 16K, total bytes=16M, and catch 128 items
{
    m_hSocket = -1;     // default is not opened
    bzero( m_szLocalBindIP, sizeof(m_szLocalBindIP) );
}

CUDPCaptureThread::~CUDPCaptureThread()
{
}

int CUDPCaptureThread::ExitInstance()
{
    if( m_hSocket >= 0 )
    {
        close( m_hSocket );
        m_hSocket = -1;
    }

    return 0;
}

bool CUDPCaptureThread::InitInstance()
{
	m_hSocket = socket( PF_PACKET, SOCK_RAW, htons(3) ); 	// ETH_P_ALL
    if( m_hSocket < 0 )
        return false;           //  create socket failed.

    sockaddr_in LocalBindAddr;
    LocalBindAddr.sin_family = AF_INET;
    LocalBindAddr.sin_port = 0;
    if( m_szLocalBindIP[0] )
	    LocalBindAddr.sin_addr.s_addr = inet_addr( m_szLocalBindIP );
    else
		LocalBindAddr.sin_addr.s_addr = INADDR_ANY;
    bind( m_hSocket, (sockaddr*)&LocalBindAddr, sizeof(LocalBindAddr) );

    if( !Set_Promisc_Mode() )		// let network adapter run into promiscous mode
    	return false;

	int nBufSize = 2L*1024*1024L;
	int anName[] = { CTL_NET, NET_CORE, NET_CORE_RMEM_MAX };
    int nNewValue = nBufSize;
    int nNameSize = sizeof(anName)/sizeof(int);
	sysctl(anName, nNameSize, NULL, NULL, (void*)&nNewValue, sizeof(int));
    setsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&nBufSize, sizeof(nBufSize) );

    return true;
}

//------------------------------------------
// Function:
//		Set network adapter into promiscuous mode, that is receive all UDP packet
// Input Parameter:
//		none
// Output Parameter:
//		true		   	succ
//		false			failure
bool CUDPCaptureThread::Set_Promisc_Mode()
{
	char szNetAdapter[10];
    struct ifreq ifr;

	sockaddr_in LocalBindAddr;
    LocalBindAddr.sin_family = AF_INET;
    LocalBindAddr.sin_port = 0;
    if( 0 == m_szLocalBindIP[0] )
		strcpy( szNetAdapter, "eth0" );		//	set default eth to eth0
    else
    {										//	auto match the eth by local bind
    	LocalBindAddr.sin_addr.s_addr = inet_addr( m_szLocalBindIP );
	    for(int i=0; i<4; i++)				// max support 4 network adapters
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
    ifr.ifr_flags |= (IFF_ALLMULTI|IFF_PROMISC);
    if( ioctl( m_hSocket, SIOCSIFFLAGS, &ifr ) < 0 )
    	return false;

	return true;
}

void CUDPCaptureThread::Run()
{
	CIP_Packet * pPacket = NULL;
	while( FALSE == m_bIsRequestQuit )
    {
		if( NULL == pPacket )
        {				//	if pPacket != NULL means last recvfrom is failed.
	    	pPacket = AllocatePacket();
    	    if( NULL == pPacket )
        		continue;
        }
        if( false == pPacket->SetBufSize( UDP_BUF_MAX_SIZE ) )	//	Max IP Packet < 1536 bytes
        {
			DeAllocate( pPacket );
            pPacket->Release();
            continue;
        }
        ASSERT( pPacket->GetBuffer() );
        int nSockAddrLen = sizeof(pPacket->m_IP_Param);
    	int nLen = recvfrom( m_hSocket, pPacket->GetBuffer(), pPacket->GetBufSize(), 0,\
        	(struct sockaddr*)&pPacket->m_IP_Param, (socklen_t*)&nSockAddrLen );
		ASSERT( nLen > 0 );
		if( nLen <= 0 )
	        continue;
//#ifdef _DEBUG
//        TRACE( "Receive One Packet Len=%d\n", nLen  );
//#endif //_DEBUG
        pPacket->PutDataLen( nLen );
        if( AddPacket( pPacket ) )
		{
	        pPacket->Release();
    	    pPacket = NULL;
        }
    }
}

void CUDPCaptureThread::Delete()
{
	if( m_bAutoDelete )
    	delete this;
}

//------------------------------------------
// Function:
//		Create UDP Capture thread
// Input Parameter:
//		lpszLocalBind		local bind
//		nCatchCount			catch count,default is 128
//							0 disable catch function
// Output Parameter:
//
bool CUDPCaptureThread::Create( const char * lpszLocalBind, int nCatchCount )
{
	if( nCatchCount < 0 )
    	nCatchCount = 128;
    else if( nCatchCount > 512 )
    	nCatchCount = 512;
    m_nCatchItemCount = nCatchCount;		//	set catch item count
	if( lpszLocalBind )
    	strncpy( m_szLocalBindIP, lpszLocalBind, sizeof(m_szLocalBindIP)-1 );
    return CreateThread();
}

