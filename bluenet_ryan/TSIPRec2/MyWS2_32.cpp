// MyWS2_32.cpp: implementation of the CMyWS2_32 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyWS2_32.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyWS2_32::CMyWS2_32()
{
	m_pfnBind = NULL;
	m_pfnCreateWSCreateEvent = NULL;
	m_pfnSetSockOpt = NULL;
	m_pfnhtons = NULL;
	m_pfninet_addr = NULL;
	m_pfnWSAJoinLeaf = NULL;
	m_pfnGetSockOpt = NULL;
	m_pfnWSACloseEvent = NULL;
	m_pfnCloseSocket = NULL;
	m_pfnWSCleanUp = NULL;
	m_pfnGetOverlappedResult = NULL;
	m_pfnWaitForMultiEvent = NULL;
	m_pfnResetEvent = NULL;
	m_pfnRecv = NULL;
	m_pfnGetLastError = NULL;
	m_pfnSocket = NULL;
	m_pfnStartup = NULL;
	m_pfnGetHostName = NULL;
	m_pfnGetHostByName = NULL;
	m_pfnWSAConnect = NULL;
	m_pfnhtonl = NULL;

	m_hDll = NULL;
	LoadLib();
}

CMyWS2_32::~CMyWS2_32()
{
	if( m_hDll )
		::FreeLibrary( m_hDll );
}

void CMyWS2_32::LoadLib()
{
	m_hDll = ::LoadLibrary( "WS2_32.DLL" );
	if( NULL == m_hDll )
		return;					//	失败
	m_pfnBind = (LPFN_BIND)::GetProcAddress( m_hDll, "bind" );
	m_pfnCreateWSCreateEvent= (LPFN_WSACREATEEVENT) ::GetProcAddress( m_hDll, "WSACreateEvent" );
	m_pfnSetSockOpt=(LPFN_SETSOCKOPT)::GetProcAddress( m_hDll, "setsockopt" );
	m_pfnhtons =(LPFN_HTONS)::GetProcAddress( m_hDll,"htons" );
	m_pfninet_addr=(LPFN_INET_ADDR)::GetProcAddress(m_hDll,"inet_addr");
	m_pfnWSAJoinLeaf=(LPFN_WSAJOINLEAF)::GetProcAddress(m_hDll,"WSAJoinLeaf");
	m_pfnGetSockOpt=(LPFN_GETSOCKOPT)::GetProcAddress(m_hDll,"getsockopt");
	m_pfnWSACloseEvent=(LPFN_WSACLOSEEVENT)::GetProcAddress(m_hDll,"WSACloseEvent");
	m_pfnCloseSocket=(LPFN_CLOSESOCKET)::GetProcAddress(m_hDll,"closesocket");
	m_pfnWSCleanUp=(LPFN_WSACLEANUP)::GetProcAddress(m_hDll,"WSACleanup");
	m_pfnGetOverlappedResult=(LPFN_WSAGETOVERLAPPEDRESULT)::GetProcAddress(m_hDll,"WSAGetOverlappedResult");
	m_pfnWaitForMultiEvent=(LPFN_WSAWAITFORMULTIPLEEVENTS)::GetProcAddress(m_hDll,"WSAWaitForMultipleEvents");
	m_pfnResetEvent=(LPFN_WSARESETEVENT)::GetProcAddress(m_hDll,"WSAResetEvent");
	m_pfnRecv=(LPFN_WSARECV)::GetProcAddress(m_hDll,"WSARecv");
	m_pfnGetLastError=(LPFN_WSAGETLASTERROR)::GetProcAddress( m_hDll, "WSAGetLastError" );
	m_pfnSocket = (LPFN_WSASOCKETA)::GetProcAddress( m_hDll,"WSASocketA" );
	m_pfnStartup = (LPFN_WSASTARTUP)::GetProcAddress( m_hDll, "WSAStartup" );
	m_pfnGetHostName = (LPFN_GETHOSTNAME)::GetProcAddress( m_hDll, "gethostname" );
	m_pfnGetHostByName = (LPFN_GETHOSTBYNAME)::GetProcAddress( m_hDll, "gethostbyname" );
	m_pfnWSAConnect = (LPFN_WSACONNECT)::GetProcAddress( m_hDll, "WSAConnect" );
	m_pfnhtonl = (LPFN_HTONL)::GetProcAddress( m_hDll, "htonl" );
}

int CMyWS2_32::WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData)
{
	if( NULL == m_pfnStartup )
	{
		LoadLib();
		if( NULL == m_pfnStartup )
			return WSAVERNOTSUPPORTED;
	}
	return m_pfnStartup( wVersionRequested, lpWSAData );
}

SOCKET CMyWS2_32::WSASocket(int af, int type, int protocol, LPWSAPROTOCOL_INFO lpProtocolInfo, GROUP g, DWORD dwFlags)
{
	if( NULL == m_pfnSocket )
		return INVALID_SOCKET ;
	return m_pfnSocket( af, type, protocol, lpProtocolInfo, g, dwFlags);
}

int CMyWS2_32::bind(SOCKET s,const struct sockaddr FAR * name,int namelen )
{
	if( NULL == m_pfnBind )
		return SOCKET_ERROR;
	return m_pfnBind(s,name,namelen);
}

