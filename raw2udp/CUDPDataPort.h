#ifndef CUDPDATAPORT_H
#define CUDPDATAPORT_H

#define IP_MAX_PACKET_SIZE 2048

#include <netinet/in.h>
class CUDPDataPort
{
public:
    CUDPDataPort();
    ~CUDPDataPort();
    bool Initialize( const char* localBindIP, int port, const char* dstIP, int dstPort );
    void Invalid();
    int GetData( unsigned char * buf );
    int SentData( unsigned char * buf, int len);

public:
    int m_hSocket;
    int myDstPort;

private:
    struct sockaddr_in dstAddr;
    socklen_t dstAddrLen ;


};

#endif // UDPPORT_H
