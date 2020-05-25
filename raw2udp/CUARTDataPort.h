#ifndef CUARTDATAPORT_H
#define CUARTDATAPORT_H


//typedef enum _MT_FE_LNB_VOLTAGE
//{
//    MtFeLNB_13V = 0
//    ,MtFeLNB_18V
//} MT_FE_LNB_VOLTAGE;
//

class CUARTDataPort
{
public:
    CUARTDataPort();
    ~CUARTDataPort();
    bool Initialize(char * UART_DEVICE,int BaudRate, int nBits, char nEvent, int nStop);
    void Invalid();
    int SendData( unsigned char * buf ,int Send_len);
    int GetData( unsigned char * buf,int rec_len );
    int set_opt(int BaudRate, int nBits, char nEvent, int nStop);

public:
    int uart_fd;

private:

    int MyBaudRate;
    int MynBits;
    char MynEvent;
    int Mynstop;

};

#endif // V4L2DATAPORT_H
