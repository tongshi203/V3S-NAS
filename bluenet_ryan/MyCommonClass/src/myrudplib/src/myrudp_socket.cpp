/***********************************************************************
 *
 *	My RUDP Socket functions
 *
 *	Chen Yongjian @ Zhoi
 *	2015.1.12 @ Xi'an
 *
 *
 ***********************************************************************/
#ifndef __USE_XOPEN2K
	#define __USE_XOPEN2K
#endif // __USE_XOPEN2K

#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sched.h>
#include <sys/select.h>
#include <unistd.h>
#include <assert.h>
#include <algorithm>
#include <time.h>

#include <my_log_print.h>
#include <my_pthread_mutex_help.h>
#include <mydatatype.h>
#include <crc.h>

#include "myrudp_socket.h"
#include "myrudp_cmd_value.h"
#include "myrudp_dbgprint.h"

#ifdef __MYRUDP_USE_OPENSSL__
	#include "myrudp_openssl_svr_helper.h"
	#include <openssl/rand.h>
#endif // #ifdef __MYRUDP_USE_OPENSSL__

////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
extern void DbgDumpData( const char *pszTitle, const uint8_t * pBuf, int nLen );
#endif // _DEBUG

////////////////////////////////////////////////////////////////////////

#ifdef __MYRUDP_USE_OPENSSL__
static int					s_nMyRUDPOpenSSLServerRefCounter = 0;
CMyRUDPOpenSSLServerHelper	g_MyRUDPOpenSSLServerHelper;

////////////////////////////////////////////////////////////////////////
static void MyRUDPOpenSSLHelper_Initialize()
{
	if( s_nMyRUDPOpenSSLServerRefCounter )
		return;
	time_t tNow = time(NULL);
	RAND_seed( &tNow, sizeof(time_t) );
	s_nMyRUDPOpenSSLServerRefCounter = 1;
	g_MyRUDPOpenSSLServerHelper.Initialize();
}

#endif //#ifdef __MYRUDP_USE_OPENSSL__

//--------------------------------------------------------------
/** CYJ 2015-03-02
 *
 *	Create Server driver object
 *
 * @return	My RUDP server driver object
 */
CMyRUDP_ServerDrv * MyRUDP_CreateServerDrv()
{
	CMyRUDP_Socket * pRetVal = new CMyRUDP_Socket;
	if( NULL == pRetVal )
		return NULL;

	return static_cast< CMyRUDP_ServerDrv * >( pRetVal );
}

CMyRUDP_Socket::CMyRUDP_Socket()
 :m_InDataBuf(0)
{
#ifdef __MYRUDP_USE_OPENSSL__
	// 2015.4.19 CYJ Add
	MyRUDPOpenSSLHelper_Initialize();
#endif // #ifdef __MYRUDP_USE_OPENSSL__

	m_hThread_ReadData = 0;
	m_hThread_ProcessData = 0;
	m_hThread_Timer = 0;
	m_nThreadState = THREAD_STATE_IDLE;

	m_pThreadPoolObj = NULL;
	memset( &m_LocalBindIP, 0, sizeof(m_LocalBindIP) );

	m_hSocket = -1;							// socket to receive and send data
	pthread_mutex_init( &m_SyncObj_Socket, NULL);

	pthread_mutex_init( &m_SyncObj_FastTimer, NULL );

#ifdef __USE_XOPEN2K
	pthread_condattr_init( &m_Event_FastTimer_Attr );
	pthread_condattr_setclock( &m_Event_FastTimer_Attr, CLOCK_MONOTONIC );
	pthread_cond_init( &m_Event_FastTimer, &m_Event_FastTimer_Attr );
#else
	pthread_cond_init( &m_Event_FastTimer, NULL );
#endif

	// peer object array
	memset( m_apPeerObjects, 0, sizeof(m_apPeerObjects) );
	m_pEventObj = NULL;
	memset( m_anFastTimerSessionID, 0, sizeof(m_anFastTimerSessionID) );
	m_bHasFastTimer = false;
	m_nRefCount = 1;
}

