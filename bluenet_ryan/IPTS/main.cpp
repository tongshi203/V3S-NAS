#include <QCoreApplication>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "udp_rec_thread.h"
#include "parse_content_xml.h"
#include "udp_send.h"

static bool GetLocalIP(char* pLocalIP);

int main( int argc, char* argv[] )
{
    CParseContentXml parseContentXml;
    QVector<CONTENT_INFO> vecContentInfo = parseContentXml.GetContentInfo();
    if( 0 == vecContentInfo.count() )
    {
#ifdef _DEBUG
        printf("content.xml is error!!!\n");
#endif
        return -1;
    }
#ifdef _DEBUG
    for( int i=0; i<vecContentInfo.count(); i++ )
        printf("%s:%d 0x%02x \n", vecContentInfo[i].m_strIP.toLatin1().data(), vecContentInfo[i].m_nPort, vecContentInfo[i].m_byPID );
#endif

    QCoreApplication a(argc, argv);

    CUDPRecThread  RecThread;
    RecThread.CreateThread();
    CUDPSend       UDPSend;
    UDPSend.Initialize( "1.8.23.22", 50000 );
    // 封装以太网帧的源端和目的端的ip & port
    const char * pDstIP = "224.0.0.1";
    char szLocalIP[50];
    memset( szLocalIP, 0, sizeof(szLocalIP) );
    GetLocalIP( szLocalIP );
    WORD wDstPort = 20000;
    WORD wSrcPort = 20001;
    for( int i=0; i<vecContentInfo.count(); i++ )
    {
        CSubNetworkEncapsulation* pSubNetworkEncapsulation = new CSubNetworkEncapsulation(&UDPSend);

        pSubNetworkEncapsulation->SetPID( vecContentInfo[i].m_byPID );
        pSubNetworkEncapsulation->SetRawUDPIPPort( pDstIP, wDstPort, szLocalIP, wSrcPort );

        // 收udp数据的ip & port
        RecThread.AddDataPort( vecContentInfo[i].m_strIP.toLatin1().data(), vecContentInfo[i].m_nPort, szLocalIP, pSubNetworkEncapsulation );
    }

    return a.exec();
}

//////////////////////////////////////////////////////////////////////
static bool GetLocalIP( char* pLocalIP )
{
    // 获取IP
    FILE* f = fopen( "/proc/net/dev", "r" );
    if( NULL == f )
    {
#ifdef _DEBUG
        printf("Error: open /proc/net/dev failed!");
#endif
        return false;
    }
    char szLine[512];
    memset( szLine, 0, 512 );
    fgets(szLine, sizeof(szLine), f);
    fgets(szLine, sizeof(szLine), f);
    while( fgets(szLine, sizeof(szLine), f) )
    {
        char szName[128];
        memset( szName, 0, 128 );
        sscanf( szLine, "%s", szName );
        int nLen = strlen( szName );
        if (nLen <= 0)
            continue;

        if ( ':' == szName[nLen - 1] )
            szName[nLen - 1] = 0;

        if ( 0 == strcmp( szName, "lo" ) || NULL != strstr( szName, "virbr" ) )
            continue;

        struct ifreq ifr;
        int nInetSock = socket( AF_INET, SOCK_DGRAM, 0 );
        if( -1 == nInetSock )
        {
#ifdef _DEBUG
            printf("%s:%d create socket failed!  errno = %d\n", __FUNCTION__,__LINE__,errno);
#endif
            return false;
        }
        int flags = fcntl( nInetSock, F_GETFL, 0 );
        fcntl( nInetSock, F_SETFL, flags | O_NONBLOCK );
        strcpy( ifr.ifr_name, szName );
        bool bGetIP = false;
        if ( -1 != ioctl(nInetSock, SIOCGIFADDR, &ifr)  )
        {
            memcpy( pLocalIP, inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr), strlen(inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr)) );
            bGetIP = true;
        }
        else
            printf("%s:%d get ip from %s  failed\n", __FUNCTION__,__LINE__,szName);

        ::close( nInetSock );
        nInetSock = -1;

        if( bGetIP )
            break;
    }

    fclose(f);
    f = NULL;

    return true;
}
