// MyWS2_32.h: interface for the CMyWS2_32 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYWS2_32_H__917BFD28_C6BB_49CF_9973_DC07DEF3EE8D__INCLUDED_)
#define AFX_MYWS2_32_H__917BFD28_C6BB_49CF_9973_DC07DEF3EE8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define INCL_WINSOCK_API_TYPEDEFS 1

#include <Winsock2.h>

class CMyWS2_32  
{
public:
	int bind(SOCKET s,const struct sockaddr FAR * name,int namelen );
	WSAEVENT WSACreateEvent(void);

	int gethostname( char FAR *name, int namelen );			//	2002.4.22 Ìí¼Ó
	struct hostent FAR * gethostbyname( const char FAR *name );

	int setsockopt(SOCKET s,int level,int optname,const char FAR * optval,int optlen);
	u_short htons( u_short hostshort );	
	u_long htonl (u_long hostlong);
	unsigned long inet_addr(const char FAR * cp);
	SOCKET WSAJoinLeaf(SOCKET s,const struct sockaddr FAR * name,int namelen,LPWSABUF lpCallerData,LPWSABUF lpCalleeData,LPQOS lpSQOS,LPQOS lpGQOS,DWORD dwFlags);
	int getsockopt(SOCKET s,int level,int optname,char FAR * optval,int FAR * optlen);
	BOOL WSACloseEvent(WSAEVENT hEvent);
	int closesocket(SOCKET s);
	int WSACleanup(void);
	BOOL WSAGetOverlappedResult(SOCKET s,LPWSAOVERLAPPED lpOverlapped,LPDWORD lpcbTransfer,BOOL fWait,LPDWORD lpdwFlags);
	DWORD WSAWaitForMultipleEvents(DWORD cEvents,const WSAEVENT FAR * lphEvents,BOOL fWaitAll,DWORD dwTimeout,BOOL fAlertable);
	BOOL WSAResetEvent(WSAEVENT hEvent);
	int WSARecv(SOCKET s,LPWSABUF lpBuffers,DWORD dwBufferCount,LPDWORD lpNumberOfBytesRecvd,LPDWORD lpFlags,LPWSAOVERLAPPED lpOverlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	int WSAGetLastError(void);
	int WSAConnect(SOCKET s,const struct sockaddr FAR *name,int namelen,LPWSABUF lpCallerData,LPWSABUF lpCalleeData,LPQOS lpSQOS,LPQOS lpGQOS );

	SOCKET WSASocket(int af, int type,int protocol,LPWSAPROTOCOL_INFO lpProtocolInfo,GROUP g,DWORD dwFlags );
	int WSAStartup(WORD wVersionRequested,LPWSADATA lpWSAData);

	CMyWS2_32();
	virtual ~CMyWS2_32();

private:
	void LoadLib();

	LPFN_BIND			m_pfnBind;
	LPFN_WSACREATEEVENT	m_pfnCreateWSCreateEvent;
	LPFN_SETSOCKOPT		m_pfnSetSockOpt;
	LPFN_HTONS			m_pfnhtons;
	LPFN_INET_ADDR		m_pfninet_addr;
	LPFN_WSAJOINLEAF	m_pfnWSAJoinLeaf;
	LPFN_GETSOCKOPT		m_pfnGetSockOpt;
	LPFN_WSACLOSEEVENT	m_pfnWSACloseEvent;
	LPFN_CLOSESOCKET	m_pfnCloseSocket;
	LPFN_WSACLEANUP		m_pfnWSCleanUp;
	LPFN_WSAGETOVERLAPPEDRESULT		m_pfnGetOverlappedResult;
	LPFN_WSAWAITFORMULTIPLEEVENTS	m_pfnWaitForMultiEvent;
	LPFN_WSARESETEVENT	m_pfnResetEvent;
	LPFN_WSARECV		m_pfnRecv;
	LPFN_WSAGETLASTERROR	m_pfnGetLastError;
	LPFN_WSASOCKETA		m_pfnSocket;
	LPFN_WSASTARTUP		m_pfnStartup;
	LPFN_GETHOSTNAME	m_pfnGetHostName;
	LPFN_GETHOSTBYNAME	m_pfnGetHostByName;
	LPFN_WSACONNECT		m_pfnWSAConnect;
	LPFN_HTONL			m_pfnhtonl;			// 2003-3-13 Ìí¼Ó

	HMODULE	m_hDll;
};

#endif // !defined(AFX_MYWS2_32_H__917BFD28_C6BB_49CF_9973_DC07DEF3EE8D__INCLUDED_)