//--------------------------------------------------------------
CMyRUDP_Socket::~CMyRUDP_Socket()
{
	Close();

	pthread_mutex_destroy( &m_SyncObj_Socket );

	pthread_cond_destroy( &m_Event_FastTimer );
	pthread_mutex_destroy( &m_SyncObj_FastTimer );
#ifdef __USE_XOPEN2K
	pthread_condattr_destroy( &m_Event_FastTimer_Attr );
#endif // __USE_XOPEN2K

#ifdef _DEBUG
	fprintf( stderr, "CMyRUDP_Socket::~CMyRUDP_Socket()\n" );
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-03-02
 *
 *	Add refernce count
 *
 * @return	updated reference count
 */
int CMyRUDP_Socket::AddRef()
{
	return InterlockedIncrement( &m_nRefCount );
}

//--------------------------------------------------------------
/** CYJ 2015-03-02
 *
 *	Decrease refernce count
 *
 * @return	updated reference count
 *
 * @note
 *		when reference count = 0, delete itself
 */
int CMyRUDP_Socket::Release()
{
	int nRetVal = InterlockedDecrement( &m_nRefCount );
#ifdef _DEBUG
	assert( nRetVal >= 0 );
#endif // _DEBUG
	if( nRetVal )
		return nRetVal;

	Close();
	delete this;

	return 0;
}


//--------------------------------------------------------------
/** CYJ 2015-01-17
 *
 *	Open
 *
 * @param [in]	nThreadCount		Thread count
 * @param [in]	pEventObj			event object
 * @param [in]	nPort				port to bind
 * @param [in]	pszLocalBindIP		local bind IP
 *
 * @return		0					succ
 *				other				error code
 */
int CMyRUDP_Socket::Open( int nThreadCount, CMyRUDP_ServerEvent * pEventObj, int nPort, const char * pszLocalBindIP )
{
	if( nThreadCount <= 0 )
		nThreadCount = 16;
	else if( nThreadCount > 1024 )
		nThreadCount = 1024;

	CMyThreadPool * pThreadPoolObj = new CMyThreadPool;
	if( NULL == pThreadPoolObj )
	{
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyRUDP_Socket::Open, Failed to Allocate thread pool\n." );
		return ENOMEM;
	}

	// FIXME, how many thread should be used
	if( pThreadPoolObj->Initialize( nThreadCount ) )
	{
		delete pThreadPoolObj;
		pThreadPoolObj= NULL;
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "%s, Initialize( %d ) thread pool\n.", __FUNCTION__, nThreadCount );
		return ENOMEM;
	}

	int nRetVal = Open( pThreadPoolObj, pEventObj, nPort, pszLocalBindIP );
	pThreadPoolObj->Release();	// the Open will call pThreadObj->AddRef();

	return nRetVal;
}

//--------------------------------------------------------------
/** CYJ 2015-01-17
 *
 *	Open
 *
 * @param [in]	nThreadCount		Thread count
 * @param [in]	pEventObj			event object
 * @param [in]	nPort				port to bind
 * @param [in]	pszLocalBindIP		local bind IP
 *
 * @return		0					succ
 *				other				error code
 */
int CMyRUDP_Socket::Open( CMyThreadPool * pThreadPoolObj, CMyRUDP_ServerEvent * pEventObj, int nPort, const char * pszLocalBindIP )
{
#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s ( %p, %p, %s : %d )called.\n", __FUNCTION__, pThreadPoolObj, pEventObj, pszLocalBindIP, nPort );
	assert( pEventObj );
#endif // _DEBUG

	Close();

	if( NULL == pEventObj || NULL == pThreadPoolObj )
	{
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyRUDP_Socket::Open, Invalid argument, eventobj=%p, pThreadPoolObj=%p.", pEventObj, pThreadPoolObj );
		return EINVAL;
	}

	m_pThreadPoolObj = pThreadPoolObj;
	m_pThreadPoolObj->AddRef();

	m_pEventObj = pEventObj;

    // init buffer
    // free data buffer
    if( false == m_InDataBuf.Initialize( READ_DATA_BUFFER_SIZE ) )
	{
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyRUDP_Socket::Open, Failed to allocate memory for input cache buffer." );
		Close();
		return ENOMEM;
	}

	memset( m_anFastTimerSessionID, 0, sizeof(m_anFastTimerSessionID) );
	m_bHasFastTimer = false;
	m_listUsedSession.clear();	// used session ID
	m_listFreeSession.clear();	// free session ID
	m_listDelayRemoveSession.clear();
	// assume all session are free
	for(int i=0; i<MY_RUDP_MAX_SESSION_NUMBER; i++ )
	{
		m_listFreeSession.push_back( i );
	}

	// create socket
	// FIXME, only support IPv4 now.
	m_hSocket =  socket( AF_INET, SOCK_DGRAM, 0 );
	if( m_hSocket < 0 )
	{
		int nErrNo = errno;
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyRUDP_Socket::Open, failed to create socket, errno=%d.", nErrNo );
		Close();
		return nErrNo;
	}
	// bind the socket
	memset( &m_LocalBindIP, 0, sizeof(m_LocalBindIP) );
	m_LocalBindIP.sin_family = AF_INET;
	if( pszLocalBindIP && *pszLocalBindIP )
		m_LocalBindIP.sin_addr.s_addr = inet_addr( pszLocalBindIP );
	else
		m_LocalBindIP.sin_addr.s_addr = htonl( INADDR_ANY );
	m_LocalBindIP.sin_port = htons( nPort );

	int nRetVal = bind( m_hSocket, (struct sockaddr*)&m_LocalBindIP, sizeof(m_LocalBindIP) );
	if( nRetVal )
	{
		int nErrNo = errno;
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyRUDP_Socket::Open, failed to bind socket, errno=%d.", nErrNo );
		Close();
		return nErrNo;
	}

	// set receive buffer size, 2MB, should change system buffer size by modify /proc/....
	int nRecvBuf = 8L*1024*1024;
	setsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUFFORCE, (const char*)&nRecvBuf,sizeof(int) );
	// set send buffer size, 2MB, should change system buffer size by modify /proc/....
	int nSendBuf = 8L*1024*1024;//设置为32K
	setsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUFFORCE, (const char*)&nSendBuf,sizeof(int) );

#ifdef _DEBUG
	nRecvBuf = 0;
	nSendBuf = 0;
	socklen_t nRetValLen = sizeof(int);
	getsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuf,&nRetValLen );
	nRetValLen = sizeof(int);
	getsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&nSendBuf,&nRetValLen );
	MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyRUDP_Socket::Open, RecvBufSize=%d, SendBufSize=%d\n.", nRecvBuf, nSendBuf );
