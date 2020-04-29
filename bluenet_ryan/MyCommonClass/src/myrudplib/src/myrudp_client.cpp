/*********************************************************************************
 *
 *	My RUDP Connection for client
 *
 *	Chen Yongjian @ Zhoi
 *	2015.1.29 @ Xi'an
 *
 *
 ********************************************************************************/
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
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <crc.h>
#include <time.h>

#include <my_pthread_mutex_help.h>

#ifdef __MYRUDP_USE_OPENSSL__
	#include <openssl/aes.h>
	#include <openssl/rand.h>
#endif // __MYRUDP_USE_OPENSSL__

#include "myrudp_client.h"
#include "myrudp_cmd_value.h"
#include "myrudp_auto_delete_buf.h"
#include "myrudp_event.h"
#include "myrudp_dbgprint.h"

////////////////////////////////////////////////////////////////////////
static const unsigned char 	s_abyReqConnUUID[16] = { 0x8c,0xd9,0x7c,0x3d,0xaf,0xe7,0x41,0xb4,0x9e,0x91,0xf8,0x4a,0xb8,0x3e,0x57,0xc8 };
static const unsigned char 	s_abyRspConnUUID[16] = { 0xa6,0xc5,0xed,0x4f,0xda,0x08,0x47,0x21,0xb0,0x48,0xf1,0x1f,0x99,0x77,0x21,0x05 };
static const unsigned char 	s_abySynConnUUID[16] = { 0xa7,0xd3,0x02,0x0d,0x38,0xa7,0x41,0xae,0x99,0x4e,0x59,0x09,0x92,0x16,0x83,0x6a };
static const unsigned char 	s_abySyn2ConnUUID[16]= { 0x02,0xa6,0x40,0x91,0x48,0xdc,0x49,0x3b,0x92,0xa5,0x00,0x09,0x72,0xd6,0xc2,0x7c };

////////////////////////////////////////////////////////////////////////
#define PUT_WORD_TO_BUF( pBuf, wData )		{ pBuf[0]=(uint8_t)((wData)>>8); pBuf[1]=(uint8_t)(wData); }
#define PUT_DWORD_TO_BUF_BE( pBuf, dwData )	for(int i=3; i>=0; i--){ pBuf[i]=(uint8_t)dwData; dwData>>=8; }
#define PUT_DWORD_TO_BUF_LE( pBuf, dwData )	for(int i=0; i<4; i++){ pBuf[i]=(uint8_t)dwData; dwData>>=8; }

#ifdef _DEBUG
	#define DEBUG_PRINT_ENTER_FUNCTION()	MyRUDP_fprintf( "-----------Enter L%d, %s -----------\n", __LINE__, __FUNCTION__ )
	#define DEBUG_PRINT_LEAVE_FUNCTION()	MyRUDP_fprintf( "-----------Leave L%d, %s -----------\n", __LINE__, __FUNCTION__ )
#endif // _DEBUG

#ifdef __TEST_USING_THREAD_POOL__
	// http://wenda.tianya.cn/question/158f187bb886dcb1
	// 在电信级以太网测试中要求标准用户的丢帧率优于0.5%，而对于白金用户在承诺带宽速率下应低于0.001%。
// #define __RAND_LOST_DATA_RATE__ 	( 255 * 10 / 100 )		// 10%, receive and lost
	#ifdef __RAND_LOST_DATA_RATE__
		uint32_t s_dwTotalPacketSend = 0;
		uint32_t s_dwDiscardPacketSend = 0;
		uint32_t s_dwTotalPacketReceived = 0;
		uint32_t s_dwDiscardPacketReceived = 0;
	#endif // __RAND_LOST_DATA_RATE__
#endif // #ifdef __TEST_USING_THREAD_POOL__

#ifdef __TEST_USING_THREAD_POOL__
static CMyThreadPool *	s_pClientThreadPool = NULL;
void MyRUDPClientSetThreadPool( CMyThreadPool * pThreadPool )
{
	s_pClientThreadPool = pThreadPool;
}
#endif //__TEST_USING_THREAD_POOL__

////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void DbgDumpData( const char *pszTitle, const uint8_t * pBuf, int nLen )
{
	struct timeval tv;
	gettimeofday( &tv, NULL );

	struct tm tm_localtime_data;
	localtime_r(&tv.tv_sec, &tm_localtime_data );

	MyRUDP_fprintf_lock();

	fprintf( stderr, "\n--------- %02d:%02d:%02d : %06ld,  %s ( %d bytes ) ------\n", \
			tm_localtime_data.tm_hour, tm_localtime_data.tm_min, tm_localtime_data.tm_sec, tv.tv_usec, pszTitle, nLen );

	char szTmpBuf[256];
	char * pszTmp = szTmpBuf;

	int nOfs = 0;
	while( nLen > 0 )
	{
		sprintf( pszTmp, "%02x ", *pBuf++ );
		pszTmp += 3;
		nOfs ++;
		nLen --;
		if( 8 == nOfs || 16 == nOfs || 24 == nOfs )
		{
			strcpy( pszTmp, " -  " );
			pszTmp += 4;
		}
		else if( 32 == nOfs )
		{
			nOfs = 0;
			fprintf( stderr, "%s\n", szTmpBuf );
			pszTmp = szTmpBuf;
		}
	}
	strcpy( pszTmp , "\n" );
	fprintf( stderr, "%s\n", szTmpBuf );

	MyRUDP_fprintf_unlock();
}
#endif // _DEBUG

#ifdef __MYRUDP_USE_OPENSSL__
	#if defined(__RAND_LOST_DATA_RATE__)
		static uint8_t MY_RAND_BYTE()
		{
			uint8_t byRetVal;
			RAND_bytes( &byRetVal, 1 );
			return byRetVal;
		}
	#endif //_DEBUG

	static uint32_t MY_RAND_DWORD()
	{
		uint32_t dwRetVal;
		RAND_bytes( (uint8_t*)&dwRetVal, sizeof(dwRetVal) );
		return dwRetVal;
	}
#else
	#if defined(__RAND_LOST_DATA_RATE__)
		#define MY_RAND_BYTE()	(uint8_t)rand()
	#endif // #ifdef _DEBUG
	#define MY_RAND_DWORD()	(uint32_t)rand()
#endif // __MYRUDP_USE_OPENSSL__

////////////////////////////////////////////////////////////
static uint16_t	s_awMTUSize[ 8 ] = { 256, 512, 1024, 1392, 0, 0, 0, 0 };

////////////////////////////////////////////////////////////
uint16_t MyRUDP_GetMTUSizeByIndex( int nIndex )
{
	return s_awMTUSize[ nIndex & 7 ];
}

//----------------------------------------------------
uint8_t MyRUDP_GetMTUIndexBySize( int nSize )
{
	int nCount = sizeof( s_awMTUSize ) / sizeof( s_awMTUSize[0] );
	for(int i=0; i<nCount; i++ )
	{
        if( nSize < s_awMTUSize[i] )
			return i ? i-1 : i;
	}
	return -1;
}

//--------------------------------------------------------------
static uint32_t GetDwordFromBuf( const uint8_t * pBuf )
{
	return pBuf[0]*0x1000000L + pBuf[1]*0x10000 + pBuf[2]*0x100 + pBuf[3];
}

//--------------------------------------------------------------
/** CYJ 2015-03-02
 *
 *	Create Client driver
 *
 * @note
 *		call Release function to release the object instance
 */
CMyRUDP_ClientDrv * MyRUDP_CreateClientDrv()
{
	CMyRUDP_Client * pRetVal = new CMyRUDP_Client;
	if( NULL == pRetVal )
		return NULL;

	return static_cast< CMyRUDP_ClientDrv * >( pRetVal );
}

///////////////////////////////////////////////////////////////////
CMyRUDP_Client::CMyRUDP_Client()
	:m_InCacheBuf(0)
{
	m_hThread_ReadData = 0;
	m_hThread_ProcessData = 0;
	m_nThreadState = THREAD_STATE_IDLE;

	pthread_mutex_init( &m_SyncObj_SendOut, NULL );

	m_nState = MYRUDP_CLIENT_STATE_DISCONNECTED;
	memset( &m_ServerAddr, 0, sizeof(m_ServerAddr) );
	m_dwSessionID = 0;		// session ID
	m_dwSessionTagID = 0;	// session Tag ID

	memset( m_abyXorData, 0, sizeof(m_abyXorData) );

	m_nCounterToSendKeepAliveCmd = DEFAULT_KEEP_ALIVE_COUNTER;
	m_nSendKeepAliveIntervalInSeconds = DEFAULT_KEEP_ALIVE_COUNTER;
	m_nSecondsToReconnectToServer = DEFAULT_KEEP_ALIVE_COUNTER * 3;
	m_bNetworkIsAvailable = true;

	m_byMTUIndex = MY_RUDP_MTU_SIZE_512;
	m_wMTUSize = 512;

	m_nSecondsFromLastReceiveFromServer = 0;
	m_pEventObj = NULL;
	m_nSecondsToResendReqOrSyncCmd = 0;
	m_bNeedFastTimer = false;

	m_nRefCount = 1;

	m_hSocket = -1;				// 2015.3.18 CYJ add
	m_wSeqNo = 0;
	m_tNow = 0;

	// 2015.4.28 CYJ Add
	m_aPipeToWakeupSelect[0] = -1;
	m_aPipeToWakeupSelect[1] = -1;

#ifdef __TEST_USING_THREAD_POOL__
	m_ReadDataTask.m_pClient = this;
	m_ProcessDataTask.m_pClient = this;
#endif // __TEST_USING_THREAD_POOL__

#ifdef __MYRUDP_USE_OPENSSL__
	memset( m_abyDataEncryptionAESKey, 0, sizeof(m_abyDataEncryptionAESKey) );
#endif //	__MYRUDP_USE_OPENSSL__
}

//--------------------------------------------------------------
CMyRUDP_Client::~CMyRUDP_Client()
{
	Close();

	pthread_mutex_destroy( &m_SyncObj_SendOut );
}

//--------------------------------------------------------------
/** CYJ 2015-03-02
 *
 *	Add refernce count
 *
 * @return	updated reference count
 */
int CMyRUDP_Client::AddRef()
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
int CMyRUDP_Client::Release()
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
/** CYJ 2015-01-29
 *
 *	Open and connect to the server automatically, if disconnect from the server, it will re-connect again
 *
 * @param [in]	pszServerIP			server IP
 * @param [in]	nPort				server port
 * @param [in]	pEventObj			event object
 *
 * @return		0					succ
 *				other				error code
 */
