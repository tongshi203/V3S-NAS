#include <string.h>
#include <stdio.h>
#include "CUDPDataPort.h"
#include "CV4l2DataPort.h"
#include <stdlib.h>
#include <unistd.h>
#include "CUARTDataPort.h"

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
//    if( argc < 3 )
//    {
//        printf("please input like this:./raw2udp 192.168.100.100:10000 600*512\n");
//        return -1;
//    }
//    char *targetip = NULL;
//    int targetport;
    int raw_width=640,raw_height=512;

    uint8_t TxData_ON[9] = {0xAA,0x05,0x00,0x2F,0x01,0x01,0xE0,0xEB,0xAA}; //数字接口打开
    uint8_t TxData_OFF[9] = {0xAA,0x05,0x00,0x2F,0x01,0x00,0xDF,0xEB,0xAA}; //数字接口关闭
    uint8_t TxDRCData[9] = {0xAA,0x05,0x00,0x2E,0x01,0x02,0xE0,0xEB,0xAA};//视频源DRC
    //uint8_t Tx_Frame[10] = {0xAA,0x06,0x01,0xA3,0x01,0x00,0x1E,0x73,0xEB,0xAA};//30Hz
    //uint8_t Tx_Frame[10] = {0xAA,0x06,0x01,0xA3,0x01,0x00,0x0F,0x64,0xEB,0xAA};//15Hz
    uint8_t Tx_Frame[10] = {0xAA,0x06,0x01,0xA3,0x01,0x00,0x05,0x5A,0xEB,0xAA};//5Hz

//    GetIPPort( argv[1], targetip, targetport);
//    if(targetip != NULL && targetport != 0)
//        printf("send udp to (ip:port) %s:%d\n",targetip,targetport);
//    else
//    {
//        printf("please input ip should like this:192.168.100.100:10000");
//        return -1;
//    }
//
//    GetImagePort( argv[1], raw_width, raw_height);
//    if(raw_width != 0 && raw_height != 0)
//        printf("the image is %d*%d\n",raw_width,raw_height);
//    else
//    {
//        printf("please input image size should like this:600*512");
//        return -1;
//    }


    unsigned char *buf = new unsigned char[raw_width*raw_height*2]();
    unsigned char *hdata_buf = new unsigned char[raw_width*2 + 8]();

    CUARTDataPort * myUARTPort = NULL;
    myUARTPort = new CUARTDataPort();
    myUARTPort -> Initialize("/dev/ttyS1",115200,8,'N',1);

    myUARTPort->SendData(TxData_ON,9);
    sleep(1);
    myUARTPort->SendData(Tx_Frame,10);
    sleep(1);

    CV4l2DataPort * myReceiveTSPort = NULL;
    myReceiveTSPort = new CV4l2DataPort();
    myReceiveTSPort -> Initialize("/dev/video0",raw_width,raw_height,12);

    CUDPDataPort* pMySendUDPPort = new CUDPDataPort;

//    pMySendUDPPort->Initialize( "NULL", 50000, (const char*)targetip, targetport);
    pMySendUDPPort->Initialize( "NULL", 50000, "1.8.86.220", 20000);

    printf("net success\n");
    hdata_buf[0] = 0;
    hdata_buf[1] = 0;
    hdata_buf[2] = raw_width & 0xff;
    hdata_buf[3] = (raw_width >> 8) & 0xff;
    hdata_buf[4] = raw_height & 0xff;
    hdata_buf[5] = (raw_height >> 8) & 0xff;
    hdata_buf[6] = 0;
    hdata_buf[7] = 0;

    printf("start capture\n");
    while( 1 )
    {
        int ret = myReceiveTSPort -> GetData(buf);
        printf("ret = %d\n",ret);
        if( ret == 0 )
        {
            for(int i = 0; i < raw_height; i++)
            {
              hdata_buf[0] = i & 0xFF;
              hdata_buf[1] = (i>>8) & 0xFF;

              memcpy(&hdata_buf[8], &buf[raw_width*2*i] , raw_width*2);
//              memcpy(&hdata_buf[8], buf + raw_width*i, raw_width);
//              pMySendUDPPort->SentData( hdata_buf, raw_width + 8);//send a line data
              pMySendUDPPort->SentData( hdata_buf, raw_width*2 + 8);//send a line data
            usleep(2);
//              printf("i = %d\n",i);
            }

        }
    }

    if( buf != NULL )
        delete buf;
    printf("buf delete success\n");
    if( hdata_buf != NULL )
        delete hdata_buf;
printf("hdatabuf delete success\n");

    if( myUARTPort != NULL )
        delete myUARTPort;
printf("uart delete success\n");

    if( myReceiveTSPort != NULL )
        delete myReceiveTSPort;
printf("rec delete success\n");

    if( pMySendUDPPort!= NULL )
    {
        delete pMySendUDPPort;
        pMySendUDPPort = NULL;
    }
printf("send delete success\n");
    return 0;
}
