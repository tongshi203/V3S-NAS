#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>    /*PPSIX 终端控制定义*/
#include "CUARTDataPort.h"

using namespace std;

#define u8 unsigned char

#define  LOGD(...)  do {printf(__VA_ARGS__);printf("\n");} while (0)
#define DBG(fmt, args...) LOGD("%s:%d, " fmt, __FUNCTION__, __LINE__, ##args);
#define ASSERT(b) \
do \
{ \
    if (!(b)) \
    { \
        LOGD("error on %s:%d", __FUNCTION__, __LINE__); \
        return 0; \
    } \
} while (0)


CUARTDataPort::CUARTDataPort()
{
    uart_fd = -1;

    MyBaudRate = 115200;
    MynBits = 8;
    MynEvent = 'N';
    Mynstop = 1;

}

CUARTDataPort::~CUARTDataPort()
{

    Invalid();

}


int CUARTDataPort::set_opt(int BaudRate, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	/* 获取fd串口对应的termios结构体，这步主要是查询串口是否启动正常 */
	if  ( tcgetattr( uart_fd,&oldtio)  !=  0) {
		perror("SetupSerial 1");
		return -1;
	}
	//清空
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;	//配置成本地模式(本地连接、不改变端口所有者)、可读
	newtio.c_cflag &= ~CSIZE;		//清空数据位设置

	newtio.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
    newtio.c_oflag  &= ~OPOST;   /*Output*/

	/* 选择数据位 */
	switch( nBits )
	{
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}
	/* 选择校验位 */
	switch( nEvent )
	{
	case 'O':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);	//启用输入奇偶检测、去掉第八位
		break;
	case 'E':
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':
		newtio.c_cflag &= ~PARENB;
		break;
	}
	/* 选择波特率 */
	switch( BaudRate )
	{
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	case 460800:
		cfsetispeed(&newtio, B460800);
		cfsetospeed(&newtio, B460800);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}
	/* 选择停止位，貌似linux下不能设置(1.5 0.5)停止位 */
	if( nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB;
	else if ( nStop == 2 )
		newtio.c_cflag |=  CSTOPB;
	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(uart_fd,TCIFLUSH);
	/* 设置新配置 */
	if((tcsetattr(uart_fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
//	printf("set done!\n\r");
	return 0;
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
bool CUARTDataPort::Initialize(char * UART_DEVICE,int BaudRate, int nBits, char nEvent, int nStop)
{
    int i;
    int ret;

    MyBaudRate = BaudRate;
    MynBits = nBits;
    MynEvent = nEvent;
    Mynstop = nStop;

    uart_fd = open(UART_DEVICE, O_RDWR);  // | O_NONBLOCK);//打开摄像头

    if(uart_fd < 0)
    {
        cout << "can't open the uart device. " << endl;
        return false;
    }
    else
    {
        printf("open %s is success\n",UART_DEVICE);
        set_opt(MyBaudRate, MynBits, MynEvent, Mynstop); //设置串口
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

int CUARTDataPort::SendData( unsigned char * buf ,int Send_len)
{
    int ret;
    ret = write(uart_fd,buf,Send_len);
    return ret;

}

int CUARTDataPort::GetData( unsigned char * buf,int rec_len )
{
    int ret;
    ret = read(uart_fd,buf,rec_len);
    return ret;

}
//-------------------------------------------
// Function:
//      close and release the resource
//
void CUARTDataPort::Invalid()
{
    close( uart_fd );

    cout << "close the uart. " << endl;

    uart_fd = -1;
}


