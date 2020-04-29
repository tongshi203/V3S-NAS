#ifndef __UDP_DATA_PORT_H_20190703__
#define __UDP_DATA_PORT_H_20190703__

#include "sub_network_encapsulation.h"
#include <arpa/inet.h>

class CUDPDataPort
{
public:
    CUDPDataPort();
    ~CUDPDataPort();
    bool Initialize( const char * lpszDstIP, int nPort, const char* lpszLocalBindIP, CSubNetworkEncapsulation * pSubNetworkEncapsulation );
    void Invalid();
    operator int(){ return m_hSocket; }
    CSubNetworkEncapsulation * GetSubNetworkEncapsulation(){ return m_pSubNetworkEncapsulation; }

public:
    int                 m_hSocket;
    struct ip_mreq      m_mreq;
    struct sockaddr_in  m_DstIP;
    CSubNetworkEncapsulation*   m_pSubNetworkEncapsulation;

private:
    void AdjustRcvBufferSize();
    bool EnableAllMulti(const char * pszLocalBindIP);
};

#endif // __UDP_DATA_PORT_H_20190703__