#endif // _DEBUG

	// create receive data thread
	m_nThreadState = THREAD_STATE_RUNNING;
	m_hThread_ProcessData = 0;
	nRetVal = pthread_create( &m_hThread_ProcessData, NULL, RunLink_ProcessData, (void*)this );
	if( nRetVal )
	{
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyRUDP_Socket::Open, failed to create process data thread." );
		m_hThread_ProcessData = 0;
		Close();
		return ENOSR;
	}

	// create read data thread
	m_hThread_ReadData = 0;
	nRetVal = pthread_create( &m_hThread_ReadData, NULL, RunLink_ReadData, (void*)this );
	if( nRetVal )
	{
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyRUDP_Socket::Open, failed to create read data thread." );
		m_hThread_ReadData = 0;
		Close();
		return ENOSR;
	}

	m_hThread_Timer = 0;
	nRetVal = pthread_create( &m_hThread_Timer, NULL, RunLink_Timer, (void*)this );
	if( nRetVal )
	{
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyRUDP_Socket::Open, failed to create Timer thread." );
		m_hThread_Timer = 0;
		Close();
		return ENOSR;
	}

#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s succ, Leaved.", __FUNCTION__ );
#endif // _DEBUG

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-01-12
 *
 *	Close the socket and exit the working thead
 *
 * send 2 MYRUDP_CLOSE data to all connections but not wait for ACK
 */
void CMyRUDP_Socket::Close()
{
#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s succ, Enter.", __FUNCTION__ );
#endif // _DEBUG

	if( m_hSocket >= 0 )
	{
		// FIXME, to send MYRUDP_CLOSE to all connected peers
	}

	// wait for working thread exist
	m_nThreadState = THREAD_STATE_REQ_EXIT;
	if( m_pThreadPoolObj )
	{
		m_pThreadPoolObj->Release();
		m_pThreadPoolObj = NULL;
	}

	if( m_hThread_Timer )
	{
		void *pRetVal;
		pthread_cond_broadcast( &m_Event_FastTimer );
		pthread_join( m_hThread_Timer, &pRetVal );
		m_hThread_Timer = 0;
	}

	// wait for process data thread exit
	if( m_hThread_ProcessData )
	{
		void *pRetVal;
		pthread_join( m_hThread_ProcessData, &pRetVal );
		m_hThread_ProcessData = 0;
	}

	// wait for read data thread exit.
	if( m_hThread_ReadData )
	{
		void *pRetVal;
		pthread_join( m_hThread_ReadData, &pRetVal );
		m_hThread_ReadData = 0;
	}
	m_nThreadState = THREAD_STATE_IDLE;

	// close socket
	if( m_hSocket >= 0 )
	{	// 2015.4.14 CYJ Add
		CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_Socket );
		close( m_hSocket );
		m_hSocket = -1;
	}

	// free data buffer
	m_InDataBuf.Clean();

#ifdef _DEBUG
	int nTotalCount = m_listUsedSession.size() + m_listFreeSession.size();
	assert( 0 == nTotalCount || MY_RUDP_MAX_SESSION_NUMBER == nTotalCount );
#endif // _DEBUG

	// clear data
	m_listUsedSession.clear();	// used session ID
	m_listFreeSession.clear();	// free session ID
	m_listDelayRemoveSession.clear();
	memset( m_anFastTimerSessionID, 0, sizeof(m_anFastTimerSessionID) );
	m_bHasFastTimer = false;

	for( int i=0; i<MY_RUDP_MAX_SESSION_NUMBER; i++ )
	{
		CMyRUDP_OnePeerObject * pObj = m_apPeerObjects[ i ];
		if( pObj )
			pObj->Release();
	}
	memset( m_apPeerObjects, 0, sizeof(m_apPeerObjects) );

	m_pEventObj = NULL;

#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s succ, Leaved.", __FUNCTION__ );
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-01-12
 *
 *	Send data to the peer, just call sento with sync
 *
 * @param [in] 	pBuf				data to sent
 * @param [in]	nLen				data length
 * @param [in]	pDstAddr			peer socket addr
 * @param [in]	nAddrLen			peer socket addr length
 *
 * @return		0					succ
 *				other				error code
 */
int CMyRUDP_Socket::SendTo( const unsigned char *pBuf, int nLen, const struct sockaddr * pDstAddr, socklen_t nAddrLen )
{
#ifdef _DEBUG
	assert( pBuf && nLen && pDstAddr && nAddrLen );

//	DbgDumpData("Send Data", pBuf, nLen );
#endif // _DEBUG

	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_Socket );

	if( m_hSocket < 0 )
		return -1;
	int nRetVal = sendto( m_hSocket, pBuf, nLen, 0, pDstAddr, nAddrLen );

#ifdef _DEBUG
	if( nRetVal != nLen )
		MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s succ, Send data failed (%d).", __FUNCTION__, nRetVal );
#endif // _DEBUG

	return ( nRetVal == nLen ) ? 0 : -1;
}

//--------------------------------------------------------------
/** CYJ 2015-01-17
 *
 *	Increase read data thread priority
 *
 * @note
 *		Should run as root, otherwise will adjust the thread priority failed
 */
