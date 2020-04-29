// UDPDataPort.cpp: implementation of the CUDPDataPort class.
//
//////////////////////////////////////////////////////////////////////
//  2003-11-21  修改 Initialize，若绑定失败，重新绑定
//	2002.12.11	原来异步读取为 256, 现改成 128 个
//	2002.6.17	修改 Initialize，Windows 98 下，不能将 UDP 缓冲区设成用户提供的缓冲区。

#include "stdafx.h"
#include "resource.h"
#include "UDPDataPort.h"
#include "MyThread.h"

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32  


/////////////////////////////////////////////////////////////////////
//	创建 UDP 接收端口
//	返回参数
//		NULL			失败
//		其他			成功
extern "C" CSrcDataPort * WINAPI CreateUDPDataPort()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CUDPDataPort * pUDPPort = new CUDPDataPort;
	if( NULL == pUDPPort )
		return NULL;
	pUDPPort->AddRef();

	return static_cast<CSrcDataPort*>(pUDPPort);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUDPDataPort::CUDPDataPort()
{
	m_hSocket = INVALID_SOCKET;
	m_nItemCount = 0;					//	异步操作的个数
	m_nCurReadItemCount = 0;			//	当前读取的记录数
	m_bNeedToCallCleanUp = FALSE;		//	不须调用 CleanUp
	RtlZeroMemory( &m_mrMReq, sizeof(m_mrMReq) );
}

CUDPDataPort::~CUDPDataPort()
{
	Invalidate();
}

void CUDPDataPort::SafeDelete()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	delete this;
}

///////////////////////////////////////////
//	初始化
//	入口参数
//		lpszIP			待接收的 IP 地址
//		nPort			接收的端口
//		lpszLocalBind	本地绑定的网卡
//		nCount			同时异步读取的顺序，缺省－1，自动确认
//	返回参数
//		TRUE			成功
//		FALSE			失败
//	错误值有:
//		
//	修改记录：
//		2003-11-21 若绑定失败，则重新绑定
BOOL	CUDPDataPort::Initialize( LPCSTR lpszIP, UINT nPort, LPCSTR lpszLocalBind, int nCount )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

#ifdef __USE_FOR_LINUX__			// compile for linux
	printf("CUDPDataPort::Initialize, IP=%s, PORT=%d, LOCALIP=%s\n", lpszIP, nPort, lpszLocalBind );
#endif //__USE_FOR_LINUX__

	ASSERT( lpszIP && *lpszIP && nPort >= 1024 );
	if( nCount < 0 )
		nCount = DEFAULT_ASYN_COUNT;		//	自动确定

	m_nItemCount = 0;
	m_nLastError = 0;

	WSADATA wsaData;
	m_nLastError = m_drv.WSAStartup(0x0202,&wsaData);
	if( m_nLastError )
	{
#ifdef __USE_FOR_LINUX__			// compile for linux
		printf("WSAStartup failed with error %d\n", m_nLastError );
#endif // __USE_FOR_LINUX__
		return FALSE;
	}
	m_bNeedToCallCleanUp = TRUE;

	m_hSocket = m_drv.WSASocket( AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if( m_hSocket == INVALID_SOCKET )
	{
		m_nLastError = m_drv.WSAGetLastError();
#ifdef __USE_FOR_LINUX__			// compile for linux
		printf("Failed to get a socket %d\n", m_nLastError );
#endif // __USE_FOR_LINUX__
		return FALSE;
	}

	BOOL bMultipleApps = TRUE;		/* allow reuse of local port if needed */
	m_drv.setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&bMultipleApps, sizeof(BOOL) );

	struct sockaddr_in  local;
	ZeroMemory( &local, sizeof(local) );
	local.sin_family      = AF_INET;
	local.sin_port        = m_drv.htons( nPort );

#ifdef __USE_FOR_LINUX__			// compile for linux
	local.sin_addr.s_addr = m_drv.inet_addr( lpszIP );	
#else
	if( lpszLocalBind && lpszLocalBind[0] )
		local.sin_addr.s_addr = m_drv.inet_addr( lpszLocalBind );
	else
		local.sin_addr.s_addr = m_drv.htonl( INADDR_ANY );