WSAEVENT CMyWS2_32::WSACreateEvent(void)
{
	if( NULL == m_pfnCreateWSCreateEvent )
		return WSA_INVALID_EVENT;
	return m_pfnCreateWSCreateEvent();
}

int CMyWS2_32::setsockopt(SOCKET s,int level,int optname,const char FAR * optval,int optlen)
{
	if( NULL == m_pfnSetSockOpt )
		return SOCKET_ERROR ;
	return m_pfnSetSockOpt( s,level,optname,optval,optlen);
}

u_short CMyWS2_32::htons( u_short hostshort )
{
	if( NULL == m_pfnhtons )
		return 0;
	return m_pfnhtons( hostshort );
}

unsigned long CMyWS2_32::inet_addr(const char FAR * cp)
{
	if( NULL == m_pfninet_addr )
		return 0;
	return m_pfninet_addr( cp );
}

SOCKET CMyWS2_32::WSAJoinLeaf(SOCKET s,const struct sockaddr FAR * name,int namelen,LPWSABUF lpCallerData,LPWSABUF lpCalleeData,LPQOS lpSQOS,LPQOS lpGQOS,DWORD dwFlags)
{
	if( NULL == m_pfnWSAJoinLeaf )
		return INVALID_SOCKET ;
	return m_pfnWSAJoinLeaf(s,name,namelen,lpCallerData,lpCalleeData,lpSQOS,lpGQOS,dwFlags);
}

int CMyWS2_32::getsockopt(SOCKET s,int level,int optname,char FAR * optval,int FAR * optlen)
{
	if( NULL == m_pfnGetSockOpt )
		return SOCKET_ERROR;
	return m_pfnGetSockOpt( s,level,optname,optval,optlen);
}

BOOL CMyWS2_32::WSACloseEvent(WSAEVENT hEvent)
{
	if( NULL == m_pfnWSACloseEvent )
		return FALSE;
	return m_pfnWSACloseEvent( hEvent );
}

int CMyWS2_32::closesocket(SOCKET s)
{
	if( NULL == m_pfnCloseSocket )
		return SOCKET_ERROR ;
	return m_pfnCloseSocket( s );
}

int CMyWS2_32::WSACleanup(void)
{
	if( NULL == m_pfnWSCleanUp )
		return SOCKET_ERROR;
	return m_pfnWSCleanUp();
}

BOOL CMyWS2_32::WSAGetOverlappedResult(SOCKET s,LPWSAOVERLAPPED lpOverlapped,LPDWORD lpcbTransfer,BOOL fWait,LPDWORD lpdwFlags)
{
	if( NULL == m_pfnGetOverlappedResult )
		return FALSE;
	return m_pfnGetOverlappedResult(s,lpOverlapped,lpcbTransfer,fWait,lpdwFlags);
}

DWORD CMyWS2_32::WSAWaitForMultipleEvents(DWORD cEvents,const WSAEVENT FAR * lphEvents,BOOL fWaitAll,DWORD dwTimeout,BOOL fAlertable)
{
	if( NULL == m_pfnWaitForMultiEvent )
		return WSA_WAIT_FAILED;
	return m_pfnWaitForMultiEvent( cEvents,lphEvents,fWaitAll,dwTimeout,fAlertable);
}

BOOL CMyWS2_32::WSAResetEvent(WSAEVENT hEvent)
{
	if( NULL == m_pfnResetEvent )
		return FALSE;
	return m_pfnResetEvent( hEvent );
}

int CMyWS2_32::WSARecv(SOCKET s,LPWSABUF lpBuffers,DWORD dwBufferCount,LPDWORD lpNumberOfBytesRecvd,LPDWORD lpFlags,LPWSAOVERLAPPED lpOverlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	if( NULL == m_pfnRecv )
		return SOCKET_ERROR;
	return m_pfnRecv(s,lpBuffers,dwBufferCount,lpNumberOfBytesRecvd,lpFlags,lpOverlapped,lpCompletionRoutine);
}

int CMyWS2_32::WSAGetLastError(void)
{
	if( NULL == m_pfnGetLastError )
		return SO_ERROR;
	return m_pfnGetLastError();
}

//	2002.4.22 添加
int CMyWS2_32::gethostname( char FAR *name, int namelen )
{
	if( NULL == m_pfnGetHostName )
		return SO_ERROR;
	return m_pfnGetHostName( name, namelen );
}

struct hostent FAR * CMyWS2_32::gethostbyname( const char FAR *name )
{
	if( NULL == m_pfnGetHostByName )
		return NULL;
	return m_pfnGetHostByName( name );
}

int CMyWS2_32::WSAConnect(SOCKET s,const struct sockaddr FAR *name,int namelen,LPWSABUF lpCallerData,LPWSABUF lpCalleeData,LPQOS lpSQOS,LPQOS lpGQOS )
{
	if( NULL == m_pfnWSAConnect )
		return SO_ERROR;
	return m_pfnWSAConnect( s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS );
}

///-------------------------------------------------------
/// 2003-3-13
/// 功能：
///		转换
/// 入口参数：
///
/// 返回参数：
///
u_long CMyWS2_32::htonl (u_long hostlong)
{
	ASSERT( m_pfnhtonl );
	if( NULL == m_pfnhtonl )
		return hostlong;
	return m_pfnhtonl( hostlong );
}