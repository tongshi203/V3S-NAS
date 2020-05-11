#include <string.h>
#include <stdio.h>
#include "CUDPDataPort.h"
#include "DVBDSMCC_IP_MPE.h"
#include "CV4l2DataPort.h"
#include <stdlib.h>


CV4l2DataPort * myReceiveTSPort = NULL;

void GetPIDPort( char*pInput, WORD& nPID, int&nPort )
{
    if( NULL == pInput || 0 == *pInput )
        return;

    char *rp = strtok(pInput,"-");//字符串分割
    nPID = strtol(rp, NULL, 16);//字符串转长整数
    if( NULL != rp )
    {
        rp = strtok(NULL,"-");
        nPort = atoi(rp);
    }
}

int main(int argc, char *argv[])
{
    if( argc < 2 )
    {
        printf("please input like this:./bluenet 0xb3-5053 0xb4-1200\n");
        return -1;
    }

    unsigned char *buf = new unsigned char[188*4]();
    myReceiveTSPort = new CV4l2DataPort();
    myReceiveTSPort -> Initialize("/dev/video0",188,1,8);

    CUDPDataPort* pMySendUDPPort = new CUDPDataPort[argc-1];
    CDVBDSMCC_IP_MPE* pMyDVBDSMCC = NULL;

    printf("send udp by (port:pid) ");
    for( int i=0; i<argc-1; i++ )
    {
        int nPort = 0;
        WORD nPID = 0;
        GetPIDPort( argv[1+i], nPID, nPort );
        printf("(0x%04x:%d) ", nPID, nPort);
        pMySendUDPPort[i].Initialize( "127.0.0.1", 10021+i, "127.0.0.1", nPort);
//        pMySendUDPPort[i].Initialize( "NULL", 10021+i, "1.8.86.220", nPort);
        if( NULL == pMyDVBDSMCC )
            pMyDVBDSMCC = new CDVBDSMCC_IP_MPE( &pMySendUDPPort[i], nPID );
        else
            pMyDVBDSMCC->AppendItem( new CDVBDSMCC_IP_MPE( &pMySendUDPPort[i], nPID ) );
    }
    printf("\n");

    PDVB_TS_PACKET myPacket = new DVB_TS_PACKET();
    while( 1 )
    {
        int ret = myReceiveTSPort -> GetData(buf);
        if( ret == 0 )
        {
            CDVBDSMCC_IP_MPE *pCurMyDVBDSMCC = pMyDVBDSMCC;

            unsigned short ts_pid;
            ts_pid = *(buf + 1) & 0x1f;
            ts_pid = (ts_pid << 8) | (*(buf + 2));

            for( int j=0; j<argc-1; j++ )
            {
                if(ts_pid == pCurMyDVBDSMCC->m_wPID )
                {
                    memcpy( myPacket -> m_abyData, &buf[0], DVB_TS_PACKET_SIZE);
//                    pMySendUDPPort[j].SentData( buf, 188);
                    pCurMyDVBDSMCC->PushOneTSPacket( myPacket );
                    break;
                }
                pCurMyDVBDSMCC = pMyDVBDSMCC->GetNextItem();

            }
        }
    }

    if( buf != NULL )
        delete buf;

    if( myReceiveTSPort != NULL )
        delete myReceiveTSPort;

    if( pMySendUDPPort!= NULL )
    {
        delete[] pMySendUDPPort;
        pMySendUDPPort = NULL;
    }
    if( pMyDVBDSMCC!= NULL )
    {
        delete[] pMyDVBDSMCC;
        pMyDVBDSMCC = NULL;
    }

    return 0;
}