int CMyRUDP_Client::Open( const char * pszServerIP, int nPort, CMyRUDP_EventObj * pEventObj )
{
	int i;

#ifdef _DEBUG
	MyRUDP_fprintf( "-----------Enter L%d, %s( %s : %d ) -----------\n", __LINE__, __FUNCTION__, pszServerIP, nPort );
#endif // _DEBUG

	Close();

	// 2015.5.7 CYJ Add, check server IP and Port
	if( NULL == pszServerIP || 0 == *pszServerIP || 0 == nPort )
		return EINVAL;

#ifdef __MYRUDP_USE_OPENSSL__
	int nSSLRetVal = m_OpenSSLDrv.Initialize( CMyRUDPCryption::MYRUDP_ECDH_TYPE_384 );
	if( nSSLRetVal )
	{
		MyRUDP_fprintf( "CMyRUDP_Socket::Open, m_OpenSSLDrv.Initialize()=%d.\n", nSSLRetVal );
		return nSSLRetVal;
	}
#endif // #ifdef __MYRUDP_USE_OPENSSL__

	m_nState = MYRUDP_CLIENT_STATE_DISCONNECTED;
	memset( &m_ServerAddr, 0, sizeof(m_ServerAddr) );
	m_dwSessionID = 0;		// session ID
	m_dwSessionTagID = 0;	// session Tag ID
	m_tNow = time(NULL);
	m_nSecondsFromLastReceiveFromServer = 0;
	m_pEventObj = pEventObj;
	if( m_pEventObj )
		m_pEventObj->AddRef();
	m_bNeedFastTimer = false;

	// 2015.4.3 CYJ Add
	m_nSendKeepAliveIntervalInSeconds = DEFAULT_KEEP_ALIVE_COUNTER;
	m_nCounterToSendKeepAliveCmd = DEFAULT_KEEP_ALIVE_COUNTER;
	m_nSecondsToReconnectToServer = DEFAULT_KEEP_ALIVE_COUNTER * 3;
	m_bNetworkIsAvailable = true;

	// generate Xor Data
	uint32_t dwRandData = time(NULL);
	srand( dwRandData );
	for( i=0; i<32; i++ )
	{
		dwRandData += MY_RAND_DWORD();
		dwRandData >>= 1;
		m_abyXorData[i] = (uint8_t)dwRandData;
	}

	// init buffer
    // free data buffer
    if( false == m_InCacheBuf.Initialize( READ_DATA_BUFFER_SIZE ) )
	{
		MyRUDP_fprintf( "CMyRUDP_Socket::Open, Failed to allocate memory for input cache buffer." );
		return ENOMEM;
	}

	// 2015.4.28 CYJ Add, using pipe to wake up select, so I can let select(-1) to sleep a long time
#ifndef __TEST_USING_THREAD_POOL__
	if( pipe( m_aPipeToWakeupSelect ) )
	{
		m_aPipeToWakeupSelect[0] = -1;
		m_aPipeToWakeupSelect[1] = -1;
	}
#endif // #ifdef __TEST_USING_THREAD_POOL__

	// create socket
	// FIXME, only support IPv4 now.
	m_hSocket =  socket( AF_INET, SOCK_DGRAM, 0 );
	if( m_hSocket < 0 )
	{
		int nErrNo = errno;
	#ifdef _DEBUG
		MyRUDP_fprintf( "CMyRUDP_Socket::Open, failed to create socket, errno=%d.", nErrNo );
	#endif //_DEBUG
		Close();
		return nErrNo;
	}

	// modify receive / send buffer size
	int nRecvBuf = 256L*1024;
	setsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf,sizeof(int) );
	int nSendBuf = 256L*1024;//设置为32K
	setsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf,sizeof(int) );

	m_strServerIP = pszServerIP;
	memset( &m_ServerAddr, 0, sizeof(m_ServerAddr) );
	// 2015.5.7 CYJ Add, using inet_aton instead of inet_addr
	if( 0 == inet_aton( pszServerIP, &m_ServerAddr.sin_addr ) )
	{	// a domain name
		m_ServerAddr.sin_addr.s_addr = 0;
	}
	m_ServerAddr.sin_port = htons( nPort );
	m_ServerAddr.sin_family = AF_INET;

#ifdef __TEST_USING_THREAD_POOL__
	m_hThread_ProcessData = 0;
	m_hThread_ReadData = 0;

	m_nThreadState = THREAD_STATE_RUNNING;

	assert( s_pClientThreadPool );
	s_pClientThreadPool->AddTask( &m_ReadDataTask );
	s_pClientThreadPool->AddTask( &m_ProcessDataTask );
#else
	// create receive data thread
	m_hThread_ProcessData = 0;
	m_nThreadState = THREAD_STATE_RUNNING;
	int nRetVal = pthread_create( &m_hThread_ProcessData, NULL, RunLink_ProcessData, (void*)this );
	if( nRetVal )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "failed to create process data thread." );
	#endif //_DEBUG
		m_hThread_ProcessData = 0;
		Close();
		return ENOSR;
	}

	// create read data thread
	m_hThread_ReadData = 0;
	nRetVal = pthread_create( &m_hThread_ReadData, NULL, RunLink_ReadData, (void*)this );
	if( nRetVal )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "failed to create read data thread." );
	#endif //_DEBUG
		m_hThread_ReadData = 0;
		Close();
		return ENOSR;
	}
#endif //__TEST_USING_THREAD_POOL__

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-05-31
 *
 *	Reopen server IP and port
 *
 * @param [in]	pszServerIP			server IP, must be IP, not URL
 * @param [in]	nPort				server port
 *
 * @return		0					succ
 *				other				error code
 */
int CMyRUDP_Client::SetServerIPAndPort( const char * pszServerIP, int nPort )
{
	// 2015.5.7 CYJ Add, check server IP and Port
	if( NULL == pszServerIP || 0 == *pszServerIP || 0 == nPort )
		return EINVAL;

	struct sockaddr_in 	ServerAddr;
	memset( &ServerAddr, 0, sizeof(ServerAddr) );

	// 2015.5.7 CYJ Add, using inet_aton instead of inet_addr
	if( 0 == inet_aton( pszServerIP, &ServerAddr.sin_addr ) )
	{	// a domain name
		return EINVAL;
	}

	ServerAddr.sin_port = htons( nPort );
	ServerAddr.sin_family = AF_INET;

	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_SendOut );
	m_strServerIP = pszServerIP;
	memcpy( &m_ServerAddr, &ServerAddr, sizeof(m_ServerAddr) );
	SyncObj.Unlock();

	m_nSecondsFromLastReceiveFromServer = m_nSecondsToReconnectToServer;
	m_InCacheBuf.Wakeup( true, false );

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	Disconnect from the server and free resources
 */