#endif // __USE_FOR_LINUX__	

	if( m_drv.bind( m_hSocket, (SOCKADDR *)&local, sizeof(local) ) != 0 )
	{													//  2003-11-21 add, rebind if bind failed
		local.sin_addr.s_addr = m_drv.htonl( INADDR_ANY );
		m_drv.bind( m_hSocket, (SOCKADDR *)&local, sizeof(local) );
#ifndef __USE_FOR_LINUX__			// compile for linux
		lpszLocalBind = NULL;
#endif // __USE_FOR_LINUX__
	}

	ZeroMemory( &m_mrMReq, sizeof(m_mrMReq) );
	m_mrMReq.imr_multiaddr.s_addr = m_drv.inet_addr(lpszIP);
	if( lpszLocalBind && lpszLocalBind[0] )
		m_mrMReq.imr_interface.s_addr = m_drv.inet_addr( lpszLocalBind );
	else
		m_mrMReq.imr_interface.s_addr = m_drv.htonl( INADDR_ANY );

	if(m_drv.setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char FAR *)&m_mrMReq, sizeof(m_mrMReq)) < 0)
	{
		m_nLastError = m_drv.WSAGetLastError();
#ifdef __PRINT_DEBUG_MSG_RELEASE__
		printf("Join Multicast failed. Error code=%d\n", m_nLastError );
#endif // __PRINT_DEBUG_MSG_RELEASE__
		return FALSE;
	}

	// Join the multicast group.  Note that sockM is not used 
    // to send or receive data. It is used when you want to 
    // leave the multicast group. You simply call closesocket() 
    // on it.

	strncpy( m_szIPAddress, lpszIP, sizeof(m_szIPAddress)-1 );
	m_wPort = nPort;

	struct sockaddr_in		remote;
	remote.sin_family      = AF_INET;
    remote.sin_port        = m_drv.htons( nPort );
    remote.sin_addr.s_addr = m_drv.inet_addr( lpszIP );

#ifndef __USE_FOR_LINUX__			// compile for linux
	if( CMyThread::IsWinNT() )			//	2002.6.17 修改添加，只有 Windows NT 才使用用户提供的缓冲区
	{									
		int nRcvBuf = 0;				//	完全依靠提供的缓冲区
		m_drv.setsockopt( m_hSocket, SOL_SOCKET , SO_RCVBUF, (char*)&nRcvBuf, sizeof(int) );
	}
	else
#endif // __USE_FOR_LINUX__
	{									//	2002.6.17，Windows 98 他妈的不支持用户提供的缓冲区，只能使用系统提供的缓冲区，kao
		int nRcvBuf = 1536*96;			//	1.5K
		for(int i=0; i<10; i++)
		{
			m_drv.setsockopt( m_hSocket, SOL_SOCKET , SO_RCVBUF, (char*)&nRcvBuf, sizeof(int) );

			int nBufSize = sizeof(int);
			int nBufRet = 0;
			int nRetVal = m_drv.getsockopt( m_hSocket, SOL_SOCKET , SO_RCVBUF, (char*)&nBufRet, &nBufSize );
			if( 0 == nRetVal && nBufRet == nRcvBuf )
				break;
	#ifdef _DEBUG
			else
			{
				TRACE("getsockopt, last error = %d\n",m_drv.WSAGetLastError());
			}	
	#endif // _DEBUG
			nRcvBuf -= 1536*5;
		}
	}

	TRY
	{
		m_asynobjs.SetSize( nCount );
	}
	CATCH( CMemoryException, e )
	{
		m_asynobjs.RemoveAll();
		m_nLastError = ERROR_NOT_ENOUGH_MEMORY;
		return FALSE;
	}
	END_CATCH

	RtlZeroMemory( m_asynobjs.GetData(), sizeof(UDPOVERLAP)*nCount );

	for( int i=0; i<nCount; i++ )
	{
		UDPOVERLAP & obj = m_asynobjs[i];
		obj.m_hEvent = m_drv.WSACreateEvent();
		if( WSA_INVALID_EVENT == obj.m_hEvent )
		{
			m_nLastError = m_drv.WSAGetLastError();
			Invalidate();
			return FALSE;				//	失败
		}
		obj.m_overlapped.hEvent = obj.m_hEvent;
	}
	m_nItemCount = nCount;

