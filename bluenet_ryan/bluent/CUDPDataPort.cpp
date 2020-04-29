#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "CUDPDataPort.h"

using namespace std;
CUDPDataPort::CUDPDataPort()
{
    m_hSocket = -1;
    dstAddrLen = 0;

}

CUDPDataPort::~CUDPDataPort()
{

    Invalid();

}

///
/// \brief CUDPDataPort::Initialize
///     Initialize socket
/// \param localBindIP
///     Local bind IP address which can be NULL
/// \param port
///     data port
/// \return
///     true or false
///
///
bool CUDPDataPort::Initialize( const char* localBindIP, int port, const char* dstIP, int dstPort )
{

    Invalid();

    // create the udp receive socket address.
    struct sockaddr_in mySocketAddr;
    bzero ( &mySocketAddr, sizeof( mySocketAddr ) );
    mySocketAddr.sin_family = AF_INET;
    mySocketAddr.sin_port = htons( port );

    if( localBindIP && *localBindIP )
        mySocketAddr.sin_addr.s_addr = inet_addr( localBindIP );
    else
        mySocketAddr.sin_addr.s_addr = htons( INADDR_ANY );

    // create the udp socket.
    m_hSocket = socket( AF_INET, SOCK_DGRAM, 0 );

    //bind the socket to the address.
    int result = bind( m_hSocket, ( struct sockaddr * )&mySocketAddr, sizeof( mySocketAddr ) );
    if( result == -1 )
    {
        cout << "bind false. " << endl;
        return false;
    }

    int bufSize = 2048 * 1024L;
    int ret = setsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, (char * )&bufSize, sizeof( bufSize ) );
    if( ret < 0 )
        cout  << "set sock failed." << 0;

    if( dstIP && *dstIP)
    {
        bzero ( &dstAddr, sizeof( dstAddr ) );
        dstAddr.sin_family = AF_INET;
        dstAddr.sin_port = htons( dstPort );
        dstAddr.sin_addr.s_addr = inet_addr( dstIP );

        dstAddrLen = sizeof( struct sockaddr_in );
        myDstPort = dstPort;

    }

    return true;
}

///
/// \brief CUDPDataPort::GetData
/// \param buf
///     save the receive data
/// \return
///     return the length of receive data
///

int CUDPDataPort::GetData( unsigned char * buf )
{
    int len = 0;

    fd_set m_fdset_read;
    FD_ZERO( &m_fdset_read );
    int nMaxFd = 0;   // find the max fd

    FD_SET( m_hSocket, &m_fdset_read );
    nMaxFd = m_hSocket;

    struct timeval TimeOut;   // Linux struct
    TimeOut.tv_sec = 0;
    TimeOut.tv_usec = 1000;
    select( nMaxFd+1, &m_fdset_read, NULL, NULL, &TimeOut );

    if(FD_ISSET( m_hSocket, &m_fdset_read ) )
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_length = sizeof( client_addr );
        len = recvfrom( m_hSocket, buf, IP_MAX_PACKET_SIZE, 0, ( struct sockaddr * )&client_addr, &client_addr_length );
        //cout << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port)<< endl;
        if( len > 0 )
        {
           // cout<<"<<< " << len << endl;
        }
    }
    return len;

}

int CUDPDataPort::SentData( unsigned char * buf, int len)
{
    if( dstAddrLen == 0 )
        return 0;
    return sendto( m_hSocket, buf, len, 0, ( struct sockaddr * )&dstAddr, dstAddrLen );

}

//-------------------------------------------
// Function:
//      close and release the resource
//
void CUDPDataPort::Invalid()
{
    if( m_hSocket > 0 )
    {

        close( m_hSocket );
        cout << "close the socket. " << endl;

    }
    m_hSocket = -1;
}