void CMyRUDP_Client::Close()
{
#ifdef _DEBUG
	DEBUG_PRINT_ENTER_FUNCTION();
#endif // _DEBUG

#ifdef __TEST_USING_THREAD_POOL__
	assert( s_pClientThreadPool );
	s_pClientThreadPool->RemoveTask( &m_ReadDataTask );
	s_pClientThreadPool->RemoveTask( &m_ProcessDataTask );
#endif //__TEST_USING_THREAD_POOL__

	m_bNeedFastTimer = false;

	if( m_hSocket >= 0 && MYRUDP_CLIENT_STATE_DISCONNECTED != m_nState )
		SendCloseCmdToServer();

	// 2015.4.28 CYJ Add
	m_InCacheBuf.Wakeup( true, true );
	if( m_aPipeToWakeupSelect[1] >= 0 )
	{	// to wake up select
		uint8_t byData = 0;
		write( m_aPipeToWakeupSelect[1], &byData, 1 );
	}

	// wait for working thread exist
	m_nThreadState = THREAD_STATE_REQ_EXIT;
	CMyRUDP_Packet_Sender::Invalidate();

	// 2015.4.28 CYJ Add
	m_InCacheBuf.Wakeup( true, true );

	// wait for read data thread exit.
	if( m_hThread_ReadData )
	{
		void *pRetVal;
		pthread_join( m_hThread_ReadData, &pRetVal );
		m_hThread_ReadData = 0;
	}

	// 2015.4.28 CYJ Add
	m_InCacheBuf.Wakeup( true, true );

	// wait for process data thread exit
	if( m_hThread_ProcessData )
	{
		void *pRetVal;
		pthread_join( m_hThread_ProcessData, &pRetVal );
		m_hThread_ProcessData = 0;
	}

	m_nThreadState = THREAD_STATE_IDLE;

	m_InPacketMgr.Invalidate();

	// close socket
	if( m_hSocket >= 0 )
	{
		close( m_hSocket );
		m_hSocket = -1;
	}

	m_nState = MYRUDP_CLIENT_STATE_DISCONNECTED;
	memset( &m_ServerAddr, 0, sizeof(m_ServerAddr) );
	m_dwSessionID = 0;		// session ID
	m_dwSessionTagID = 0;	// session Tag ID
	if( m_pEventObj )
		m_pEventObj->Release();
	m_pEventObj = NULL;

	memset( m_abyXorData, 0, sizeof(m_abyXorData) );

	// free data buffer
	m_InCacheBuf.Clean();

#ifdef __MYRUDP_USE_OPENSSL__
	memset( m_abyDataEncryptionAESKey, 0, sizeof(m_abyDataEncryptionAESKey) );
#endif //	__MYRUDP_USE_OPENSSL__

	// 2015.4.28 CYJ Add
	for( int i=0; i<2; i++ )
	{
		if( m_aPipeToWakeupSelect[i] < 0 )
			continue;
		close( m_aPipeToWakeupSelect[i] );
		m_aPipeToWakeupSelect[i] = -1;
	}

#ifdef _DEBUG
	DEBUG_PRINT_LEAVE_FUNCTION();
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	Set raw data to server
 *
 * @param [in]
 * @param [in]
 * @param [in]
 *
 * @return
 *
 */
int CMyRUDP_Client::SendRawDataToPeer( const uint8_t * pBuf, int nLen )
{
#ifdef _DEBUG
//	DEBUG_PRINT_ENTER_FUNCTION();
//	DbgDumpData( "Send: ", pBuf, nLen );
#endif // _DEBUG

#if defined(__RAND_LOST_DATA_RATE__)
	if( s_dwTotalPacketSend > 0x7FFFFFFF )
	{
		s_dwTotalPacketSend >>= 1;
		s_dwDiscardPacketSend >>= 1;
	}
	s_dwTotalPacketSend ++;
	if( MY_RAND_BYTE() < __RAND_LOST_DATA_RATE__ )
	{
		s_dwDiscardPacketSend ++;
//		MyRUDP_fprintf( "++++++++++++++ Abort Sending data\n" );
		MyRUDP_fprintf( "<ABORT ONE PACKET: %6.1f%%>", ((double)s_dwDiscardPacketSend )*100/s_dwTotalPacketSend );
		return 0;		// discard data, testing
	}
#endif // _DEBUG

	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_SendOut );
	// 2015.4.28 CYJ Add check m_hSocket
	// 2015.5.7 CYJ add, check m_ServerAddr.sin_addr.s_addr
	if( m_hSocket < 0 || 0 == m_ServerAddr.sin_addr.s_addr )
		return -1;
	return sendto( m_hSocket, (char*)pBuf, nLen, 0, (sockaddr*)&m_ServerAddr, sizeof(m_ServerAddr) ) == nLen ? 0 : 1;
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	Send REQ Command to server
 *
 * @return	0				succ
 *			other			error code
 */
int CMyRUDP_Client::SendReqCmd()
{
#ifdef _DEBUG
	DEBUG_PRINT_ENTER_FUNCTION();
#endif // _DEBUG

	// must be clear before construct the command data
	m_dwSessionID = 0;		// session ID
	m_dwSessionTagID = 0;	// session Tag ID
	m_nSecondsToResendReqOrSyncCmd = 0;
	m_nSecondsFromLastReceiveFromServer = 0;	// just send ReqCmd to server, reset it

	// 2015.5.6 CYJ Add, clean old data when send req command
	while( m_InCacheBuf.GetCount() )
	{
		m_InCacheBuf.Free();
	}

	CMyRUDP_AutoDeleteBuf CmdDataBuf;

	// [ CommonHeader ][ UUID ][ XorData ][ MTUIndex ][ PubKeyLen ][ --N-- ][ CRC32 ]
	int nCmdDataLen = MY_RUDP_HEADER_LEN + 16 + 32 + 1 + 2 + 4;
#ifdef __MYRUDP_USE_OPENSSL__
	std::vector<uint8_t> aPubKey = GetEncryptedPublicKey();
  #ifdef _DEBUG
	assert( false == aPubKey.empty() );
  #endif //_DEBUG
	if( aPubKey.empty() )
		return EIO;
	nCmdDataLen += aPubKey.size();
#endif //	__MYRUDP_USE_OPENSSL__

	if( false == CmdDataBuf.Allocate( nCmdDataLen + 64 ) )
		return ENOMEM;
	uint8_t * pCmdBuf = CmdDataBuf.GetBuffer();

	// SeqNo must be 0, to avoid encrypt sessionTagID
	PrepareCommonHeader( pCmdBuf, MYRUDP_CMD_REQ );
	pCmdBuf[11] = 0;
	pCmdBuf[12] = 0;
	CommitCommonHeader( pCmdBuf );

	uint8_t * pTmpBuf = pCmdBuf + MY_RUDP_HEADER_LEN;
	memcpy( pTmpBuf, s_abyReqConnUUID, 16 );
	pTmpBuf += 16;
	// should do AES256 encrypted first.
	memcpy( pTmpBuf, m_abyXorData, 32 );
	pTmpBuf += 32;
	*pTmpBuf++ = MY_RUDP_MTU_SIZE_512;		// using 512 now

#ifdef __MYRUDP_USE_OPENSSL__
	uint16_t wPubKeyLen = aPubKey.size();
	PUT_WORD_TO_BUF( pTmpBuf, wPubKeyLen );
	pTmpBuf += 2;		// PubKey Len + PubKey
	memcpy( pTmpBuf, &aPubKey.front(), wPubKeyLen );
	pTmpBuf += wPubKeyLen;
#else
	pTmpBuf += 2;		// PubKey Len + PubKey
#endif

	uint32_t dwCRC32 = CCRC::GetCRC32( nCmdDataLen-4, pCmdBuf );
	PUT_DWORD_TO_BUF_LE( pTmpBuf, dwCRC32 );
#ifdef _DEBUG
	assert( 0 == CCRC::GetCRC32( nCmdDataLen, pCmdBuf ) );
#endif // _DEBUG

	m_nState = MYRUDP_CLIENT_STATE_WAIT_RSP;
	// notify start connnecting
	if( m_pEventObj )
		m_pEventObj->OnConnectionStateChanged( CMyRUDP_EventObj::MY_RUDP_CONNECTION_STATE_CONNECTING );

	return SendRawDataToPeer( pCmdBuf, nCmdDataLen );
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	Send SYN command to server
 *
 * @return	0				succ
 *			other			error code
 */
void CMyRUDP_Client::SendSyncCmd()
{
#ifdef _DEBUG
	DEBUG_PRINT_ENTER_FUNCTION();
#endif // _DEBUG

	m_nSecondsToResendReqOrSyncCmd = 0;

	CMyRUDP_AutoDeleteBuf CmdDataBuf;

	// [ CommonHeader ][ UUID ][ CRC32 ]
	int nCmdDataLen = MY_RUDP_HEADER_LEN + 16 + 4;
	if( false == CmdDataBuf.Allocate( nCmdDataLen + 64 ) )
		return;
	uint8_t * pCmdBuf = CmdDataBuf.GetBuffer();

	// SeqNo must be 0, to avoid encrypt sessionTagID
	PrepareCommonHeader( pCmdBuf, MYRUDP_CMD_SYN );

	// client's state has been set to MYRUDP_CLIENT_STATE_CONNECTED, so the SYN is lost, add re-transmit flag ON
	// 2015.5.4 CYJ Modify, change from "==MYRUDP_CLIENT_STATE_CONNECTED" to ">= MYRUDP_CLIENT_STATE_WAIT_SYNC_2"
	if( m_nState >= MYRUDP_CLIENT_STATE_WAIT_SYNC_2 )
		pCmdBuf[3] |= 0x80;

	pCmdBuf[11] = 0;
	pCmdBuf[12] = 0;
	CommitCommonHeader( pCmdBuf );

	uint8_t * pTmpBuf = pCmdBuf + MY_RUDP_HEADER_LEN;
	memcpy( pTmpBuf, s_abySynConnUUID, 16 );
	pTmpBuf += 16;

	uint32_t dwCRC32 = CCRC::GetCRC32( nCmdDataLen-4, pCmdBuf );
	PUT_DWORD_TO_BUF_LE( pTmpBuf, dwCRC32 );
#ifdef _DEBUG
	assert( 0 == CCRC::GetCRC32( nCmdDataLen, pCmdBuf ) );
#endif // _DEBUG

	SendRawDataToPeer( pCmdBuf, nCmdDataLen );
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	Get RSP command from server
 *
 * @param [in]	pBuf		RSP data
 * @param [in]	nLen		RSP data length
 */
void CMyRUDP_Client::OnRspCmd( unsigned char *pBuf, int nLen)
{
#ifdef _DEBUG
	MyRUDP_fprintf( "--- Get RSP data: %d bytes, flags=%02x\n", nLen, pBuf[3] );
#endif // _DEBUGT

	//	[ RUDP_Header ][  UUID  ][ MTU ][ ECC PubKeyLen ][ PubKey Data] [ CRC32_XorData ] [ CRC32 ]
	//        18	       16       1		  2					N			4        4

	int nMinRspLen = MY_RUDP_HEADER_LEN + 16 + 1 + 2 + 4 + 4;
	if( nLen < nMinRspLen )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d, no engouth data ( %d < %d )\n", __FUNCTION__, __LINE__, nLen, nMinRspLen );
	#endif // _DEBUG
		return;				// not a Rsp data
	}
    if( memcmp( pBuf+MY_RUDP_HEADER_LEN, s_abyRspConnUUID, 16 ) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d, UUID not a Rsp UUID\n", __FUNCTION__, __LINE__ );
	#endif // _DEBUG
		return;				// not a Rsp data
	}

	CMyRUDP_HeaderReader Reader( pBuf );
	if( Reader.GetCommand() != MYRUDP_CMD_RSP )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d, not RSP command\n", __FUNCTION__, __LINE__ );
	#endif // _DEBUG
		return;				// not a Rsp data
	}
	if( Reader.GetPackteCount() || Reader.GetPackteIndex() || Reader.GetSeqNo() )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d, should be 0 %d, %d, %d\n", __FUNCTION__, __LINE__, Reader.GetPackteCount(), Reader.GetPackteIndex(), Reader.GetSeqNo() );
	#endif // _DEBUG
		return;				// all these values should be 0
	}
	if( CCRC::GetCRC32( nLen, pBuf ) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d, no Data crc32 error\n", __FUNCTION__, __LINE__ );
	#endif // _DEBUG
		return;				// CRC32 check failed
	}

	// 2015.5.7 CYJ Add, check Xor Data CRC32
	uint32_t dwXorDataCRC32 = GetDwordFromBuf( pBuf + nLen - 8 );
	if( CCRC::GetCRC32( sizeof(m_abyXorData), m_abyXorData ) != dwXorDataCRC32 )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d, XorData CRC32 not same, discard the RSP data packet\n", __FUNCTION__, __LINE__ );
	#endif // _DEBUG
		return;
	}

	// 2015.5.4 CYJ Modify, change from "MYRUDP_CLIENT_STATE_CONNECTED == m_nState" to "m_nState >= MYRUDP_CLIENT_STATE_WAIT_SYNC_2"
	if( m_nState >= MYRUDP_CLIENT_STATE_WAIT_SYNC_2 )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d, Has connected succ, may be Syn cmd lost\n", __FUNCTION__, __LINE__ );
	#endif // _DEBUG
		SendSyncCmd();
		return;
	}

	m_byMTUIndex = Reader.GetMTUIndex();
	m_wMTUSize = Reader.GetMTUSize();

	m_dwSessionID = Reader.GetSessionID();
	m_dwSessionTagID = Reader.GetSessionTagID();

#ifdef _DEBUG
	MyRUDP_fprintf( "--- %s: MTU Size=%d/%d, sessionID=%d, TagID=0x%08x \n", __FUNCTION__, m_byMTUIndex, m_wMTUSize, m_dwSessionID, m_dwSessionTagID );
#endif // _DEBUG

	// init sender and receiver
	if( m_InPacketMgr.Initialize( m_wMTUSize ) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s Initialize In Packet manager failed, may be no memory.\n", __FUNCTION__ );
	#endif // _DEBUG

		// FIXME, how notify the APP, error occur.
		return;
	}

	// init sender
	if( CMyRUDP_Packet_Sender::Initialize( m_wMTUSize, m_dwSessionID, m_dwSessionTagID, m_abyXorData ) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s Initialize sender failed, may be no memory.\n", __FUNCTION__ );
	#endif // _DEBUG

		// FIXME, how notify the APP, error occur.
		return;
	}

#ifdef __MYRUDP_USE_OPENSSL__
	// get AES key
	//	[ RUDP_Header ][  UUID  ][ MTU ][ ECC PubKeyLen ][ PubKey Data] [ CRC32 ]
	//        18	       16       1		  2					N			4
	uint8_t * pPeerPubKey = pBuf + MY_RUDP_HEADER_LEN + 16 + 1;
	uint16_t wPeerPubKeyLen = pPeerPubKey[0] * 0x100 + pPeerPubKey[1];
	pPeerPubKey += 2;
	if( false == GetDataEncryptionAESKey( pPeerPubKey, wPeerPubKeyLen ) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s Failed to get data decryption AES Key from peer.\n", __FUNCTION__ );
	#endif // _DEBUG
		return;
	}
#endif // #ifdef __MYRUDP_USE_OPENSSL__

	SendSyncCmd();

	// 2015.5.4 CYJ Modify
	m_nState = MYRUDP_CLIENT_STATE_WAIT_SYNC_2;
}

//--------------------------------------------------------------
/** CYJ 2015-05-04
 *
 *	Get ACK of Sync
 *
 * @param [in]
 * @param [in]
 * @param [in]
 *
 * @return
 *
 */
void CMyRUDP_Client::OnSync2Cmd( unsigned char *pBuf, int nLen)
{
#ifdef _DEBUG
	MyRUDP_fprintf( "--- [%d] Get SYNC_2 data: %d bytes, flags=%02x\n", m_dwSessionID, nLen, pBuf[3] );
#endif // _DEBUGT

	if( MYRUDP_CLIENT_STATE_CONNECTED == m_nState )
		return;

	//	[ RUDP_Header ][  UUID  ][ CRC32 ]
	//        18	       16    	4
	int nMinRspLen = MY_RUDP_HEADER_LEN + 16 + 4;
	if( nLen < nMinRspLen )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d, no engouth data ( %d < %d )\n", __FUNCTION__, __LINE__, nLen, nMinRspLen );
	#endif // _DEBUG
		return;				// not a Rsp data
	}
    if( memcmp( pBuf+MY_RUDP_HEADER_LEN, s_abySyn2ConnUUID, 16 ) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d, UUID not a SYNC2 UUID\n", __FUNCTION__, __LINE__ );
	#endif // _DEBUG
		return;				// not a Rsp data
	}

	if( CCRC::GetCRC32( nLen, pBuf ) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d, no Data crc32 error\n", __FUNCTION__, __LINE__ );
	#endif // _DEBUG
		return;				// CRC32 check failed
	}

	m_nState = MYRUDP_CLIENT_STATE_CONNECTED;

	// notify connected succ
	if( m_pEventObj )
		m_pEventObj->OnConnectionStateChanged( CMyRUDP_EventObj::MY_RUDP_CONNECTION_STATE_CONNECTED );
}