void CMyRUDP_Socket::IncreateReadThreadPriority()
{
	// We'll operate on the currently running thread.
    pthread_t this_thread = pthread_self();

 	struct sched_param params;
	// We'll set the priority to the maximum.
	params.sched_priority = sched_get_priority_max(SCHED_FIFO);

#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "Trying to set thread realtime prio = %d", params.sched_priority );
#endif //_DEBUG

	// Attempt to set thread real-time priority to the SCHED_FIFO policy
	int nRetVal = pthread_setschedparam(this_thread, SCHED_FIFO, &params);
	if( nRetVal )
		MyLog_Printf( MY_LOG_LEVEL_WARNING, "Unsuccessful in setting thread realtime prio, error code: %d", nRetVal );

#ifdef _DEBUG
	// Now verify the change in thread priority
    int policy = 0;
    if( pthread_getschedparam(this_thread, &policy, &params) )
		MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "Couldn't retrieve real-time scheduling paramers" );

    // Check the correct policy was applied
    if(policy != SCHED_FIFO)
        MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "Scheduling (%d) is NOT SCHED_FIFO!", policy );

    // Print thread scheduling priority
    MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG,  "Thread priority is : %d", params.sched_priority );
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-01-17
 *
 *	Wait for input data ready
 *
 * @return	>0			has data ready
 *			0			timeout
 *			<0			error
 */
int CMyRUDP_Socket::WaitForInDataReady()
{
	fd_set fdset;
    struct timeval timeout_val;
    timeout_val.tv_sec = 1;			// timeout 1 second
    timeout_val.tv_usec = 0;

    // wait for any data ready
	FD_ZERO(&fdset);
	FD_SET( m_hSocket, &fdset );

	int nRetVal = select( m_hSocket + 1, &fdset, NULL, NULL, &timeout_val );
	if( nRetVal < 0 )
	{
		sched_yield();
		usleep( 1000 );		// error occur, FIXME! how to fix the error
	}

	return nRetVal;
}

//--------------------------------------------------------------
/** CYJ 2015-01-17
 *
 *	Allocate input data buffer
 *
 * @return	64KB data buffer
 *
 */
unsigned char * CMyRUDP_Socket::AllocateInputDataBuf()
{
	PBYTE pRetVal = NULL;

    while( THREAD_STATE_RUNNING == m_nThreadState )
	{	// check the thread is running or not
		pRetVal = m_InDataBuf.Allocate( READ_DATA_PACKET_MAX_SIZE, 50 );
		if( pRetVal )
			return pRetVal;		// allocate succ

		sched_yield();
	}

	return NULL;
}

//--------------------------------------------------------------
/** CYJ 2015-01-16
 *
 *	Read data thread function
 */
