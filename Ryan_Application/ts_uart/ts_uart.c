#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX 终端控制定义*/
#include <errno.h>      /*错误号定义*/
#include <string.h>
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	/* 获取fd串口对应的termios结构体，这步主要是查询串口是否启动正常 */
	if  ( tcgetattr( fd,&oldtio)  !=  0) {
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
	switch( nSpeed )
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
	tcflush(fd,TCIFLUSH);
	/* 设置新配置 */
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
//	printf("set done!\n\r");
	return 0;
}

int main()
{
    int fd,wr_static,i=10;
	char *uart1 = "/dev/ttyS1";		//ttyS代表板上的串口
	char *buffer = "hello v3s world!\n";
    char read_buffer[10] = {'\n'};
	printf("\r\nv3ss uart1 writetest start\r\n");
	/* 打开串口，可读写，不将该设备作为此进程的控制终端，非阻塞方式操作 *///| O_BLOCK
	if((fd = open(uart1, O_RDWR ))<0){
		printf("open %s is failed",uart1);
	}
	else{
		printf("open %s is success\n",uart1);
		set_opt(fd, 115200, 8, 'N', 1); //设置串口
    }
		while(i--)
		{
			wr_static = write(fd,buffer, strlen(buffer));   //发送串口流数据
			if(wr_static<0)
				printf("write failed\n");
			else{
				printf("wr_static is %d\n",wr_static);
			}

			while(read(fd,read_buffer,5)==0);
			printf("%s",read_buffer);

		}

	close(fd);	//释放串口设备资源

    return 0;
}