#ifdef __MYRUDP_USE_OPENSSL__

//--------------------------------------------------------------
/** CYJ 2015-04-19
 *
 *	Get encrypted public key
 */
std::vector<uint8_t> CMyRUDP_Client::GetEncryptedPublicKey()
{
    const std::vector<uint8_t> & aOriginalPubKey = m_OpenSSLDrv.GetPubKey();

#ifdef _DEBUG
//	DbgDumpData( "MY ORIGINAL PUBLIC KEY:", aOriginalPubKey.data(), aOriginalPubKey.size() );
#endif //_DEBUG

    std::vector<uint8_t> aRetPubKey;
    int nEncryptedDataLen = ( aOriginalPubKey.size() + 15 ) & (~15 );
    int nPaddingLen = nEncryptedDataLen - aOriginalPubKey.size();
    uint32_t nBufNeed = nEncryptedDataLen + 1;
    aRetPubKey.resize( nBufNeed );
    if( aRetPubKey.size() != nBufNeed )
	{
		aRetPubKey.clear();
		return aRetPubKey;		// failed
	}

    unsigned char abyAESKey[16];
    nPaddingLen |= ( abyAESKey[0] & 0xF0 );		// use the random data
	unsigned char abyIV[16];
	memcpy( abyAESKey, m_abyXorData, 16 );
	memcpy( abyIV, m_abyXorData+16, 16 );

	abyAESKey[3] = 0x67;
	abyAESKey[7] ^= 0x83;
	abyAESKey[12] &= 0x51;
	abyIV[8] = 0x22;
	abyIV[5] ^= 0xFF;
	abyIV[11] ^= 0x88;

#ifdef _DEBUG
//	DbgDumpData( "AES 128 KEY:", abyAESKey, sizeof(abyAESKey) );
//	DbgDumpData( "AES 128 IV:", abyIV, sizeof(abyIV) );
#endif // _DEBUG

	AES_KEY aes;
	AES_set_encrypt_key( abyAESKey, 128, &aes);
	AES_cbc_encrypt( &aOriginalPubKey.front(), &aRetPubKey.front(), nEncryptedDataLen, &aes, abyIV, 1 );	// encrypt

	aRetPubKey[ nEncryptedDataLen ] = (uint8_t)nPaddingLen;

#ifdef _DEBUG
//	DbgDumpData( "ENCRPYTED MY PUBLIC KEY:", aRetPubKey.data(), aRetPubKey.size() );
#endif //_DEBUG

	return aRetPubKey;
}

//--------------------------------------------------------------
/** CYJ 2015-04-19
 *
 *	Get AES public Key
 *
 * @param [in]	pbyPeerPubKey			peer public key
 * @param [in]	nLen					public key length
 *
 * @return	true						succ
 *			false						failed
 */
bool CMyRUDP_Client::GetDataEncryptionAESKey( const uint8_t * pbyPeerPubKey, int nLen )
{
#ifdef _DEBUG
	assert( nLen >= 17 && (nLen&0xF) == 1 );
//	DbgDumpData( "ENCRYPTED PEER PUBLIC KEY:", pbyPeerPubKey, nLen );
#endif //_DEBUG

	nLen --;
	uint8_t byPaddingLen = pbyPeerPubKey[ nLen ] & 0xF;

	std::vector<uint8_t>  aPubKey;
	aPubKey.resize( nLen );
	if( aPubKey.size() != (uint32_t)nLen )
		return false;

	unsigned char abyAESKey[32];
	unsigned char abyIV[16];

	memcpy( abyAESKey, m_abyXorData, 32 );
	abyAESKey[25] ^= 0xff;
	abyAESKey[15] ^= 0x78;
	abyAESKey[5] &= 0xA5;
	memset( abyIV, m_abyXorData[0], 16 );
	abyIV[0] = (uint8_t)( m_dwSessionID >> 8 );
	abyIV[1] = (uint8_t)( m_dwSessionID );
	abyIV[7] = (uint8_t)( m_dwSessionID );
	abyIV[15] = m_abyXorData[9];

#ifdef _DEBUG
//	DbgDumpData( "AES - 256 Key:", abyAESKey, sizeof(abyAESKey) );
//	DbgDumpData( "AES - 256 IV:", abyIV, sizeof(abyIV) );
#endif //_DEBUG

	AES_KEY aes;
	AES_set_decrypt_key( abyAESKey, 256, &aes);
	AES_cbc_encrypt( pbyPeerPubKey, &aPubKey.front(), nLen, &aes, abyIV, 0 );	// do decrypt

	nLen -= byPaddingLen;

#ifdef _DEBUG
//	DbgDumpData( "DECRYPTED PEER PUBLIC KEY:", aPubKey.data(), nLen );
#endif //_DEBUG

	std::vector<uint8_t> aAESKey = m_OpenSSLDrv.DecryptAESKey( &aPubKey.front(), nLen );
	if( aAESKey.empty() )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s, Failed to get data AES key.\n", __FUNCTION__ );
	#endif // _DEBUG
		return false;
	}

#ifdef _DEBUG
//	DbgDumpData( "Data AES Key:", aAESKey.data(), aAESKey.size() );
#endif //_DEBUG

	SetDataEncryptionAESKey( &aAESKey.front(), aAESKey.size() );

	return true;
}

//--------------------------------------------------------------
/** CYJ 2015-04-19
 *
 *	Set data encryption AES Key
 *
 * @param [in]	pBuf			AES Key
 * @param [in]	nLen			AES Key length
 */
void CMyRUDP_Client::SetDataEncryptionAESKey( const uint8_t * pBuf, int nLen )
{
#ifdef _DEBUG
//	DbgDumpData( __FUNCTION__, pBuf, nLen );
#endif //_DEBUG

	if( nLen >= (int)sizeof(m_abyDataEncryptionAESKey) )
		memcpy( m_abyDataEncryptionAESKey, pBuf, sizeof(m_abyDataEncryptionAESKey) );
	else
	{
		memcpy( m_abyDataEncryptionAESKey, pBuf, nLen );
		memcpy( m_abyDataEncryptionAESKey+nLen, m_abyXorData, sizeof(m_abyDataEncryptionAESKey)-nLen );
	}

	m_InPacketMgr.SetDataEncryptionAESKey( m_abyDataEncryptionAESKey );
	CMyRUDP_Packet_Sender::SetDataEncryptionAESKey( m_abyDataEncryptionAESKey );
}

#endif // #ifdef __MYRUDP_USE_OPENSSL__

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	Send close command to server
 */
void CMyRUDP_Client::SendCloseCmdToServer()
{
#ifdef _DEBUG
	DEBUG_PRINT_ENTER_FUNCTION();
#endif // _DEBUG

	uint8_t	abyCmdData[ MY_RUDP_HEADER_LEN ];

	// SeqNo must be 0, to avoid encrypt sessionTagID
	PrepareCommonHeader( abyCmdData, MYRUDP_CMD_CLOSE );
	abyCmdData[11] = 0;
	abyCmdData[12] = 0;
	CommitCommonHeader( abyCmdData );

	for(int i=0; i<5; i++ )
	{
		SendRawDataToPeer( abyCmdData, MY_RUDP_HEADER_LEN );
		usleep( 10*1000 );		// wait 10ms, send twice
	}
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	Send keep alive command to server per 10 second
 */
void CMyRUDP_Client::SendKeepAliveToServer()
{
#ifdef _DEBUG
	DEBUG_PRINT_ENTER_FUNCTION();
#endif // _DEBUG

	uint8_t	abyCmdData[ MY_RUDP_HEADER_LEN ];

	// SeqNo must be 0, to avoid encrypt sessionTagID
	PrepareCommonHeader( abyCmdData, MYRUDP_CMD_NOP );
	abyCmdData[11] = 0;
	abyCmdData[12] = 0;
	CommitCommonHeader( abyCmdData );

	SendRawDataToPeer( abyCmdData, MY_RUDP_HEADER_LEN );
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	Prepare common header
 *
 * @param [in]	pBuf			header buffer
 * @param [in]	byCmdValue		command value
 */
void CMyRUDP_Client::PrepareCommonHeader( uint8_t * pBuf, uint8_t byCmdVale )
{
	int i;

	memset( pBuf, 0, MY_RUDP_HEADER_LEN );
	pBuf[ 0 ] = MYRUDP_MAGIC_ID_HI;
	pBuf[ 1 ] = MYRUDP_MAGIC_ID_LOW;
	// current version is 0
	pBuf[ 2 ] = ( byCmdVale & 0x1F );
	pBuf[ 3 ] = 0;

	uint32_t dwTmp = m_dwSessionID;
	for( i=2; i>=0; i-- )
	{
		pBuf[4+i] = (uint8_t)dwTmp;
		dwTmp >>= 8;
	}
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	update CRC16
 *
 * @param [in]	pBuf			common header buffer
 *
 */
void CMyRUDP_Client::UpdateCRC( uint8_t * pBuf )
{
	uint16_t wCRC16 = CCRC::GetCRC16( MY_RUDP_HEADER_LEN-2, pBuf );
	pBuf[16] = (uint8_t)( wCRC16 );
	pBuf[17] = (uint8_t)( wCRC16 >> 8 );

#ifdef _DEBUG
	assert( 0 == CCRC::GetCRC16( MY_RUDP_HEADER_LEN, pBuf ) );
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	Commit Common header
 *
 * @param [in]	pBuf			header buffer
 */
void CMyRUDP_Client::CommitCommonHeader( uint8_t * pBuf )
{
	int i;

	// encrypt sessionTag ID
    uint32_t dwSessionTagID = m_dwSessionTagID;

    CMyRUDP_HeaderReader Reader( pBuf );
	uint32_t dwSeqNo = Reader.GetSeqNo();
    if( dwSeqNo )
	{
		// SessionTagID =  ( OrgSessionTagID ^ s_adwXorData[SeqNo] ) + (SeqNo << 10) + (PacketIndex << 4) + DataPaddLen
		uint32_t dwXorData = 0;
		uint8_t *pXorData = m_abyXorData;
		for( int i=0; i<4; i++ )
		{
			dwXorData <<= 8;
			dwXorData |= pXorData[ (dwSeqNo + i) & 0x1F ];
		}
		dwSessionTagID ^= dwXorData;
		dwSessionTagID += ( dwSeqNo << 10 ) + ( Reader.GetPackteIndex() << 4 ) + Reader.GetPaddingDataLen();
	}

	for( i=3; i>=0; i-- )
	{
		pBuf[7+i] = (uint8_t)dwSessionTagID;
		dwSessionTagID >>= 8;
	}

	UpdateCRC( pBuf );
}

//--------------------------------------------------------------
/** CYJ 2015-01-17
 *
 *	Allocate input data buffer
 *
 * @return	64KB data buffer
 *
 */
uint8_t * CMyRUDP_Client::AllocateInputDataBuf()
{
	uint8_t * pRetVal = NULL;

    while( THREAD_STATE_RUNNING == m_nThreadState )
	{	// check the thread is running or not
	#ifdef __TEST_USING_THREAD_POOL__
		pRetVal = m_InCacheBuf.Allocate( READ_DATA_PACKET_MAX_SIZE, 0 );
		if( NULL == pRetVal )
			return NULL;
	#else
		// 2015.4.28 CYJ Add, change to -1, if thread to be quit, using m_InCacheBuf.Wakeup
		pRetVal = m_InCacheBuf.Allocate( READ_DATA_PACKET_MAX_SIZE, -1 );
	#endif
		if( pRetVal )
			return pRetVal;		// allocate succ
		sched_yield();
	}
	return NULL;
}


//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	Read data working thread
 */
void CMyRUDP_Client::Run_ReadData()
{
	m_tNow = time( NULL );

#ifdef _DEBUG
	MyRUDP_fprintf( "--- Server IP: 0x%08x.\n", m_ServerAddr.sin_addr.s_addr );
#endif // _DEBUG

	if( 0 == m_ServerAddr.sin_addr.s_addr )
		GetIPFromDSN();

	while( THREAD_STATE_RUNNING == m_nThreadState )
	{
		// in the DoReceiveData function, will sleep 500 ms
		DoReceiveData();
	}
}

//--------------------------------------------------------------
/** CYJ 2015-05-07
 *
 *	Get IP from DSN
 */
void CMyRUDP_Client::GetIPFromDSN()
{
#ifdef _DEBUG
	DEBUG_PRINT_ENTER_FUNCTION();
#endif // _DEBUG

	struct hostent hostinfo;
	struct hostent *hostaddr;

	int nRetVal = 0;
	int nErrNo;
	int i;

	#define HOST_ADDR_BUF_SIZE 8192
	CMyRUDP_AutoDeleteBuf HostAddrBuf( HOST_ADDR_BUF_SIZE );
	if( false == HostAddrBuf.IsValid() )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- Failed to allocate memory to gethostbyname_r\n" );
	#endif //_DEBUG
		return;
	}
	unsigned char * pbyHostAddrBuf = HostAddrBuf.GetBuffer();

#ifdef _DEBUG
	MyRUDP_fprintf( "[ MYDNS ] my_gethostbyname_working_thread ENTER.\n" );
#endif // #ifdef _MY_DNS_DEBUG

	// 2012.12.29 CYJ Modify, add process TRY_AGAIN error code and max try times = 10
	for( i=0; i<10; i++ )
	{
		memset( pbyHostAddrBuf, 0, HOST_ADDR_BUF_SIZE );
		memset( &hostinfo, 0, sizeof(hostinfo) );
		hostaddr = NULL;
		nRetVal = gethostbyname_r( m_strServerIP.c_str(), &hostinfo, (char*)pbyHostAddrBuf, HOST_ADDR_BUF_SIZE, &hostaddr, &nErrNo );
		if( TRY_AGAIN != nRetVal )
			break;
	}

	if( nRetVal || NULL == hostaddr || NULL == hostaddr->h_addr_list[0] || hostaddr->h_length <= 0 || hostaddr->h_length > (int)sizeof(struct in_addr) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "[ MYDNS ] gethostbyname_r(%s ) failed, errno=%d\n", m_strServerIP.c_str(), h_errno );
		if( hostaddr )
			MyRUDP_fprintf(  "[ MYDNS ] hostaddr->h_addr_list[0]=%p, len=%d\n", hostaddr->h_addr_list[0], hostaddr->h_length );
	#endif // #ifdef _DEBUG
		return;
	}

	unsigned char byIPMask = 0;
	unsigned char * pabyIPAddr = (unsigned char*)hostaddr->h_addr_list[0];
	for( i=0; i<hostaddr->h_length; i++ )
	{
		byIPMask |= pabyIPAddr[i];
	}
	if( 0 == byIPMask )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "get IP Address failed, sicne all IP is 0\n" );
	#endif // #ifdef _MY_DNS_DEBUG
		return;
	}

	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_SendOut );
	memcpy( &m_ServerAddr.sin_addr, pabyIPAddr, hostaddr->h_length );