#ifdef __PRINT_DEBUG_MSG_RELEASE__
	printf("init UDP receiver socket succ, %s : %d\n", lpszIP, nPort );
#endif // __PRINT_DEBUG_MSG_RELEASE__

	return TRUE;
}

//////////////////////////////////////////////
///功能:
///		获取当前最大底项目数
///入口参数:
///		无
///返回参数:
///		当前使用底单元数
int		CUDPDataPort::GetItemCount()
{
	return m_nItemCount;
}

//////////////////////////////////////////////
//功能:
//		使之失效，释放全部资源
//入口参数:
//		无
//返回参数:
//		无
void	CUDPDataPort::Invalidate()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if( FALSE == m_bNeedToCallCleanUp )
		return;
	m_bNeedToCallCleanUp = FALSE;

	m_nLastError = 0;	
	int nCount = m_asynobjs.GetSize();
	for(int i=0; i<nCount; i++)
	{
		UDPOVERLAP & obj = m_asynobjs[i];
		if( WSA_INVALID_EVENT == obj.m_hEvent )
			continue;
		m_drv.WSACloseEvent( obj.m_hEvent );
	}

	if(m_drv.setsockopt (m_hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char FAR *)&m_mrMReq, sizeof(m_mrMReq)) < 0)
		return;

	if( INVALID_SOCKET != m_hSocket )
		m_drv.closesocket( m_hSocket );
	m_hSocket = INVALID_SOCKET;

	m_drv.WSACleanup();

	m_asynobjs.RemoveAll();
	m_nItemCount = 0;
}

//////////////////////////////////////////////
//功能:
//		获取当前的事件句柄，用来等待
//入口参数:
//		hNo				句柄，由 ReadAsyn 返回
//返回参数:
//		事件句柄
//		INVALID_HANDLE_VALUE 失败，一般不出现，除非 Initialize 失败后还调用该函数
HANDLE 	CUDPDataPort::GetEventHandle(SDP_HANDLE hNo)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ASSERT( hNo > 0 && hNo <= m_nItemCount );
	if( hNo <= 0 || hNo > m_nItemCount )
		return INVALID_HANDLE_VALUE;

	ASSERT( m_asynobjs.GetSize() == m_nItemCount );
	UDPOVERLAP & obj = m_asynobjs[ hNo-1 ];
	return obj.m_hEvent;
}

//////////////////////////////////////////////
//功能:
//		异步读取数据
//入口参数:
//		pBuf			缓冲区地址
//		dwBufSize		缓冲区大小
//		pdwByteRead		实际读取的字节数,不能为 NULL
//返回参数:
//		0				成功，pdwByteRead 存放实际的字节数
//		> 0				延迟操作，需要调用 GetOverlappedResult
//		< 0				未知错误，参照 m_nLastError
SDP_HANDLE	CUDPDataPort::ReadAsyn(	PBYTE pBuf, DWORD dwBufSize,PDWORD pdwByteRead )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT( m_nCurReadItemCount >= 0 && m_nCurReadItemCount < m_nItemCount );
	ASSERT( pBuf && dwBufSize && pdwByteRead );
	
	UDPOVERLAP & obj = m_asynobjs[ m_nCurReadItemCount ];
	if( obj.m_bIsPending )
	{
		m_nLastError = ERROR_NO_SPOOL_SPACE;
		return -1;			
	}

	obj.m_dwByteRead = 0;
	obj.m_dwFlags = 0;
	obj.m_wsaDataBuf.buf = (char*)pBuf;
	obj.m_wsaDataBuf.len = dwBufSize;
	obj.m_bIsPending = FALSE;
	m_drv.WSAResetEvent( obj.m_hEvent );

	m_nLastError = m_drv.WSARecv( m_hSocket, &obj.m_wsaDataBuf, 1, &obj.m_dwByteRead, &obj.m_dwFlags, &obj.m_overlapped, NULL );
	if( 0 == m_nLastError )
	{									//	成功
		if( pdwByteRead )
			*pdwByteRead = obj.m_dwByteRead;
#ifdef _DEBUG
		obj.m_wsaDataBuf.buf = NULL;
#endif // _DEBUG
		return 0;						//	成功，没有延迟操作
	}
	
	m_nLastError = m_drv.WSAGetLastError();
	ASSERT( WSA_IO_PENDING == m_nLastError );
	if( WSA_IO_PENDING == m_nLastError )
	{									//	延迟操作
		obj.m_bIsPending = TRUE;
		m_nCurReadItemCount ++;
		int nRetVal = m_nCurReadItemCount;
		if( m_nCurReadItemCount >= m_nItemCount )
			m_nCurReadItemCount = 0;
		return nRetVal;
	}
	else
		obj.m_dwByteRead = 0;			//	其他错误，放弃

	return -1;
}

