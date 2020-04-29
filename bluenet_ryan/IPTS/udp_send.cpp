#include "udp_send.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

CUDPSend::CUDPSend()
{
    m_nUDPSockfd = -1;
}

CUDPSend::~CUDPSend()
{
    if( -1 != m_nUDPSockfd )
    {
        close(m_nUDPSockfd);
        m_nUDPSockfd = -1;
    }
}

bool CUDPSend::Initialize(const char *lpszDstIP, WORD wDstPort)
{
    m_UDPServerAddr.sin_family = AF_INET;
    m_UDPServerAddr.sin_port = htons(wDstPort);
    m_UDPServerAddr.sin_addr.s_addr = inet_addr(lpszDstIP);

    if( -1 == m_nUDPSockfd )
    {
        m_nUDPSockfd = socket( AF_INET,SOCK_DGRAM,0 );
        if( -1 == m_nUDPSockfd )
        {
#ifdef _DEBUG
            printf("%s:%d create socket failed!\n",__FUNCTION__,__LINE__);
#endif
            return false;
        }

        bool bReusable = true;
        setsockopt( m_nUDPSockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&bReusable, sizeof(bReusable) );

        int nBufSize = 2048 * 1024L;
        setsockopt( m_nUDPSockfd, SOL_SOCKET, SO_SNDBUF, (char*)&nBufSize, sizeof(nBufSize) );
    }

    return true;
}

void CUDPSend::Send( PBYTE pBuf, WORD wBufLen )
{
    QMutexLocker syncobj( &m_SyncObj );

    if( NULL == pBuf || 0 == wBufLen )
        return;

    sendto( m_nUDPSockfd, pBuf, wBufLen, 0, (struct sockaddr*)&m_UDPServerAddr,sizeof(m_UDPServerAddr) );
}