#ifdef _DEBUG
	MyRUDP_fprintf( "Get IP ( %s ) of %s\n", inet_ntoa(m_ServerAddr.sin_addr), m_strServerIP.c_str() );
#endif // _DEBUG

	// 2015.5.12 CYJ Add, wake up process thread to connect to server
	m_InCacheBuf.Wakeup( true, false );
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	On Timer Per second
 */
void CMyRUDP_Client::OnTimerOneSecond()
{
	// send keep alive command
	if( MYRUDP_CLIENT_STATE_WAIT_RSP == m_nState )
	{
		m_nSecondsToResendReqOrSyncCmd ++;
		if( m_nSecondsToResendReqOrSyncCmd > SECONDS_RESEND_REQ_CMD )
			SendReqCmd();			// resend REQ command
	}
	// 2015.5.4 CYJ Add
	if( MYRUDP_CLIENT_STATE_WAIT_SYNC_2 == m_nState )
	{
		m_nSecondsToResendReqOrSyncCmd ++;
		if( m_nSecondsToResendReqOrSyncCmd > 2 )
			SendSyncCmd();
		if( m_nSecondsToResendReqOrSyncCmd > SECONDS_RESEND_SYN_CMD )
		{	// timeout, close and reconnect
			SendCloseCmdToServer();
			// let the timer to disconnect from the server and re-connect
			m_nSecondsFromLastReceiveFromServer = m_nSecondsToReconnectToServer;
		}
	}

	// 心跳包发送策略
	// 每隔 N 秒向服务器发送一次心跳包
	// 如果在5秒内接收到服务器来的数据，则不再发送，反之，间隔1秒再次发送心跳包（连续5秒）
	// 实际发送次数 ( KEEP_ALIVE_PACKET_MAX_SEND_COUNT+1 ）
	#define KEEP_ALIVE_PACKET_MAX_SEND_COUNT	4
	if( MYRUDP_CLIENT_STATE_CONNECTED == m_nState && --m_nCounterToSendKeepAliveCmd <= KEEP_ALIVE_PACKET_MAX_SEND_COUNT )
	{
		SendKeepAliveToServer();
		if( m_nCounterToSendKeepAliveCmd <= 0 || m_nSecondsFromLastReceiveFromServer <= 5 )
			m_nCounterToSendKeepAliveCmd = m_nSendKeepAliveIntervalInSeconds;
	}

	m_nSecondsFromLastReceiveFromServer ++;
	if( m_nSecondsFromLastReceiveFromServer >= m_nSecondsToReconnectToServer )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s, %d seconds no data from server, should reconnect.\n", __FUNCTION__, m_nSecondsFromLastReceiveFromServer );
	//	assert( false );
	#endif // _DEBUG

		m_nSecondsFromLastReceiveFromServer = 0;
		m_InPacketMgr.Invalidate();
		CMyRUDP_Packet_Sender::Invalidate();
		m_nState = MYRUDP_CLIENT_STATE_DISCONNECTED;
		// notify start connnecting
		if( m_pEventObj )
			m_pEventObj->OnConnectionStateChanged( CMyRUDP_EventObj::MY_RUDP_CONNECTION_STATE_DISCONNECTED );
	}

	CMyRUDP_Packet_Sender::OnTimer( m_tNow );
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	Do receive data
 *
 */
void CMyRUDP_Client::DoReceiveData()
{
#ifdef _DEBUG
//	DEBUG_PRINT_ENTER_FUNCTION();
#endif // _DEBUG

#ifdef __TEST_USING_THREAD_POOL__
	struct timeval timeout_val;
    timeout_val.tv_sec = 0;			// timeout 50 ms
	timeout_val.tv_usec = 0;
	#define pSelectTimeOut	 (&timeout_val)
#else
	#define pSelectTimeOut	 NULL
#endif // __TEST_USING_THREAD_POOL__

    // wait for any data ready
    fd_set fdset;
	FD_ZERO( &fdset );
	FD_SET( m_hSocket, &fdset );
	// 2015.4.28 CYJ Add, using a pipe to wake up select
	int nSelectMaxHandle = m_hSocket;
	if( m_aPipeToWakeupSelect[0] >= 0 )
	{
		if( m_aPipeToWakeupSelect[0] > nSelectMaxHandle )
			nSelectMaxHandle = m_aPipeToWakeupSelect[0];
		FD_SET( m_aPipeToWakeupSelect[0], &fdset );
	}

	int nRetVal = select( nSelectMaxHandle + 1, &fdset, NULL, NULL, pSelectTimeOut );
	if( nRetVal <= 0 )
	{
	#ifdef _DEBUG
		if(nRetVal < 0 )
			MyRUDP_fprintf( "select failed. m_hSocket=%d, errno=%d\n", m_hSocket, errno );
	#endif // _DEBUG
		return;						// timeout
	}
	// 2015.4.28 CYJ Add, to check m_hSocket is set or not
	if( 0 == FD_ISSET(m_hSocket, &fdset) )
		return;

	while( THREAD_STATE_RUNNING == m_nThreadState )
	{
		uint8_t * pBuf = AllocateInputDataBuf();
		if( NULL == pBuf )
			break;

		struct sockaddr_in peer_addr;
		socklen_t peer_addr_len = sizeof(peer_addr);
		int nRetVal = recvfrom( m_hSocket, pBuf, READ_DATA_PACKET_MAX_SIZE, MSG_DONTWAIT, (sockaddr*)&peer_addr, &peer_addr_len );
		if( nRetVal <= 0 )
		{
			// FIXME, should deal with error code
		#ifdef _DEBUG
			if( errno != EAGAIN && errno != EWOULDBLOCK )
				MyRUDP_fprintf( "recvfrom failed, error code=%d / %d\n", nRetVal, errno );
		#endif // _DEBUG
			break;						// no data or error
		}

	#if defined(__RAND_LOST_DATA_RATE__)
		if( s_dwTotalPacketReceived >= 0x7FFFFFFF )
		{
			s_dwTotalPacketReceived >>= 1;
			s_dwDiscardPacketReceived >>= 1;
		}
		s_dwTotalPacketReceived ++;

		if( MY_RAND_BYTE() < __RAND_LOST_DATA_RATE__ )
		{
			s_dwDiscardPacketReceived ++;
		//	MyRUDP_fprintf( "++++++++++++++ Abort received data\n" );
			MyRUDP_fprintf( "{DISCARD ONE PACKET, %5.1f%%}", ((double)s_dwDiscardPacketReceived)*100/s_dwTotalPacketReceived );
			continue;
		}
	#endif // _DEBUG

		if( peer_addr.sin_port != m_ServerAddr.sin_port )
		{
		#ifdef _DEBUG
			MyRUDP_fprintf( "---- %s : %d, Port not same %d != %d.\n", __FUNCTION__, __LINE__, ntohs(peer_addr.sin_port), ntohs(m_ServerAddr.sin_port) );
		#endif // _DEBUG
			continue;				// not data from the server
		}
		if( nRetVal < MY_RUDP_HEADER_LEN )
		{
		#ifdef _DEBUG
			MyRUDP_fprintf( "---- %s : %d, error data length %d < %d.\n", __FUNCTION__, __LINE__, nRetVal, MY_RUDP_HEADER_LEN );
		#endif // _DEBUG
			continue;				// not my data
		}
		if( MYRUDP_MAGIC_ID_HI != pBuf[0] || MYRUDP_MAGIC_ID_LOW != pBuf[1] )
		{
		#ifdef _DEBUG
			MyRUDP_fprintf( "---- %s : %d, Bad Magic Word %02x %02x\n", __FUNCTION__, __LINE__, pBuf[0], pBuf[1] );
		#endif // _DEBUG
			continue;				// magic word not match
		}
		uint8_t byCmdValue = ( pBuf[2] & 0x1F );
		if( byCmdValue >= MYRUDP_CMD_MAX_VALUE )
		{
		#ifdef _DEBUG
			MyRUDP_fprintf( "---- %s : %d, Bad Cmd %d\n", __FUNCTION__, __LINE__, byCmdValue );
		#endif // _DEBUG
			continue;				// unknown command
		}

		// check OK, submit it
		m_InCacheBuf.Submit( nRetVal, peer_addr );
	}
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	On data received from server
 *
 * @param [in]
 * @param [in]
 * @param [in]
 *
 * @return
 *
 */
void CMyRUDP_Client::OnDataReceivedFromServer( unsigned char *pBuf, int nLen, const struct sockaddr_in & SrcAddr )
{
#ifdef _DEBUG
//	DEBUG_PRINT_ENTER_FUNCTION();
//	DbgDumpData( "Recv: ", pBuf, nLen );
#endif // _DEBUG
	if( CCRC::GetCRC16( MY_RUDP_HEADER_LEN, pBuf) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "---- %s : %d, Bad Header CRC16\n", __FUNCTION__, __LINE__ );
	#endif // _DEBUG
		return;				// header CRC checksum error
	}

	int nOldSecondsRecievedFromServer = m_nSecondsFromLastReceiveFromServer;
	m_nSecondsFromLastReceiveFromServer = 0;

	OnDataReceivedFromPeer();		// 2015.5.22 CYJ Add

	uint8_t byCmdValue = ( pBuf[2] & 0x1F );
	switch( byCmdValue )
	{
	case MYRUDP_CMD_NOP:
		OnKeepAlivePacketRecieved( nOldSecondsRecievedFromServer );			// 2015.5.9 CYJ Add
		return;

	case MYRUDP_CMD_REQ:		// request connecting
	case MYRUDP_CMD_SYN:		// sync for connecting
		// not support now, may be used for P2P connection
		return;

	case MYRUDP_CMD_RSP:		// response for connecting
		OnRspCmd( pBuf, nLen );
		return;

	case MYRUDP_CMD_SYN_2:		//	2015.5.4 CYJ Add
		OnSync2Cmd( pBuf, nLen );
		return;

	case MYRUDP_CMD_DATA:		// sending data
		OnDataCmdFromServer( pBuf, nLen );
		break;

	case MYRUDP_CMD_ACK:		// sending ACK
		OnAckCmdFromServer( pBuf, nLen );
		break;

	case MYRUDP_CMD_RESEND:		// request resend
		OnResendCmdDataPacket( pBuf, nLen );
		return;

	case MYRUDP_CMD_SKIP:		// skip, 2015.4.2 CYJ Add
		OnSkipCmdFromServer( pBuf, nLen );
		return;

	case MYRUDP_CMD_CLOSE:		// close the connection
		// server is disconnect, re-connect again
		OnCmdCloseFromServer( pBuf );
		break;
	}
}

