/////////////////////////////////////////////////////////////////
//      UDP Packet Capture Function
//

#ifndef __UDP_PACKET_CAPTURE_H_20030806__
#define __UDP_PACKET_CAPTURE_H_20030806__

#pragma pack(push,4)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <MyThread.h>
#include <MySyncObj.h>
#include <MyList.h>
#include <BufPacket4C.h>
#include <IPRecDrvInterface.h>
#include <LookaheadPacketMgr.h>
#include <time.h>

class CIP_Packet : public CBufPacket4C< IMyBufPacket >
{
public:
	CIP_Packet() : CBufPacket4C< IMyBufPacket >( 0 )
    {
    	bzero( &m_IP_Param, sizeof(m_IP_Param) );
        m_tLastAccessTime = time(NULL);
    }
	virtual void SafeDelete(){ delete this; }
    DWORD & Admin_AccessReservedBytes()
    {
    	return CBufPacket4C< IMyBufPacket >::Admin_AccessReservedBytes();
    }

public:
	sockaddr_in		m_IP_Param;
    time_t          m_tLastAccessTime;
};

class CUDPCaptureThread
: public CMyThread,
  public CLookaheadPacketMgr< CIP_Packet >
{
public:
    CUDPCaptureThread();
    ~CUDPCaptureThread();

    enum { UDP_BUF_MAX_SIZE = 2048 };

public:
	virtual int ExitInstance();					//	返回参数
	virtual bool InitInstance();				//	是否成功
	virtual void Run();							//	运行主体
	virtual void Delete();						//	safe delete this

public:
	bool Create( const char * lpszLocalBind, int nCatchCount = 128 );

private:
	bool Set_Promisc_Mode(); 

protected:
    int m_hSocket;                   // Socket handle
    char m_szLocalBindIP[33];        // local bind IP address
};

#pragma pack(pop)

#endif // __UDP_PACKET_CAPTURE_H_20030806__