void CMyRUDP_Socket::Run_ReadData()
{
#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s succ, Enter.", __FUNCTION__ );
#endif // _DEBUG

	// increase thread priority
	IncreateReadThreadPriority();

	while( THREAD_STATE_RUNNING == m_nThreadState )
	{
		if( WaitForInDataReady() <= 0 )
		{
			sched_yield();
			continue;
		}

        // read the data
        while( THREAD_STATE_RUNNING == m_nThreadState )
		{
			PBYTE pBuf = AllocateInputDataBuf();
			if( NULL == pBuf )
				break;

			struct sockaddr_in peer_addr;
			socklen_t peer_addr_len = sizeof(peer_addr);
			int nRetVal = recvfrom( m_hSocket, pBuf, READ_DATA_PACKET_MAX_SIZE, MSG_DONTWAIT, (sockaddr*)&peer_addr, &peer_addr_len );

			// EAGAIN or EWOULDBLOCK ==> no data
			// FIXME, need to deal with other error code.
			if( nRetVal <= 0 )
				break;

#ifdef _DEBUG
//	DbgDumpData("Receive Data", pBuf, nRetVal );
#endif //_DEBUG

			// my RUDP packet size must >= 16 bytes
			// first 2 bytes is magic data
			// if not my rudp data packet, discard it and continue reading
			if( nRetVal < MY_RUDP_HEADER_LEN || MYRUDP_MAGIC_ID_HI != pBuf[0] || MYRUDP_MAGIC_ID_LOW != pBuf[1] )
			{
			#ifdef _DEBUG
				MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s Error Data: Len=%d, MagicData: %02X%02X.", __FUNCTION__, nRetVal, pBuf[0], pBuf[1] );
			#endif // _DEBUG
				continue;
			}

#if 0 && defined _DEBUG
			if( pBuf[2] == 5 )
			{
				char szTmp[256];
				char *pszT = szTmp;
				sprintf( pszT, "[%2d] ACK TailSeq: 0x%04x, Expect: 0x%04x; ", (int)(m_tNow%60), pBuf[11] * 0x100 + pBuf[12], pBuf[13] * 0x100 + pBuf[14] );
				pszT += strlen( pszT );
				int nCount = ( nRetVal - 18 ) / 2;
				uint8_t * pbyT = pBuf + 18;
				for(int i=0; i<nCount; i++ )
				{
					sprintf( pszT, "%04x ", pbyT[0] * 0x100 + pbyT[1] );
					pbyT += 2;
					pszT += 5;
				}
				MyRUDP_fprintf( "%s\n", szTmp );
			}
			else
				MyRUDP_fprintf( "======== %s[%2d] ( %5d bytes, %02x -- 0x%02x%02x, %02x/%02x )\n", __FUNCTION__, m_tNow%60, nRetVal, pBuf[2], pBuf[11], pBuf[12], pBuf[14], pBuf[13] );
#endif //_DEBUG

			// Check Command
			int nCmdValue = pBuf[2] & 0x1F;
			int nCmdVersion = pBuf[2] >> 5;
			if( nCmdValue >= MYRUDP_CMD_MAX_VALUE || nCmdVersion > MYRUDP_CMD_MAX_VERSION )
			{
			#ifdef _DEBUG
				MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s succ, Not support command ( %d ) or version (%d).", __FUNCTION__, nCmdValue, nCmdVersion );
			#endif // _DEBUG
				continue;
			}

			// check session ID
			if( false == IsValidSessionID( pBuf ) )
			{
			#ifdef _DEBUG
				int nSessionID = pBuf[4]*0x10000L + pBuf[5]*0x100 + pBuf[6];
				MyRUDP_fprintf( "Sesion (%d)  => %p, not valid, discard the data (%d bytes).\n", nSessionID-1, m_apPeerObjects[ nSessionID-1 ], nRetVal );
			#endif // _DEBUG
				continue;
			}

			// check OK, submit it
			m_InDataBuf.Submit( nRetVal, peer_addr );
		}

		sched_yield();
	}

#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s succ, Leaved.", __FUNCTION__ );
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-01-21
 *
 *	Check the session is valid or not
 *
 * @param [in]	pBuf			input data buffer
 *
 * @return		true			succ, valid session ID
 *				other			invalid session ID
 */
bool CMyRUDP_Socket::IsValidSessionID( unsigned char * pBuf )
{
	// session 0 is reserved for connecting requst, in other command, session 0 is invalid
	int nSessionID = pBuf[4]*0x10000L + pBuf[5]*0x100 + pBuf[6];
	if( nSessionID > MY_RUDP_MAX_SESSION_NUMBER )
	{
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s Error, SessionID: %x.", __FUNCTION__, nSessionID );
	#endif // _DEBUG
		return false;
	}

	if( MYRUDP_CMD_REQ == (pBuf[2] & 0x1F) )
		return ( nSessionID == 0 );		// only req command should be 0

	return( nSessionID && m_apPeerObjects[ nSessionID-1 ] );
}

//--------------------------------------------------------------
/** CYJ 2015-01-17
 *
 *	Check the session is valid or not
 *
 * @param [in]	pBuf				data buffer
 * @param [in]	dwSessionID			session ID
 * @param [in]	dwSessionTagID		session Tag ID
 *
 * @return		0					valid
 *				1					session not exist
 *				-1					session error
 */
int CMyRUDP_Socket::IsSessionValid( const unsigned char *pBuf, int nSessionID, uint32_t dwSessionTagID )
{
	if( nSessionID > MY_RUDP_MAX_SESSION_NUMBER || nSessionID < 0 )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "CMyRUDP_Socket::%s Error SessionID: %d.\n", __FUNCTION__, nSessionID );
	#endif // _DEBUG
		return -1;
	}

	if( ( MYRUDP_CMD_REQ == (pBuf[2] & 0x1F) ) && ( 0 == nSessionID ) )
		return 1;

	nSessionID --;
	CMyRUDP_OnePeerObject * pObj = m_apPeerObjects[ nSessionID ];
	if( NULL == pObj )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "CMyRUDP_Socket::%s SessionID:%d, pObj=NULL.\n", __FUNCTION__, nSessionID );
	#endif // _DEBUG
		return -1;					// session not exist
	}

	if( pObj->CheckSessionValid( pBuf, dwSessionTagID ) )
		return 0;					// session valid, connecting or connected and sessionTagID is same

#ifdef _DEBUG
//	MyRUDP_fprintf( "CMyRUDP_Socket::%s Should not be here, sessionID:%d, dwSessionTagID:0x%08x.\n", __FUNCTION__, nSessionID, dwSessionTagID );
#endif // _DEBUG
	return -1;						// not valid, should be ignore
}

//--------------------------------------------------------------
/** CYJ 2015-01-16
 *
 *	Process data thread function
 */