//--------------------------------------------------------------
/** CYJ 2015-05-09
 *
 *	On keep alive packet received from the server
 */
void CMyRUDP_Client::OnKeepAlivePacketRecieved( int nSecondsLastRecived )
{
#ifdef _DEBUG
	MyRUDP_fprintf( "---- %s : %d, KeepAlive Packet received from server\n", __FUNCTION__, __LINE__ );
#endif // _DEBUG

	ForceSendoutPacket( nSecondsLastRecived < 10 ? 1 : 10 );

    if( m_pEventObj )
		m_pEventObj->OnKeepAlivePacketReceived( nSecondsLastRecived );
}

//--------------------------------------------------------------
/** CYJ 2015-01-30
 *
 *	On data command received from the server
 *
 * @param [in]	pBuf				data buffer
 * @param [in]	nLen				data length
 */
void CMyRUDP_Client::OnDataCmdFromServer( unsigned char *pBuf, int nLen )
{
#ifdef _DEBUG
//	DEBUG_PRINT_ENTER_FUNCTION();
#endif // _DEBUG

	if( false == VerifySessionAndTagID( pBuf ) )
		return;

	if( false == m_InPacketMgr.IsValid() )
		return;						// not ready

	// FIXME
	// do data decryption

	uint16_t awACKSeqNo[ MYRUDP_MAX_ACK_SEQNO_COUNT ];
	int nACKCount = sizeof(awACKSeqNo) / sizeof( awACKSeqNo[0] );
	int nRetVal = m_InPacketMgr.OnDataPacketReady( pBuf, nLen, awACKSeqNo, nACKCount );
	if( nRetVal < 0 )
	{
		if( -2 == nRetVal )
		{
			uint16_t wSeqNo = CMyRUDP_HeaderReader::GetSeqNo( pBuf );
			RequestPeerToResendPacket( (wSeqNo&0xFF) == (m_InPacketMgr.GetTailSeqNo()&0xFF) );

			if( m_InPacketMgr.HasDataToBeRead() )
			{
			#ifdef _DEBUG
				MyRUDP_fprintf( "********** Still has data to read, add to Task\n" );
			#endif // _DEBUG
				NotifyHasDataToBeProcess();
			}
		}
		return;		// error occured, discard the data packet
	}

	// send back ACK
#ifdef _DEBUG
	assert( nACKCount >= 1 && nACKCount <= MYRUDP_MAX_ACK_SEQNO_COUNT );
#endif // _DEBUG

	SendACKToPeer( awACKSeqNo, nACKCount );

	// one full data packet is received, should add to task and schedule
	if( nRetVal > 0 )
		NotifyHasDataToBeProcess();

	RequestPeerToResendPacket( false );
}

//--------------------------------------------------------------
/** CYJ 2015-02-23
 *
 *	Notify has data to be read from the in-buffer list
 */
void CMyRUDP_Client::NotifyHasDataToBeProcess()
{
	OnDataReadyEvent();
}

//--------------------------------------------------------------
/** CYJ 2015-02-23
 *
 *	Request the peer to resend packet
 *
 * @param [in]	bForce					bForce to resend
 */
void CMyRUDP_Client::RequestPeerToResendPacket( bool bForce )
{
	uint16_t awSeqNo[ MYRUDP_MAX_ACK_SEQNO_COUNT ];
	int nRetCount = m_InPacketMgr.GetResendSeqNo( awSeqNo, MYRUDP_MAX_ACK_SEQNO_COUNT, bForce );
	if( nRetCount <= 0 )
		return;

	#define RESEND_MAX_BUF_LEN ( MY_RUDP_HEADER_LEN + 2*MYRUDP_MAX_ACK_SEQNO_COUNT )

	uint8_t abyCmdBuf[ RESEND_MAX_BUF_LEN ];

	uint8_t * pBuf = abyCmdBuf;
	memset( pBuf, 0, RESEND_MAX_BUF_LEN );

	// SeqNo must be 0, to avoid encrypt sessionTagID
	PrepareCommonHeader( pBuf, MYRUDP_CMD_RESEND );

	uint16_t wTailSeqNo = m_InPacketMgr.GetTailSeqNo();
	pBuf[11] = (uint8_t)(wTailSeqNo>> 8);
	pBuf[12] = (uint8_t)wTailSeqNo;

	uint16_t wExpectedSeqNo = m_InPacketMgr.GetExpectedSeqNo();
	pBuf[13] = (uint8_t)( wExpectedSeqNo >> 8 );
	pBuf[14] = (uint8_t)( wExpectedSeqNo );

	CommitCommonHeader( pBuf );

	uint8_t * pTmpBuf = pBuf + MY_RUDP_HEADER_LEN;
	for(int i=0; i<nRetCount; i++ )
	{
		uint16_t wSeqNo = awSeqNo[i];
	#ifdef _DEBUG
		MyRUDP_fprintf( "======= Request resend: 0x%04x.\n", wSeqNo );
	#endif // _DEBUG

		pTmpBuf[0] = (uint8_t)( wSeqNo>>8 );
		pTmpBuf[1] = (uint8_t)( wSeqNo );
		pTmpBuf += 2;
	}

	// send RESEND to the peer
	// normal UDP packet, not need reliable
	SendRawDataToPeer( abyCmdBuf, RESEND_MAX_BUF_LEN + nRetCount * 2 );
}

//--------------------------------------------------------------
/** CYJ 2015-02-11
 *
 *	Send ACK SeqNo to peer
 *
 * @param [in]	awACKSeqNo			ACK SeqNo
 * @param [in]	nCount				ACK SeqNo Count
 */
void CMyRUDP_Client::SendACKToPeer( const uint16_t awACKSeqNo[], int nCount )
{
	#define ACK_MAX_BUF_LEN ( MY_RUDP_HEADER_LEN + MYRUDP_MAX_ACK_SEQNO_COUNT*2 + 16 )	// keep 16 bytes

	uint8_t abyCmdBuf[ ACK_MAX_BUF_LEN ];

	uint8_t * pBuf = abyCmdBuf;
	memset( pBuf, 0, ACK_MAX_BUF_LEN );

	// SeqNo must be 0, to avoid encrypt sessionTagID
	PrepareCommonHeader( pBuf, MYRUDP_CMD_ACK );

	uint16_t wTailSeqNo = m_InPacketMgr.GetTailSeqNo();
	pBuf[11] = (uint8_t)(wTailSeqNo>> 8);
	pBuf[12] = (uint8_t)wTailSeqNo;

	uint16_t wExpectedSeqNo = m_InPacketMgr.GetExpectedSeqNo();
	pBuf[13] = (uint8_t)( wExpectedSeqNo >> 8 );
	pBuf[14] = (uint8_t)( wExpectedSeqNo );

	CommitCommonHeader( pBuf );

	uint8_t * pTmpBuf = pBuf + MY_RUDP_HEADER_LEN;
	// 2015.4.2 CYJ modify, add Mode field
	pTmpBuf[0] = MY_RUDP_ACK_MODE_ARRAY;
	pTmpBuf ++;
	for(int i=0; i<nCount; i++ )
	{
		uint16_t wSeqNo = awACKSeqNo[i];
		pTmpBuf[0] = (uint8_t)( wSeqNo>>8 );
		pTmpBuf[1] = (uint8_t)( wSeqNo );
		pTmpBuf += 2;
	}

	int nOutDataLen = MY_RUDP_HEADER_LEN + nCount*2 + 1;

#ifdef _DEBUG
	assert( nOutDataLen == pTmpBuf - abyCmdBuf );
#endif // _DEBUG

	// send ACK to the peer
	// normal UDP packet, not need reliable
	SendRawDataToPeer( abyCmdBuf, nOutDataLen );
}

//--------------------------------------------------------------
/** CYJ 2015-04-02
 *
 *	send ACK in range mode
 *
 * @param [in]	wHeadSeqNo						the Head SeqNo has been received
 * @param [in]	wTailSeqNo						the Tail SeqNo has been received
 */
void CMyRUDP_Client::SendACKToPeer( uint16_t wHeadSeqNo, uint16_t wTailSeqNo )
{
	uint8_t abyCmdBuf[ ACK_MAX_BUF_LEN ];

	uint8_t * pBuf = abyCmdBuf;
	memset( pBuf, 0, ACK_MAX_BUF_LEN );

	// SeqNo must be 0, to avoid encrypt sessionTagID
	PrepareCommonHeader( pBuf, MYRUDP_CMD_ACK );

	uint16_t wInDataTailSeqNo = m_InPacketMgr.GetTailSeqNo();
	pBuf[11] = (uint8_t)(wInDataTailSeqNo>> 8);
	pBuf[12] = (uint8_t)wInDataTailSeqNo;

	uint16_t wExpectedSeqNo = m_InPacketMgr.GetExpectedSeqNo();
	pBuf[13] = (uint8_t)( wExpectedSeqNo >> 8 );
	pBuf[14] = (uint8_t)( wExpectedSeqNo );

	CommitCommonHeader( pBuf );

	uint8_t * pTmpBuf = pBuf + MY_RUDP_HEADER_LEN;
	// 2015.4.2 CYJ modify, add Mode field
	pTmpBuf[0] = MY_RUDP_ACK_MODE_RANGE;
	pTmpBuf ++;

	// wHeadSeqNo
	pTmpBuf[0] = (uint8_t)( wHeadSeqNo>>8 );
	pTmpBuf[1] = (uint8_t)( wHeadSeqNo );
	// wTailSeqNo
	pTmpBuf[2] = (uint8_t)( wTailSeqNo>>8 );
	pTmpBuf[3] = (uint8_t)( wTailSeqNo );

	int nOutDataLen = MY_RUDP_HEADER_LEN + 1 + 4;

	// send ACK to the peer
	// normal UDP packet, not need reliable
	SendRawDataToPeer( abyCmdBuf, nOutDataLen );
}

