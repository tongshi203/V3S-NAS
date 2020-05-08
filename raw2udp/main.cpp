#include <string.h>
#include <stdio.h>
#include "CUDPDataPort.h"
#include "CV4l2DataPort.h"
#include <stdlib.h>


void GetIPPort( char*pInput, char*ipaddr, int&nPort )
{
    if( NULL == pInput || 0 == *pInput )
        return;

    char *rp = strtok(pInput,":");//字符串分割
    ipaddr = rp;
    if( NULL != rp )
    {
        rp = strtok(NULL,":");
        nPort = atoi(rp);
    }
}

void GetImagePort( char*pInput, int& width, int&height )
{
    if( NULL == pInput || 0 == *pInput )
        return;

    char *rp = strtok(pInput,"*");//字符串分割
    width = atoi(rp);//字符串转长整数
    if( NULL != rp )
    {
        rp = strtok(NULL,"*");
        height = atoi(rp);
    }
}

int main(int argc, char *argv[])
{
    if( argc < 3 )
    {
        printf("please input like this:./raw2udp 192.168.100.100:10000 600*512\n");
        return -1;
    }
    char *targetip = NULL;
    int targetport;
    int raw_width=0,raw_height=0;

    GetIPPort( argv[1], targetip, targetport);
    if(targetip != NULL && targetport != 0)
        printf("send udp to (ip:port) %s:%d\n",targetip,targetport);
    else
    {
        printf("please input ip should like this:192.168.100.100:10000");
        return -1;
    }

    GetImagePort( argv[1], raw_width, raw_height);
    if(raw_width != 0 && raw_height != 0)
        printf("the image is %d*%d\n",raw_width,raw_height);
    else
    {
        printf("please input image size should like this:600*512");
        return -1;
    }

    unsigned char *buf = new unsigned char[raw_width*raw_height*2]();
    unsigned char *hdata_buf = new unsigned char[raw_width*2 + 8]();

    CV4l2DataPort * myReceiveTSPort = NULL;
    myReceiveTSPort = new CV4l2DataPort();
    myReceiveTSPort -> Initialize("/dev/video0",raw_width,raw_height,12);

    CUDPDataPort* pMySendUDPPort = new CUDPDataPort;

    pMySendUDPPort->Initialize( "NULL", 50000, (const char*)targetip, targetport);

    printf("\n");
    hdata_buf[0] = 0;
    hdata_buf[1] = 0;
    hdata_buf[2] = raw_width & 0xff;
    hdata_buf[3] = (raw_width >> 8) & 0xff;
    hdata_buf[4] = raw_height & 0xff;
    hdata_buf[5] = (raw_height >> 8) & 0xff;
    hdata_buf[6] = 0;
    hdata_buf[7] = 0;
    unsigned char *hdata_fp;
    while( 1 )
    {
        int ret = myReceiveTSPort -> GetData(buf);
        hdata_fp = buf;

        if( ret == 0 )
        {
            for(int i = 0; i < raw_height; i++)
            {
              memcpy(&hdata_buf[8], hdata_fp, raw_width*2);
              pMySendUDPPort->SentData( hdata_buf, raw_width*2 + 8);//send a line data
              hdata_buf += raw_width*2;
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

    return 0;
}
