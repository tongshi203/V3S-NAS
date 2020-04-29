//---------------------------------------------------------------------------

#ifndef __UDPDataPortLinux_H_20040405__
#define __UDPDataPortLinux_H_20040405__

#ifndef _WIN32
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#else
	#define INCL_WINSOCK_API_TYPEDEFS 1
	#include <Winsock2.h>
	#include <WS2TCPIP.H>
#endif //_WIN32

#include "IPData.h"

//---------------------------------------------------------------------------
class CUDPDataPort
{
public:
	CUDPDataPort();
    ~CUDPDataPort();

    bool Initialize( const char * lpszDstIP, int nPort, const char* lpszLocalBindIP,COneDataPortItem * pDataPortItem );
    void Invalid();
    operator int(){ return m_hSocket; }
public:
	int m_hSocket;
    struct ip_mreq	m_mreq;
	struct sockaddr_in m_DstIP;
    COneDataPortItem * m_pDataPortItem;

private:
	void AdjustRcvBufferSize();
#ifndef _WIN32
	bool EnableAllMulti(const char * pszLocalBindIP);
#endif //_WIN32
};

#endif		// __UDPDataPortLinux_H_20040405__