//--------------------------------------------------------------
/** CYJ 2015-01-30
 *
 *	On ACK command received from the server
 *
 * @param [in]	pBuf				data buffer
 * @param [in]	nLen				data length
 */
void CMyRUDP_Client::OnAckCmdFromServer( unsigned char *pBuf, int nLen )
{
#ifdef _DEBUG
//	DEBUG_PRINT_ENTER_FUNCTION();
#endif // _DEBUG

	if( false == VerifySessionAndTagID( pBuf ) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "!!!!!!!!!! Invalid Session or TagID.\n" );
		DbgDumpData( "Invalid SessionID data" ,pBuf, nLen );
	#endif // _DEBUG
		return;
	}

	uint16_t wPeerTailSeqNo = CMyRUDP_HeaderReader::GetSeqNo( pBuf );
	uint16_t wPeerRcvHeadSeqNo = pBuf[13] * 0x100 + pBuf[14];

	pBuf += MY_RUDP_HEADER_LEN;
	nLen -= MY_RUDP_HEADER_LEN;

	// 2015.4.2 CYJ Add, to support ACK in 2 modes
	int nMode = pBuf[0];
	pBuf ++;
	nLen --;

#ifdef _DEBUG
	if( MY_RUDP_ACK_MODE_ARRAY == nMode )
		assert( 0 == (nLen&1) );
	else
		assert( 0 == (nLen&3) );
#endif // _DEBUG

	if( MY_RUDP_ACK_MODE_ARRAY == nMode )
	{
		uint16_t awACKSeqNo[ MYRUDP_MAX_ACK_SEQNO_COUNT ];
		int nAckCount = nLen / 2;
		if( nAckCount > MYRUDP_MAX_ACK_SEQNO_COUNT )
			nAckCount = MYRUDP_MAX_ACK_SEQNO_COUNT;

		// convert ACK endian
		for(int i=0; i<nAckCount; i++ )
		{
			awACKSeqNo[i] = pBuf[0] * 0x100 + pBuf[1];
			pBuf += 2;
		}

		CMyRUDP_Packet_Sender::OnACK( awACKSeqNo, nAckCount, wPeerRcvHeadSeqNo, wPeerTailSeqNo );
	}
	else if( MY_RUDP_ACK_MODE_RANGE == nMode )
	{	// range mode
		int nAckCount = nLen / 4;
		for(int i=0; i<nAckCount; i++ )
		{
			uint16_t wHeadSeqNo = pBuf[0] * 0x100 + pBuf[1];
			uint16_t wTailSeqNo = pBuf[2] * 0x100 + pBuf[3];
			pBuf += 4;
			CMyRUDP_Packet_Sender::OnACK( wHeadSeqNo, wTailSeqNo, wPeerRcvHeadSeqNo, wPeerTailSeqNo );
		}
	}
	else
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "%s, unknown ACK value: 0x%02x.\n", __FUNCTION__, nMode );
		assert( false );
	#endif // _DEBUG
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-23
 *
 *	Resend command
 *
 * @param [in]	pBuf				data buffer
 * @param [in]	nLen				data length
 */
void CMyRUDP_Client::OnResendCmdDataPacket( unsigned char *pBuf, int nLen )
{
	if( false == VerifySessionAndTagID( pBuf ) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "!!!!!!!!!! Invalid Session or TagID.\n" );
		DbgDumpData( "Invalid SessionID data" ,pBuf, nLen );
	#endif // _DEBUG
		return;
	}

	uint16_t wPeerTailSeqNo = CMyRUDP_HeaderReader::GetSeqNo( pBuf );
	uint16_t wPeerRcvHeadSeqNo = pBuf[13] * 0x100 + pBuf[14];

	pBuf += MY_RUDP_HEADER_LEN;
	nLen -= MY_RUDP_HEADER_LEN;

	uint16_t awSeqNo[ MYRUDP_MAX_ACK_SEQNO_COUNT ];
	int nCount = nLen / 2;
	if( nCount > MYRUDP_MAX_ACK_SEQNO_COUNT )
		nCount = MYRUDP_MAX_ACK_SEQNO_COUNT;

	// convert ACK endian
	for(int i=0; i<nCount; i++ )
	{
		awSeqNo[i] = pBuf[0] * 0x100 + pBuf[1];
		pBuf += 2;
	}

	CMyRUDP_Packet_Sender::OnResend( awSeqNo, nCount, wPeerRcvHeadSeqNo, wPeerTailSeqNo );
}

//--------------------------------------------------------------
/** CYJ 2015-01-30
 *
 *	Verify session ID and session tag ID
 *
 * @param [in]	pBuf			data buffer
 *
 * @return		true			OK
 *				false			not a valid session
 */
bool CMyRUDP_Client::VerifySessionAndTagID( uint8_t * pBuf )
{
	CMyRUDP_HeaderReader Reader( pBuf );

	if( Reader.GetSessionID() != m_dwSessionID )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s, session ID not same: %08x, %08x \n", __FUNCTION__, Reader.GetSessionID(), m_dwSessionID );
	#endif // _DEBUG
		return false;			// session ID not same
	}

	uint32_t dwSeqNo = Reader.GetSeqNo();
	uint32_t dwSessionTagID = Reader.GetSessionTagID();
	if( dwSeqNo )
	{	// descramble sessionTagID
		// SessionTagID <=  ( OrgSessionTagID ^ s_adwXorData[SeqNo] ) + (SeqNo << 10) + (PacketIndex << 4) + DataPaddLen
		// OrgSessionTagID = SessionTagID - DataPaddLen - (SeqNo << 10) - (PacketIndex << 4);
		// OrgSessionTagID ^= s_adwXorData[SeqNo];

		dwSessionTagID -= Reader.GetPaddingDataLen();
		dwSessionTagID -= ( Reader.GetPackteIndex() << 4 );
		dwSessionTagID -= ( dwSeqNo << 10 );

		uint32_t dwXorData = 0;
		uint8_t *pXorData = m_abyXorData;
		for( int i=0; i<4; i++ )
		{
			dwXorData <<= 8;
			dwXorData |= pXorData[ (dwSeqNo + i) & 0x1F ];
		}

		dwSessionTagID ^= dwXorData;
	}

#ifdef _DEBUG
	if( dwSessionTagID != m_dwSessionTagID )
		MyRUDP_fprintf( "--- %s, session: %08x, %08x \n", __FUNCTION__, dwSessionTagID, m_dwSessionTagID );
#endif // _DEBUG

	return ( dwSessionTagID == m_dwSessionTagID );
}

//--------------------------------------------------------------
/** CYJ 2015-01-30
 *
 *	On Close Command received from the server
 *
 * @param [in]	pBuf			data buffer
 * @param [in]	nLen			data length
 */
void CMyRUDP_Client::OnCmdCloseFromServer( uint8_t * pBuf )
{
#ifdef _DEBUG
	DEBUG_PRINT_ENTER_FUNCTION();
#endif // _DEBUG

	if( false == VerifySessionAndTagID( pBuf ) )
		return;
	m_nState = MYRUDP_CLIENT_STATE_DISCONNECTED;
}