void CMyRUDP_Socket::Run_ProcessData()
{
#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s succ, Enter.", __FUNCTION__ );
#endif // _DEBUG

// if not defined __USE_XOPEN2K, then not using pthread_cond_timedwait, so signal the condition per second
#ifndef __USE_XOPEN2K
	time_t tLast = time( NULL );
#endif // __USE_XOPEN2K

	while( THREAD_STATE_RUNNING == m_nThreadState )
	{
	#ifndef __USE_XOPEN2K
		time_t tNow = time(NULL);
		if( tNow != tLast )
		{
			tLast = tNow;
			CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_FastTimer );
			pthread_cond_signal( &m_Event_FastTimer );
		}
	#endif // __USE_XOPEN2K

		PBYTE pBuf = NULL;
		DWORD dwBufLen;
		struct sockaddr_in * pPeerAddr = m_InDataBuf.Peek( pBuf, dwBufLen, 50 );	// timeout 50 ms
		if( NULL == pPeerAddr )
		{
			sched_yield();
			continue;
		}

		int nSessionID = (int)( pBuf[4] * 0x10000L + pBuf[5] * 0x100 + pBuf[6] );
		unsigned int dwSessionTagID = (unsigned int)( pBuf[7] * 0x1000000L + pBuf[8] * 0x10000L + pBuf[9] * 0x100 + pBuf[10] );

#if 0 && defined _DEBUG
	//MyRUDP_fprintf( "\n  ==== Receive %ld bytes, SessionID:%d\n", dwBufLen, nSessionID );
	//DbgDumpData("====== On Data Received ====", pBuf, dwBufLen );
if( pBuf[2] == 5 )
{
	char szTmp[256];
	char *pszT = szTmp;
	sprintf( pszT, "--[%2d] ProcessData ACK TailSeq: 0x%04x, Expect: 0x%04x; ", (int)(m_tNow%60), pBuf[11] * 0x100 + pBuf[12], pBuf[13] * 0x100 + pBuf[14] );
	pszT += strlen( pszT );
	int nCount = ( dwBufLen - 18 ) / 2;
	uint8_t * pbyT = pBuf + 18;
	for(int i=0; i<nCount; i++ )
	{
		sprintf( pszT, "%04x ", pbyT[0] * 0x100 + pbyT[1] );
		pbyT += 2;
		pszT += 5;
	}
	MyRUDP_fprintf( "%s\n", szTmp );
}
#endif //_DEBUB

	#ifdef _DEBUG
		assert( pBuf && MYRUDP_MAGIC_ID_HI == pBuf[0] && MYRUDP_MAGIC_ID_LOW == pBuf[1] );
		assert( dwBufLen >= MY_RUDP_HEADER_LEN && dwBufLen <= READ_DATA_PACKET_MAX_SIZE );
		assert( nSessionID >= 0 && nSessionID <= MY_RUDP_MAX_SESSION_NUMBER );
	#endif // _DEBUG

		// check header CRC-16
		if( 0 == CCRC::GetCRC16( MY_RUDP_HEADER_LEN, pBuf ) )
		{
			int nRetval = IsSessionValid( pBuf, nSessionID, dwSessionTagID );
			if( 0 == nRetval )
				OnDataReceived( pBuf, (int)dwBufLen, pPeerAddr );
			else if( 1 == nRetval )
				OnPeerReqConnecting( pBuf, (int)dwBufLen, pPeerAddr );
		}
		else
		{
		#ifdef _DEBUG
			MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s Header CRC16 error.", __FUNCTION__ );
		#endif // _DEBUG
		}

		// free the data buffer
		m_InDataBuf.Free();
	}

#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s succ, Leaved.", __FUNCTION__ );
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-01-21
 *
 *	Run Timer
 */
void CMyRUDP_Socket::RunTimer()
{
	time_t tNow = time( NULL );
	if( tNow == m_tNow )
		return;
#ifdef _DEBUG
	int nDeltaSecond = tNow - m_tNow;
	if( nDeltaSecond > 1 )
		MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s One Second too long( %d seconds ).", __FUNCTION__, nDeltaSecond );
#endif // _DEBUG

	m_tNow = tNow;

	// 2015.4.2 CYJ Add read/write sync object to protected m_apPeerObjects
	CSingleLock_ReadWrite PeerObjSync( &m_PeerObject_SyncObj, false, true );

	// !!!!!!!!!!!!!!!!!!!!!
	// FIXME, may be error here, since the object may be remove in the function OnTimer
	for( std::list<int>::iterator it=m_listUsedSession.begin(); it!=m_listUsedSession.end(); ++it )
	{
		CMyRUDP_OnePeerObject * pObj = m_apPeerObjects[ *it ];
	#ifdef _DEBUG
		assert( pObj );
	#endif // _DEBUG
		if( NULL == pObj )
			continue;

		pObj->AddRef();
		pObj->OnTimer( m_tNow );
		pObj->Release();
	}

	PeerObjSync.Unlock();

	DoDelayRemoveSession();
}

//--------------------------------------------------------------
/** CYJ 2015-02-26
 *
 *	Run fast timer
 */
