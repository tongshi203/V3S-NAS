#ifndef __UDP_SEND_H_20190709__
#define __UDP_SEND_H_20190709__

#include <arpa/inet.h>
#include <QMutex>
#include "value.h"

class CUDPSend
{
public:
    CUDPSend();
    ~CUDPSend();

    bool Initialize( const char* lpszDstIP, WORD wDstPort );
    void Send( PBYTE pBuf, WORD wBufLen );

private:
    int                m_nUDPSockfd;
    struct sockaddr_in m_UDPServerAddr;
    QMutex             m_SyncObj;
};

#endif // __UDP_SEND_H_20190709__