//////////////////////////////////////////////
///功能:
///		同步读取数据
///入口参数:
//		pBuf			缓冲区地址
//		dwBufSize		缓冲区大小
//		pdwByteRead		实际读取的字节数,不能为 NULL
///返回参数:
///		TRUE			成功
///		FALSE			失败
BOOL	CUDPDataPort::ReadSync( PBYTE pBuf, DWORD dwBufSize, PDWORD pdwByteRead )
{
	ASSERT( pBuf && dwBufSize );
	if( NULL == pBuf || 0 == dwBufSize )
	{
		m_nLastError = ERROR_NOT_ENOUGH_MEMORY;
		return FALSE;
	}
	WSABUF	wsBuf = { dwBufSize, (char*)pBuf };
	DWORD dwFlags = 0;
	DWORD dwByteRead = 0;
	if( NULL == pdwByteRead )
		pdwByteRead = &dwByteRead;

	m_nLastError = m_drv.WSARecv( m_hSocket, &wsBuf, 1, pdwByteRead, &dwFlags, NULL, NULL );
	if( 0 == m_nLastError )
		return TRUE;
	m_nLastError = m_drv.WSAGetLastError();
	return FALSE;
}

//////////////////////////////////////////////
//功能:
//		获取延迟操作的状态
//入口参数:
//		hReadNo			操作句柄，同 ReadAsyn 返回参数
//		pdwByteRead		输出实际的读取的字节数，不能为 NULL
//		pWait			是否等待操作结束
//返回参数:
//		TRUE			成功
//		FALSE			失败,可能是还没有完成
BOOL	CUDPDataPort::GetOverlappedResult( SDP_HANDLE hReadNo, PDWORD pdwByteRead, BOOL bWait )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT( hReadNo > 0 && hReadNo <= m_nItemCount );
	if( hReadNo <= 0 || hReadNo > m_nItemCount )
		return FALSE;					//	失败

	UDPOVERLAP & obj = m_asynobjs[ hReadNo-1 ];
	m_nLastError = 0;
	ASSERT( obj.m_bIsPending );

	if( m_drv.WSAGetOverlappedResult( m_hSocket, &obj.m_overlapped, &obj.m_dwByteRead, bWait, &obj.m_dwFlags ) )
	{
		if( pdwByteRead )
			*pdwByteRead = obj.m_dwByteRead;
		obj.m_bIsPending = FALSE;				//	标记该单元已经成功读取
		return TRUE;
	}

	m_nLastError = m_drv.WSAGetLastError();
	if( WSA_IO_INCOMPLETE == m_nLastError )
		return FALSE;							//	还没有完成

	obj.m_bIsPending = FALSE;					//	失败，放弃
	return FALSE;
}

//////////////////////////////////////////////
//功能:
//		取消以前所有的操作
//入口参数:
//		hReadNo			放弃读取的句柄
//返回参数:
//		无
//注：
//		取消所有的操作意味着，必须释放所有的内存资源
void	CUDPDataPort::CancelAsynRead(SDP_HANDLE hReadNo)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT( hReadNo > 0 && hReadNo <= m_nItemCount );
	if( hReadNo <= 0 || hReadNo > m_nItemCount )
		return;
	UDPOVERLAP & obj = m_asynobjs[ hReadNo-1 ];
	obj.m_bIsPending = FALSE;
}

//////////////////////////////////////////////
//功能:
//		我能执行异步操作吗？
//入口参数:
//		无
//返回参数:
//		TRUE			还有空间，可以
//		FALSE			不能
BOOL	CUDPDataPort::CanIDoReadAsync()
{
	UDPOVERLAP & obj = m_asynobjs[ m_nCurReadItemCount ];
	return FALSE == obj.m_bIsPending;
}