void CMyRUDP_Socket::RunFastTimer()
{
	m_bHasFastTimer = false;

	// 2015.4.2 CYJ Add read/write sync object to protected m_apPeerObjects
	CSingleLock_ReadWrite PeerObjSync( &m_PeerObject_SyncObj, false, true );

	for(int i=0; i<MY_RUDP_MAX_SESSION_NUMBER; i++ )
	{
		if( false == m_anFastTimerSessionID[i] )
			continue;
		m_anFastTimerSessionID[i] = false;

		CMyRUDP_OnePeerObject * pObj = m_apPeerObjects[ i ];
		if( NULL == pObj )		// maybe disconnected
			continue;

		pObj->AddRef();
		pObj->OnFastTimer();
		pObj->Release();
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-26
 *
 *	Register fast timer
 *
 * @param [in]	nSessionID
 */
void CMyRUDP_Socket::AddFastTimer( int nSessionID )
{
	if( nSessionID >= 0 && nSessionID < MY_RUDP_MAX_SESSION_NUMBER )
	{
		m_anFastTimerSessionID[ nSessionID ] = true;

		CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_FastTimer );
		pthread_cond_signal( &m_Event_FastTimer );

		m_bHasFastTimer = true;
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-26
 *
 *	Remove fast timer
 *
 * @note
 *		if the FastTimer has been scheduled, then can not remove it
 */
void CMyRUDP_Socket::RemoveFastTimer( int nSessionID )
{
	if( nSessionID >= 0 && nSessionID < MY_RUDP_MAX_SESSION_NUMBER )
		m_anFastTimerSessionID[ nSessionID ] = false;
}

//--------------------------------------------------------------
/** CYJ 2015-02-26
 *
 *	Run Timer thread
 */
void CMyRUDP_Socket::Run_Timer()
{
	m_tNow = time( NULL );

	while( THREAD_STATE_RUNNING == m_nThreadState )
	{
		if( m_bHasFastTimer )
			RunFastTimer();
		RunTimer();
		sched_yield();

		CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_FastTimer );

	#ifdef __USE_XOPEN2K
		struct timespec tv;
		clock_gettime( CLOCK_MONOTONIC, &tv );
		int nTimeOut = ( tv.tv_nsec / 1000000L + 500 );		// wait 500 ms
		tv.tv_sec += ( nTimeOut / 1000L );
		tv.tv_nsec = ( nTimeOut%1000 ) * 1000000L;
		pthread_cond_timedwait( &m_Event_FastTimer, &m_SyncObj_FastTimer, &tv );
	#else
		pthread_cond_wait( &m_Event_FastTimer, &m_SyncObj_FastTimer );
	#endif
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-26
 *
 *	Run timer thread linker
 */
void *CMyRUDP_Socket::RunLink_Timer( void * pThis )
{
	((CMyRUDP_Socket*)pThis)->Run_Timer();
#ifdef _DEBUG
	MyRUDP_fprintf( "===== %s Exit\n", __FUNCTION__ );
#endif // _DEBUG
	return NULL;
}

//--------------------------------------------------------------
void * CMyRUDP_Socket::RunLink_ReadData( void * pThis )
{
	((CMyRUDP_Socket*)pThis)->Run_ReadData();
#ifdef _DEBUG
	MyRUDP_fprintf( "===== %s Exit\n", __FUNCTION__ );
#endif // _DEBUG
	return NULL;
}

//--------------------------------------------------------------
void * CMyRUDP_Socket::RunLink_ProcessData( void * pThis )
{
	((CMyRUDP_Socket*)pThis)->Run_ProcessData();
#ifdef _DEBUG
	MyRUDP_fprintf( "===== %s Exit\n", __FUNCTION__ );
#endif // _DEBUG
	return NULL;
}

//--------------------------------------------------------------
/** CYJ 2015-01-16
 *
 *	On data received
 *
 * @param [in]	pBuf				data buffer
 * @param [in]	nLen				data size
 * @param [in]	pSrcAddr			peer socket address
 *
 */
void CMyRUDP_Socket::OnDataReceived( const unsigned char *pBuf, int nLen, const struct sockaddr_in * pSrcAddr )
{
	int nSessionID = (int)( pBuf[4] * 0x10000L + pBuf[5] * 0x100 + pBuf[6] ) - 1;

#ifdef _DEBUG
	assert( nSessionID >= 0 && nSessionID < MY_RUDP_MAX_SESSION_NUMBER );
#endif // _DEBUG

	CMyRUDP_OnePeerObject * pObj = m_apPeerObjects[ nSessionID ];
	if( pObj )
	{
		pObj->AddRef();
		pObj->OnDataPacket( pBuf, nLen, *pSrcAddr );
		pObj->Release();
	}
}

//--------------------------------------------------------------
/** CYJ 2015-01-16
 *
 *	On new Peer connect
 *
 * @param [in]	pBuf				data buffer
 * @param [in]	nLen				data size
 * @param [in]	pSrcAddr			peer socket address
 */
void CMyRUDP_Socket::OnPeerReqConnecting( const unsigned char *pBuf, int nLen, const struct sockaddr_in * pSrcAddr )
{
	// check the command
	int nCmdValue = pBuf[2] & 0x1F;
	if( nCmdValue != MYRUDP_CMD_REQ )
	{
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s, Not a Request Connecting Command (%d).", __FUNCTION__, nCmdValue );
	#endif // _DEBUG
		return;
	}

	// 2015.4.2 CYJ Add read/write sync object to protected m_apPeerObjects
	CSingleLock_ReadWrite PeerObjSync( &m_PeerObject_SyncObj, true, true );

	int nSessionID = AllocateOneSession( *pSrcAddr );
	if( nSessionID < 0 )
	{
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s, Failed to allocate one session.", __FUNCTION__ );
	#endif // _DEBUG
		return;
	}

	int nRetVal = m_apPeerObjects[ nSessionID ]->OnConnectDataPacket( pBuf, nLen );
	if( 0 == nRetVal )
		return;

	// connect failed, remove the object
	DeallocateOneSession( nSessionID );
}

//--------------------------------------------------------------
/** CYJ 2015-01-20
 *
 *	Allocate one session
 *
 * @param [in]	PeerAddr			peer address
 *
 * @return		>=0					session ID
 *				<0					failed
 */
int CMyRUDP_Socket::AllocateOneSession( const struct sockaddr_in & PeerAddr )
{
	if( m_listFreeSession.empty() )
	{
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s, No free session.", __FUNCTION__ );
	#endif // _DEBUG
		return -1;					// no available session
	}

	CMyRUDP_OnePeerObject * pObj = new CMyRUDP_OnePeerObject( this );
	if( NULL == pObj )
	{
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s, Failed to allocate one Peer object.", __FUNCTION__ );
	#endif // _DEBUG
		return -2;					// allocate memory failed
	}

	int nSessionID = m_listFreeSession.front();
#ifdef _DEBUG
	assert( nSessionID >= 0 && nSessionID < MY_RUDP_MAX_SESSION_NUMBER );
	assert( NULL == m_apPeerObjects[nSessionID] );
	MyRUDP_fprintf( "allocate one session : [ %d ] : 0x%08x : %d\n", nSessionID, ntohl( PeerAddr.sin_addr.s_addr ), ntohs(PeerAddr.sin_port) );
#endif // _DEBUG

	if( pObj->Initialize( PeerAddr, nSessionID ) )
	{
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "CMyRUDP_Socket::%s, Failed to Initialize Peer object.", __FUNCTION__ );
	#endif // _DEBUG
		delete pObj;
		return -3;
	}

	// add to used session ID list
	m_listFreeSession.pop_front();
	m_listUsedSession.push_back( nSessionID );
	m_apPeerObjects[ nSessionID ] = pObj;

	return nSessionID;		// succ
}

//--------------------------------------------------------------
/** CYJ 2015-01-21
 *
 *	Deallocate one session
 *
 * @param [in]	nSessionID			session ID
 */
void CMyRUDP_Socket::DeallocateOneSession( int nSessionID )
{
#ifdef _DEBUG
	assert( nSessionID >= 0 && nSessionID < MY_RUDP_MAX_SESSION_NUMBER );

	std::list<int>::iterator it = std::find( m_listUsedSession.begin(), m_listUsedSession.end(), nSessionID ); // 查找list中是否有元素“10”
	assert( it != m_listUsedSession.end() );

	assert( m_apPeerObjects[ nSessionID ] );
#endif //_DEBUG

	CMyRUDP_OnePeerObject * pObj = m_apPeerObjects[ nSessionID ];
	if( pObj )
		pObj->Release();
	m_apPeerObjects[ nSessionID ] = NULL;

	m_listUsedSession.remove( nSessionID );
	m_listFreeSession.push_back( nSessionID );
}

//--------------------------------------------------------------
/** CYJ 2015-01-23
 *
 *	Do delay remove session
 */
void CMyRUDP_Socket::DoDelayRemoveSession()
{
	if( m_listDelayRemoveSession.empty() )
		return;

	// 2015.4.2 CYJ Add read/write sync object to protected m_apPeerObjects
	CSingleLock_ReadWrite PeerObjSync( &m_PeerObject_SyncObj, true, true );
	CSingleLock DelayRemoveSession( &m_SyncObj_DelayRemoveSession, true );
// !!!!!!!!!!!!!!!!!!!!!
	// FIXME, may be error here, since the object may be remove in the function OnTimer
	for( std::list<int>::iterator it=m_listDelayRemoveSession.begin(); it!=m_listDelayRemoveSession.end(); ++it )
	{
		int nSessionID = *it;

	#ifdef _DEBUG
		assert( m_apPeerObjects[ nSessionID ] );
	#endif // _DEBUG

		DeallocateOneSession( nSessionID );
	}

	m_listDelayRemoveSession.clear();
}

//--------------------------------------------------------------
/** CYJ 2015-01-23
 *
 *	Disconnect the session, only called by CMyRUDP_OnePeerObject
 *
 * @param [in]	nSessionID			session ID to be disconnected
 *
 * @note
 *		to avoid disconnect in the function OnDataRecieve or OnTimer
 *	so put the session ID to be disconnect into the list m_listDelayRemoveSession
 *  and the OnTimer will disconnect the sessions later.
 */
void CMyRUDP_Socket::DisconnectSession( int nSessionID )
{
#ifdef _DEBUG
	assert( nSessionID >= 0 && nSessionID < MY_RUDP_MAX_SESSION_NUMBER );
	assert( m_apPeerObjects[ nSessionID ] );
#endif //_DEBUG

	if( nSessionID < 0 || nSessionID >= MY_RUDP_MAX_SESSION_NUMBER || NULL == m_apPeerObjects[ nSessionID ] )
		return;

	CSingleLock DelayRemoveSession( &m_SyncObj_DelayRemoveSession, true );
	m_listDelayRemoveSession.push_back( nSessionID );
}

//--------------------------------------------------------------
/** CYJ 2015-02-21
 *
 *	Create Event object, which will associated with the peer object
 *
 * @param [in]	pPeerObject			peer object
 *
 * @return		NULL				failed
 *				other				event responser
 */
CMyRUDP_EventObj * CMyRUDP_Socket::CreateEventObject( CMyRUDP_OnePeerObject * pPeerObject )
{
	if( NULL == m_pEventObj )
		return NULL;
	return m_pEventObj->OnNewConnection( static_cast<CMyRUDP_OnePeerObjectInterface*>(pPeerObject) );
}

