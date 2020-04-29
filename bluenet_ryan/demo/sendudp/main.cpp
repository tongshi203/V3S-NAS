/***********************************************************************************
 *
 *  Send UDP packet from DVB-MPE
 *
 *  Chen Yongjian @ Tongshi
 *  2016.4.23 @ Xi'an
 *
 *
 * Multicast IP is ignored, only send to 127.0.0.1, but keep the UDP Port
 *
 *********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>

unsigned char s_abyBuf[0x10000];

int main( int argc, char * argv[] )
{
    fprintf( stderr, "Usage: sendudp <filename>\n" );
    if( argc < 2 )
    {
        fprintf( stderr, "DVB-MPE UDP packet file is need.\n" );
        return 1;
    }

    FILE * fp = fopen( argv[1], "rb" );
    if( NULL == fp )
    {
        fprintf( stderr, "Open file failed %s\n", argv[1] );
        return 2;
    }

    int hOutUDPSocket = socket( AF_INET, SOCK_DGRAM, 0 );

    const int nHeadLen = 14 + 20 + 8;       // Eth + IP + UDP
    unsigned char * pbyPayload = s_abyBuf + nHeadLen;
    while( 1 )
    {
        int nDataLen = fread( s_abyBuf, 1, nHeadLen, fp );
        if( nDataLen != nHeadLen )
            break;
        assert( s_abyBuf[14] == 0x45 );
        int nUDPLen = s_abyBuf[ nHeadLen-4 ] * 0x100 + s_abyBuf[ nHeadLen-3 ] - 8;
        int nPort = s_abyBuf[ nHeadLen-6 ] * 0x100 + s_abyBuf[ nHeadLen-5 ];
        fprintf( stderr, "udp len=%d, dst port: %d\n", nUDPLen, nPort );
        nDataLen = fread( pbyPayload, 1, nUDPLen, fp );
        if( nDataLen != nUDPLen )
            break;

        struct sockaddr_in dst_ip;
        memset( &dst_ip,0,sizeof(dst_ip));
        dst_ip.sin_family = AF_INET;
        dst_ip.sin_addr.s_addr = inet_addr( "127.0.0.1" );
        dst_ip.sin_port = htons( nPort );

        sendto( hOutUDPSocket, pbyPayload, nDataLen, 0, (const struct sockaddr *)&dst_ip, sizeof(dst_ip) );
        usleep( 1000 );
    }
    fclose( fp );
    close( hOutUDPSocket );
    return 0;
}