//---------------------------
void *CMyRUDP_Client::RunLink_ReadData( void * pThis )
{
	((CMyRUDP_Client*)pThis)->Run_ReadData();
	return NULL;
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	Process data working thread
 */
void CMyRUDP_Client::Run_ProcessData()
{
	while( THREAD_STATE_RUNNING == m_nThreadState )
	{
	#if !defined( __TEST_USING_THREAD_POOL__ ) && !defined(ANDROID)
		// 2015.4.3 CYJ add, network is not available, sleep 1 second
		if( false == m_bNetworkIsAvailable )
			sleep( 1 );
	#endif // #ifndef __TEST_USING_THREAD_POOL__

		time_t tNow = time(NULL);
		if( m_tNow != tNow )
		{
		#if defined(_DEBUG) && defined(ANDROID)
			MyRUDP_fprintf( "----- OnTimerOneSecond, %d seconds\n", tNow - m_tNow );
		#endif // _DEBUG
			m_tNow = tNow;
			OnTimerOneSecond();
		}
		if( m_bNeedFastTimer )
		{
			m_bNeedFastTimer = false;
			OnFastTimer();
		}

		if( m_bNetworkIsAvailable && MYRUDP_CLIENT_STATE_DISCONNECTED == m_nState )
			SendReqCmd();		// connection is closed, request connect to server

		PBYTE pBuf = NULL;
		DWORD dwBufLen;
	#ifdef __TEST_USING_THREAD_POOL__
		struct sockaddr_in * pPeerAddr = m_InCacheBuf.Peek( pBuf, dwBufLen, 0 );
		if( NULL == pPeerAddr )
			return;
	#else
		#ifdef ANDROID
			// for Android, there is another timer to call the SendKeepAlive
			// 2015.5.12 CYJ Add, check m_ServerAddr.sin_addr.s_addr, if m_ServerAddr.sin_addr.s_addr = 0 then sleep a long time
			int nWaitTimeOut;
			if( m_bNetworkIsAvailable && m_ServerAddr.sin_addr.s_addr &&\
				(	HasDataToBeSend() ||\
					MYRUDP_CLIENT_STATE_CONNECTED != m_nState ||\
					m_nSecondsFromLastReceiveFromServer >= m_nSecondsToReconnectToServer ||\
					m_nCounterToSendKeepAliveCmd <= KEEP_ALIVE_PACKET_MAX_SEND_COUNT\
				 )\
			   )
			{
				nWaitTimeOut = 1000;
			}
			else
				nWaitTimeOut = -1;
		#else
			int nWaitTimeOut = HasDataToBeSend() ? 500 : 1000;
		#endif // ANDROID

		struct sockaddr_in * pPeerAddr = m_InCacheBuf.Peek( pBuf, dwBufLen, nWaitTimeOut );
		if( NULL == pPeerAddr )
		{
			sched_yield();
			continue;
		}
	#endif // __TEST_USING_THREAD_POOL__

		OnDataReceivedFromServer( pBuf, (int)dwBufLen, *pPeerAddr );
		// free the data buffer
		m_InCacheBuf.Free();
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-25
 *
 *	On data ready event
 */
void CMyRUDP_Client::OnDataReadyEvent()
{
	while( THREAD_STATE_RUNNING == m_nThreadState )
	{
		int nNextDataLen = m_InPacketMgr.GetNextPacketDataLen();
		if( nNextDataLen <= 0 )
			break;			// no data
		nNextDataLen += 64;
		uint8_t * pBuf = m_pEventObj->OnDataReceived_Allocate( nNextDataLen );
		if( NULL == pBuf )
			return;
		int nRetVal = m_InPacketMgr.CopyData( pBuf, MYRUDP_MAX_DATA_LENGTH );
		if( 0 == nRetVal )
		{	// no data
			m_pEventObj->OnDataReceived_Free( pBuf );
			return;
		}
	#ifdef _DEBUG
		assert( nRetVal > 0 );
	#endif // _DEBUG
		if( nRetVal < 0 )
		{	// buffer too small, skip the data
			m_pEventObj->OnDataReceived_Free( pBuf );
			if( m_InPacketMgr.SkipData() )
				continue;		// has more data
			return;
		}
		m_pEventObj->OnDataReceived_Commit( pBuf, nRetVal, nNextDataLen );
	}
}

//------------------------------------------------------------
void *CMyRUDP_Client::RunLink_ProcessData( void * pThis )
{
	((CMyRUDP_Client*)pThis)->Run_ProcessData();
	return NULL;
}

#ifdef _DEBUG
void CMyRUDP_Client::DebugDump()
{
	CMyRUDP_Packet_Sender::DebugDump();
//	m_InPacketMgr.DebugDump();
}
#endif //_DEBUG

//--------------------------------------------------------------
/** CYJ 2015-02-15
 *
 *	Notify one packet has been send succ
 *
 * @param [in]	pUserData			user data
 */
void CMyRUDP_Client::OnPacketHasBeenSendSucc( void * pUserData )
{
#ifdef _DEBUG
//	MyRUDP_fprintf( "--- %s( %p )\n", __FUNCTION__, pUserData );
#endif // _DEBUG

	if( m_pEventObj && pUserData )
		m_pEventObj->OnDataSentSucc( pUserData );
}

//--------------------------------------------------------------
/** CYJ 2015-04-02
 *
 *	Notify one packet has been send failed
 *
 * @param [in]	wHeadSeqNo			HeadSeqNo
 * @param [in]	wTailSeqNo			TailSeqNo
 * @param [in]	pUserData			user data
 *
 * @note
 *		should do 2 step:
 *		(1)			send SKIP command to peer
 *		(2)			send send data failed event to APP
 */
void CMyRUDP_Client::OnPacketSendFailed( uint16_t wHeadSeqNo, uint16_t wTailSeqNo, void * pUserData )
{
#ifdef _DEBUG
	MyRUDP_fprintf( "--- %s( 0x%04x, 0x%04x, %p )\n", __FUNCTION__, wHeadSeqNo, wTailSeqNo, pUserData );
#endif // _DEBUG
	// build and send SKIP command to peer

	#define SKIP_MAX_BUF_LEN ( MY_RUDP_HEADER_LEN + 4 + 16 )	// keep 16 bytes

	uint8_t abyCmdBuf[ SKIP_MAX_BUF_LEN ];

	uint8_t * pBuf = abyCmdBuf;
	memset( pBuf, 0, SKIP_MAX_BUF_LEN );

	// SeqNo must be 0, to avoid encrypt sessionTagID
	PrepareCommonHeader( pBuf, MYRUDP_CMD_SKIP );
	CommitCommonHeader( pBuf );

	uint8_t * pTmpBuf = pBuf + MY_RUDP_HEADER_LEN;
	// wHeadSeqNo
	pTmpBuf[0] = (uint8_t)( wHeadSeqNo>>8 );
	pTmpBuf[1] = (uint8_t)( wHeadSeqNo );
	// wTailSeqNo
	pTmpBuf[2] = (uint8_t)( wTailSeqNo>>8 );
	pTmpBuf[3] = (uint8_t)( wTailSeqNo );

	int nOutDataLen = MY_RUDP_HEADER_LEN + 4;

	// send SKIP to the peer, normal UDP packet, not need reliable
	SendRawDataToPeer( abyCmdBuf, nOutDataLen );

	// send event
	if( m_pEventObj && pUserData )
		m_pEventObj->OnSendDataFailed( pUserData );
}

//--------------------------------------------------------------
/** CYJ 2015-02-26
 *
 *	Register Fast timer callback function
 */
void CMyRUDP_Client::RegisterFastTimer()
{
	m_bNeedFastTimer = true;
}

//--------------------------------------------------------------
/** CYJ 2015-02-12
 *
 *	Send data
 *
 * @param [in]	pBuf				data buffer
 * @param [in]	nLen				data length
 * @param [in]	pUserData			user data, used for notify data has been send succ.
 * @param [in]	nTimeOut			timeout seconds
 *									0  		if no enough buffer then return immediately
 *									-1		wait infinitely
 *									other	time out seconds
 * @return		0					succ
 *				other				error code
 *
 * @note
 *	If server IP has not been resolve from the DSN, disable sending
 */
int CMyRUDP_Client::Send( const uint8_t * pBuf, int nLen, void * pUserData, int nTimeOut )
{
	// 2015.5.8 CYJ Add, check server IP ( if not get server IP by URL )
	// or network is not available, can not send
	if( 0 == m_ServerAddr.sin_addr.s_addr || false == m_bNetworkIsAvailable )
		return EAGAIN;

	int nRetVal = CMyRUDP_Packet_Sender::Send( pBuf, nLen, pUserData, nTimeOut );

	// 2015.4.28 CYJ Add
	// has data to be send, wake up Stop Peek sleeping
	m_InCacheBuf.Wakeup( true, false );

	return nRetVal;
}

//--------------------------------------------------------------
/** CYJ 2015-04-02
 *
 *	On Skip command from server
 *
 * @param [in]	pBuf				data buffer
 * @param [in]	nLen				data length
*/
void CMyRUDP_Client::OnSkipCmdFromServer( unsigned char *pBuf, int nLen )
{
#ifdef _DEBUG
//	DEBUG_PRINT_ENTER_FUNCTION();
#endif // _DEBUG

	if( false == VerifySessionAndTagID( pBuf ) )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "!!!!!!!!!! Invalid Session or TagID.\n" );
		DbgDumpData( "Invalid SessionID data" ,pBuf, nLen );
	#endif // _DEBUG
		return;
	}

	pBuf += MY_RUDP_HEADER_LEN;
	nLen -= MY_RUDP_HEADER_LEN;

	int nCount = nLen / 4;
	// convert ACK endian
	for(int i=0; i<nCount; i++ )
	{
		uint16_t wHeadSeqNo = pBuf[0] * 0x100 + pBuf[1];
		uint16_t wTailSeqNo = pBuf[2] * 0x100 + pBuf[3];
		pBuf += 4;

	#ifdef _DEBUG
		MyRUDP_fprintf( "[SKIP] 0x%04x - 0x%04x\n", wHeadSeqNo, wTailSeqNo );
	#endif // _DEBUG

		m_InPacketMgr.OnSkipData( wHeadSeqNo, wTailSeqNo );

		// build and send fake ACK to peer
		SendACKToPeer( wHeadSeqNo, wTailSeqNo );
	}
}

//--------------------------------------------------------------
/** CYJ 2015-04-03
 *
 *	Set keep alive packet interval, in seconds
 *
 * @param [in]	nInterval			keep alive packet interval, in seconds
 *									default is 10 seconds
 *									[ 10 - 300 ]
 */
void CMyRUDP_Client::SetKeepAliveInterval( int nInterval )
{
#if 0 && defined(_DEBUG)
	if( nInterval < 10 )
		MyRUDP_fprintf( "----------- %s ( %d ), too small, change to 10--------\n", __FUNCTION__, nInterval );
	else
		MyRUDP_fprintf( "----------- %s ( %d ) -----------\n", __FUNCTION__, nInterval );
#endif // _DEBUG

	if( nInterval < 10 )
		nInterval = 10;

	m_nSendKeepAliveIntervalInSeconds = nInterval;
	m_nSecondsToReconnectToServer = m_nSendKeepAliveIntervalInSeconds * 2;

	// 2015.4.9 CYJ Modify, to avoid too long
	if( m_nSecondsToReconnectToServer > m_nSendKeepAliveIntervalInSeconds + 60 )
		m_nSecondsToReconnectToServer = m_nSendKeepAliveIntervalInSeconds + 60;

	CheckNetworkConnection();

	// 2015.5.2 CYJ Add, wake up receive thread
	m_InCacheBuf.Wakeup( true, false );
}

//--------------------------------------------------------------
/** CYJ 2015-04-03
 *
 *	Set network is avaiable or not
 *
 * @param [in]	bEnable				network is available or not
 *									true		available
 *									false		not available
 * @return		old value
 *
 * @note
 *	If network is not avaiable, then not try to connect to network, default is available
 */
bool CMyRUDP_Client::SetNetowrkAvailable( bool bEnable )
{
	bool bRetVal = m_bNetworkIsAvailable;
	m_bNetworkIsAvailable = bEnable;

	// 2015.5.2 CYJ Add
	if( false == bRetVal && m_bNetworkIsAvailable )
	{	// network change from unavaiable to available, re-connect to server
		m_nSecondsFromLastReceiveFromServer = m_nSecondsToReconnectToServer;
		m_InCacheBuf.Wakeup( true, false );
	}

	return bRetVal;
}

#ifdef __TEST_USING_THREAD_POOL__
	void CMyRUDP_Client::CReadDataTask::RunTask()
	{
		CSingleLock Sync( &m_ReadDataTaskSyncObj, true );

		m_pClient->DoReceiveData();

		if( m_pClient->m_nThreadState == THREAD_STATE_RUNNING )
		{
			if( MYRUDP_CLIENT_STATE_CONNECTED == m_pClient->GetState() )
				s_pClientThreadPool->AddDelayTask( this, 100 );	// wait 100 ms
			else
				s_pClientThreadPool->AddDelayTask( this, 20 );	// wait 20 ms
		}
	}
	void CMyRUDP_Client::CProcessDataTask::RunTask()
	{
		CSingleLock Sync( &m_ProcessDataTaskSyncObj, true );

		m_pClient->Run_ProcessData();

		if( m_pClient->m_nThreadState == THREAD_STATE_RUNNING )
		{
			if( m_pClient->m_InCacheBuf.GetCount() || m_pClient->HasDataToBeSend() )
				s_pClientThreadPool->AddTask( this );
			else
				s_pClientThreadPool->AddDelayTask( this, 200 );	// wait 200 ms
		}
	}
#endif // #ifdef __TEST_USING_THREAD_POOL__

//--------------------------------------------------------------
/** CYJ 2015-04-09
 *
 *	Check network station and sent keep alive packet, is disconnected, reconnect it
 *
 * @note
 *		For Android, sometimes the thread is wake up very long, so using this function to check the connection
 */
void CMyRUDP_Client::CheckNetworkConnection()
{
#ifdef _DEBUG
	DEBUG_PRINT_ENTER_FUNCTION();
#endif // _DEBUG

	if( MYRUDP_CLIENT_STATE_CONNECTED == m_nState )
	{	// send keep alive to server, should get the response (keep alive) packet from the server within N seconds
		// if not get the response packet, then assume the client should re-connect to server again
		m_nSecondsFromLastReceiveFromServer = m_nSecondsToReconnectToServer - MAX_SECONDS_SHOULD_GET_KAP;

		SendKeepAliveToServer();
		m_nCounterToSendKeepAliveCmd = 1;		// try again about 1 second later

		// wake up timer thread
		m_InCacheBuf.Wakeup( true, false );
	}
}

//--------------------------------------------------------------
/** CYJ 2015-04-13
 *
 *	Send data item to peer, need ACK
 *
 * @param [in]	pBuf				data buffer to be send
 * @param [in]	nLen				data length
 *
 * @return		0					succ
 *				other				error
 */
int CMyRUDP_Client::SendDataPacketToPeer( const uint8_t * pBuf, int nLen )
{
	return SendRawDataToPeer( pBuf, nLen );
}

