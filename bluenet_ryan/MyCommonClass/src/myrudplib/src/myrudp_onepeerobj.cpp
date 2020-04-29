/*******************************************************************************
 *
 *	My RUDP One Peer Object
 *
 *	Chen Yongjian @ Zhoi
 *	2015.1.12 @ Xi'an
 *
 *
 ******************************************************************************/

#include <stdafx.h>
#include <mydatatype.h>
#include <MySyncObj.h>
#include <my_log_print.h>
#include <errno.h>
#include <my_pthread_mutex_help.h>
#include <mythreadpool.h>

#include "myrudp_onepeerobj.h"
#include "myrudp_socket.h"
#include "myrudp_cmd_value.h"
#include "myrudp_con_helper_svr.h"
#include "myrudp_dbgprint.h"
#include "myrudp_peerobj_task.h"
#include "myrudp_event.h"

#ifdef __MYRUDP_USE_OPENSSL__
	#include <openssl/rand.h>
#endif // __MYRUDP_USE_OPENSSL__

//------------------------------------------------
// session counter
static unsigned int 		s_dwSessionCounter = 0;

#ifdef _DEBUG
extern void DbgDumpData( const char *pszTitle, const uint8_t * pBuf, int nLen );
int g_nOnePeerObjectNumber = 0;
#endif // _DEBUG

/////////////////////////////////////////////////////////////

CMyRUDP_OnePeerObject::CMyRUDP_OnePeerObject( CMyRUDP_Socket * pSessionMgr )
{
	m_nRefCount = 1;
	m_pSessionMgr = pSessionMgr;
	m_nSesstionID = -1;
	m_dwSesstionTagID = 0;
	m_nState = PEER_OBJECT_STATE_IDLE;
	pthread_mutex_init( &m_SyncObj, NULL );
	memset( &m_PeerSockAddr, 0, sizeof(m_PeerSockAddr) );
	m_nKeepAliveCounter = 0;
	memset( m_abyXorData, 0, sizeof(m_abyXorData) );
	m_pConnectionHelper = NULL;
	m_byTimeToSendNopCmdToPeer = 0;

	m_pSendTask = NULL;
	m_pReceiveTask = NULL;
	m_pDelayNotifyDisconnectTask = NULL;

	pthread_mutex_init( &m_SyncObj_SendTask, NULL );
	pthread_mutex_init( &m_SyncObj_ReceiveTask, NULL );

	m_pEventObj = NULL;
	m_pThreadPoolObj = pSessionMgr->GetThreadPoolObject();
	// 2015.4.12 CYJ Add
	m_nSecondsForceToSendKeepAlive = DEFAULT_SECONDS_TO_SEND_NOP_CMD_TO_PEER;
	m_bSend2ndKeepAlivePacket = false;
#ifdef _DEBUG
	assert( m_pThreadPoolObj );
	int nObjCount = InterlockedIncrement( &g_nOnePeerObjectNumber );
	fprintf( stderr, "==== CMyRUDP_OnePeerObject::CMyRUDP_OnePeerObject( %p ) @ %d\n", this, nObjCount );
#endif // _DEBUG
	m_nSecondsShouldGetACK = 0;				// 2015.4.13 CYJ add, help to check the peer is on line or not

#ifdef __MYRUDP_USE_OPENSSL__
	memset( m_aDataEncryptionAESKey,0, sizeof(m_aDataEncryptionAESKey) );
#endif // #ifdef __MYRUDP_USE_OPENSSL__

	m_bHasGetDataFromPeer = false;	// 2015.5.7 CYJ Add
}

CMyRUDP_OnePeerObject::~CMyRUDP_OnePeerObject()
{
#ifdef _DEBUG
	int nObjCount = InterlockedDecrement( &g_nOnePeerObjectNumber );
	fprintf( stderr, "==== CMyRUDP_OnePeerObject::~CMyRUDP_OnePeerObject( %p ), === %d \n", this, nObjCount );
#endif // _DEBUG

	Invalidate();

	pthread_mutex_destroy( &m_SyncObj );
	pthread_mutex_destroy( &m_SyncObj_SendTask );
	pthread_mutex_destroy( &m_SyncObj_ReceiveTask );
}


//--------------------------------------------------------------
/** CYJ 2015-01-16
 *
 *	Initialize One peer object
 *
 * @param [in]	PeerAddr		peer object to be connected
 * @param [in]	nSessionID		sesstion ID associated with the session
 *
 * @return		0				succ
 *				other			error code
 */
int CMyRUDP_OnePeerObject::Initialize( const struct sockaddr_in & PeerAddr, int nSessionID )
{
	Invalidate();

	m_nState = PEER_OBJECT_STATE_IDLE;
	m_nSesstionID = nSessionID;
	m_nKeepAliveCounter = 0;
	m_bHasGetDataFromPeer = false;	// 2015.5.7 CYJ Add

#ifdef __MYRUDP_USE_OPENSSL__
	RAND_bytes( (uint8_t*)&m_dwSesstionTagID, sizeof(m_dwSesstionTagID) );
	m_dwSesstionTagID += s_dwSessionCounter;
#else
	m_dwSesstionTagID = ( time(NULL) ^ rand() ) + s_dwSessionCounter;
#endif
	s_dwSessionCounter ++;

	memcpy( &m_PeerSockAddr, &PeerAddr, sizeof(PeerAddr) );
	memset( m_abyXorData, 0, sizeof(m_abyXorData) );
	m_byMTUSizeIndex = MY_RUDP_MTU_SIZE_1392;

	// 2015.4.12 CYJ Add
	m_nSecondsForceToSendKeepAlive = DEFAULT_SECONDS_TO_SEND_NOP_CMD_TO_PEER;
	m_byTimeToSendNopCmdToPeer = 0;

	// 2015.4.13 CYJ add, help to check the peer is on line or not
	m_nSecondsShouldGetACK = 0;

	// FIXME
	m_pEventObj = m_pSessionMgr->CreateEventObject( this );

	m_pSendTask = new CMyRUDPDataSendTask( this );
	m_pReceiveTask = new CMyRUDPDataReceiveTask( this );
	m_pDelayNotifyDisconnectTask = new CMyRUDPNotifyDisconnectEventTask( this );
	if( NULL == m_pSendTask || NULL == m_pReceiveTask || NULL == m_pDelayNotifyDisconnectTask )
	{
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_ERROR, "OnePeerObject:%s, no memory ST:%p / RT:%p / NT: %p.\n",\
					  __FUNCTION__, m_pSendTask, m_pReceiveTask, m_pDelayNotifyDisconnectTask );
	#endif // _DEBUG
		return ENOMEM;
	}

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-01-16
 *
 *	Invalidate and free resource
 */
void CMyRUDP_OnePeerObject::Invalidate()
{
#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_ERROR, "CMyRUDP_OnePeerObject::Invalidate() called.\n" );
#endif // _DEBUG

	CMyRUDP_Packet_Sender::Invalidate();

	CMy_pthread_Mutex_Locker SyncObj_Receive( &m_SyncObj_ReceiveTask, false );
	CMy_pthread_Mutex_Locker SyncObj_Send( &m_SyncObj_SendTask );

	m_nSesstionID = -1;
	memset( &m_PeerSockAddr, 0, sizeof(m_PeerSockAddr) );

	if( m_pConnectionHelper )
	{
	#ifdef __MYRUDP_USE_OPENSSL__
		m_pConnectionHelper->Abort();
	#endif // __MYRUDP_USE_OPENSSL__
		m_pConnectionHelper->Release();
		m_pConnectionHelper = NULL;
	}

	m_nState = PEER_OBJECT_STATE_IDLE;
	m_byTimeToSendNopCmdToPeer = 0;

	if( m_pSendTask )
	{
		m_pThreadPoolObj->RemoveTask( m_pSendTask );
		delete m_pSendTask;
		m_pSendTask = NULL;
	}

	if( m_pReceiveTask )
	{
		m_pThreadPoolObj->RemoveTask( m_pReceiveTask );
		delete m_pReceiveTask;
		m_pReceiveTask = NULL;
	}

	// 2015.4.5 CYJ Add
	if( m_pDelayNotifyDisconnectTask )
	{
		m_pThreadPoolObj->RemoveTask( m_pDelayNotifyDisconnectTask );
		delete m_pDelayNotifyDisconnectTask;
		m_pDelayNotifyDisconnectTask = NULL;
	}

	if( m_pEventObj )
	{	// 2015.4.11 CYJ Add
		m_pEventObj->OnConnectionStateChanged( CMyRUDP_EventObj::MY_RUDP_CONNECTION_STATE_DISCONNECTING );
		m_pEventObj->OnConnectionStateChanged( CMyRUDP_EventObj::MY_RUDP_CONNECTION_STATE_DISCONNECTED );
		m_pEventObj->Release();
		m_pEventObj = NULL;
	}

	// 2015.4.2 CYJ Add
	m_listDataHasBeenSendSucc.clear();
	m_listDataHasBeenSendFailed.clear();
}

//--------------------------------------------------------------
/** CYJ 2015-01-21
 *
 *	Increase Reference
 *
 * @return	Increase or Decreased reference count
 *
 */
int CMyRUDP_OnePeerObject::AddRef()
{
	return InterlockedIncrement( &m_nRefCount );
}

//--------------------------------------------------------------
/** CYJ 2015-01-23
 *
 *	Decrease reference, if reference count = 0, delete itself
 *
 * @return	decreated reference count
 *
 */
int CMyRUDP_OnePeerObject::Release()
{
	int nRetVal = InterlockedDecrement( &m_nRefCount );
#ifdef _DEBUG
	assert( nRetVal >= 0 );
#endif // _DEBUG
	if( nRetVal )
		return nRetVal;

	Invalidate();
	delete this;

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-01-16
 *
 *	On packet received
 *
 * @param [in]	pBuf			data buffer
 * @param [in]	nLen			data size
 * @param [in]	PeerAddr		peer socket addr
 */
void CMyRUDP_OnePeerObject::OnDataPacket( const unsigned char * pBuf, int nLen, const struct sockaddr_in & PeerAddr )
{
#ifdef _DEBUG
//	MyRUDP_fprintf( "--- %s ( %5d bytes, %02x -- %02x %02x, %02x/%02x, %02x%02x )\n", __FUNCTION__, nLen, pBuf[2], pBuf[11], pBuf[12], pBuf[14], pBuf[13], pBuf[17], pBuf[18] );

	if( m_PeerSockAddr.sin_port )
	{
		if( m_PeerSockAddr.sin_port != PeerAddr.sin_port || m_PeerSockAddr.sin_addr.s_addr != PeerAddr.sin_addr.s_addr )
			MyRUDP_fprintf( "IP change from 0x%08x:%d => 0x%08x:%d\n", \
							m_PeerSockAddr.sin_addr.s_addr, m_PeerSockAddr.sin_port,\
							PeerAddr.sin_addr.s_addr, PeerAddr.sin_port );
	}

#endif // _DEBUG

	memcpy( &m_PeerSockAddr, &PeerAddr, sizeof(PeerAddr) );

	int nOldKeepAliveCount = m_nKeepAliveCounter;
	m_nKeepAliveCounter = 0;
	m_nSecondsShouldGetACK = 0;		// 2015.4.13 CYJ Add, should get data from peer after send out data
	m_bHasGetDataFromPeer = true;	// 2015.5.7 CYJ Add

	OnDataReceivedFromPeer();		// 2015.5.22 CYJ Add

	CMyRUDP_HeaderReader Reader( pBuf );
	switch( Reader.GetCommand() )
	{	// process
	case MYRUDP_CMD_RSP:			// should not be here
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_WARNING, "Should not be here, this is a server, should not get MYRUDP_CMD_RSP.\n" );
	#endif // _DEBUG
		return;

	case MYRUDP_CMD_NOP:			// keep alive
	#ifdef _DEBUG
		//MyRUDP_fprintf( "[%d] Keep Alive\n", m_nSesstionID );
	#endif // _DEBUG
		OnKeepAliveCmdRecieved( nOldKeepAliveCount );
		return;

	case MYRUDP_CMD_REQ:			// request connecting
		// force to reconnect, since the client will disconnected, but the server still keep connect
		OnConnectDataPacket( pBuf, nLen );
		return;

	case MYRUDP_CMD_SYN:
		OnSyncDataPacket( pBuf, nLen );
		return;

	case MYRUDP_CMD_DATA:
		OnDATACmdDataPacket( pBuf, nLen );
		return;

	case MYRUDP_CMD_ACK:
		OnACKCmdDataPacket( pBuf, nLen );
		return;

	case MYRUDP_CMD_RESEND:
		OnResendCmdDataPacket( pBuf, nLen );
		return;

	case MYRUDP_CMD_SKIP:		// 2015.4.2 CYJ Add
		OnSkipCmdFromServer( pBuf, nLen );
		return;

	case MYRUDP_CMD_CLOSE:
		OnCloseCmdDataPacket( pBuf, nLen );
		return;
	}
}

//--------------------------------------------------------------
/** CYJ 2015-05-10
 *
 *	Get Keep alive data from peer
 *
 * @param [in]	nSecondsFromLastReceiveData			second from last packet received from the peer
 *
 */
void CMyRUDP_OnePeerObject::OnKeepAliveCmdRecieved( int nSecondsFromLastReceiveData )
{
	SendKeepAliveCmdToPeer();
	ForceSendoutPacket( nSecondsFromLastReceiveData < 10 ? 1 : 10 );

	if( m_pEventObj )
		m_pEventObj->OnKeepAlivePacketReceived( nSecondsFromLastReceiveData );
}

//--------------------------------------------------------------
/** CYJ 2015-02-23
 *
 *	On resend command
 *
 * @param [in]	pBuf			data buffer
 * @param [in]	nLen			data length
 */
void CMyRUDP_OnePeerObject::OnResendCmdDataPacket( const unsigned char * pBuf, int nLen )
{
	uint16_t wPeerRevTailSeqNo = CMyRUDP_HeaderReader::GetSeqNo( pBuf );
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

	CMyRUDP_Packet_Sender::OnResend( awSeqNo, nCount, wPeerRcvHeadSeqNo, wPeerRevTailSeqNo );
}

//--------------------------------------------------------------
/** CYJ 2015-02-23
 *
 *	On resend command
 *
 * @param [in]	pBuf			data buffer
 * @param [in]	nLen			data length
 */
void CMyRUDP_OnePeerObject::OnCloseCmdDataPacket( const unsigned char * pBuf, int nLen )
{
	SetStateToDisconnected();
}

//--------------------------------------------------------------
/** CYJ 2015-02-12
 *
 *	Send Keep Alive command to peer
 */
void CMyRUDP_OnePeerObject::SendKeepAliveCmdToPeer()
{
#ifdef _DEBUG
//	MyRUDP_fprintf( "------------------------ %s called.\n", __FUNCTION__ );
#endif // _DEBUG

	uint8_t	abyCmdData[ MY_RUDP_HEADER_LEN ];

	// SeqNo must be 0, to avoid encrypt sessionTagID
	CMyRUDP_HeaderBuilder HeaderBuilder( abyCmdData, this, true );
	HeaderBuilder.SetCommand( MYRUDP_CMD_NOP );
	HeaderBuilder.SetSeqNo( 0 );
	HeaderBuilder.Commit();

	SendRawDataToPeer( abyCmdData, MY_RUDP_HEADER_LEN );
}

//--------------------------------------------------------------
/** CYJ 2015-02-12
 *
 *	On ACK command Data packet
 *
 * @param [in]
 * @param [in]
 * @param [in]
 *
 * @return
 *
 */
void CMyRUDP_OnePeerObject::OnACKCmdDataPacket( const unsigned char * pBuf, int nLen )
{
	uint16_t wPeerRevTailSeqNo = CMyRUDP_HeaderReader::GetSeqNo( pBuf );
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

		CMyRUDP_Packet_Sender::OnACK( awACKSeqNo, nAckCount, wPeerRcvHeadSeqNo, wPeerRevTailSeqNo );
	}
	else if( MY_RUDP_ACK_MODE_RANGE == nMode )
	{	// range mode
		int nAckCount = nLen / 4;
		for(int i=0; i<nAckCount; i++ )
		{
			uint16_t wHeadSeqNo = pBuf[0] * 0x100 + pBuf[1];
			uint16_t wTailSeqNo = pBuf[2] * 0x100 + pBuf[3];
			pBuf += 4;
			CMyRUDP_Packet_Sender::OnACK( wHeadSeqNo, wTailSeqNo, wPeerRcvHeadSeqNo, wPeerRevTailSeqNo );
		}
	}
	else
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "[%d] %s, unknown ACK value: 0x%02x.\n", m_nSesstionID, __FUNCTION__, nMode );
		assert( false );
	#endif // _DEBUG
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-11
 *
 *	on DATA cmd data packet
 *
 * @param [in]	pBuf				data buffer
 * @param [in]	nLen				data length
 */
void CMyRUDP_OnePeerObject::OnDATACmdDataPacket( const unsigned char * pBuf, int nLen )
{
	// do data decryption

	uint16_t awACKSeqNo[ MYRUDP_MAX_ACK_SEQNO_COUNT ];
	int nACKCount = sizeof(awACKSeqNo) / sizeof( awACKSeqNo[0] );

	if( false == m_DataPacketMgr.IsValid() )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "[%d] m_DataPacketMgr is not valid.\n", m_nSesstionID );
	#endif // _DEBUG
		return;						// not ready
	}

	int nRetVal = m_DataPacketMgr.OnDataPacketReady( pBuf, nLen, awACKSeqNo, nACKCount );
	if( nRetVal < 0 )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "[%d] OnDataPacketReady() = %d.\n", m_nSesstionID, nRetVal );
	#endif // _DEBUG

		if( -2 == nRetVal )
		{
			uint16_t wSeqNo = CMyRUDP_HeaderReader::GetSeqNo( pBuf );
			RequestPeerToResendPacket( (wSeqNo&0xFF) == (m_DataPacketMgr.GetTailSeqNo()&0xFF) );

			if( m_DataPacketMgr.HasDataToBeRead() && m_pReceiveTask && m_pEventObj )
			{
			#ifdef _DEBUG
				MyRUDP_fprintf( "********** Still has data to read, add to Task\n" );
			#endif // _DEBUG
				m_pThreadPoolObj->AddTask( m_pReceiveTask );
			}
		}
		return;		// error occured, discard the data packet
	}

	// send back ACK
#ifdef _DEBUG
	if( nACKCount < 1 || nACKCount > MYRUDP_MAX_ACK_SEQNO_COUNT )
		MyRUDP_fprintf( "error ACK count = %d\n", nACKCount );
	assert( nACKCount >= 1 && nACKCount <= MYRUDP_MAX_ACK_SEQNO_COUNT );
#endif // _DEBUG

	SendACKToPeer( awACKSeqNo, nACKCount );

	// one full data packet is received, should add to task and schedule
	if( nRetVal > 0 )
	{
	#ifdef _DEBUG
//		MyRUDP_fprintf( "Get one completed packet => %d \n", nRetVal );
	#endif // _DEBUG

		if( NULL == m_pReceiveTask || NULL == m_pEventObj )
		{
			while( m_DataPacketMgr.SkipData() );			// discard data
		}
		else
			m_pThreadPoolObj->AddTask( m_pReceiveTask );
	}

	RequestPeerToResendPacket( false );
}

//--------------------------------------------------------------
/** CYJ 2015-02-23
 *
 *	Request the peer to resend packet
 */
void CMyRUDP_OnePeerObject::RequestPeerToResendPacket( bool bForce )
{
	// check need to resend or not
	uint16_t awSeqNo[ MYRUDP_MAX_ACK_SEQNO_COUNT ];
	int nRetCount = m_DataPacketMgr.GetResendSeqNo( awSeqNo, MYRUDP_MAX_ACK_SEQNO_COUNT, bForce );
	if( nRetCount <= 0 )
		return;

	#define RESEND_MAX_BUF_LEN ( MY_RUDP_HEADER_LEN + 2*MYRUDP_MAX_ACK_SEQNO_COUNT )

	uint8_t abyCmdBuf[ RESEND_MAX_BUF_LEN ];

	uint8_t * pBuf = abyCmdBuf;

	uint16_t wExpectedSeqNo = m_DataPacketMgr.GetExpectedSeqNo();
	CMyRUDP_HeaderBuilder HeaderBuilder( pBuf, this, true );
	HeaderBuilder.SetCommand( MYRUDP_CMD_RESEND );
	HeaderBuilder.SetSeqNo( m_DataPacketMgr.GetTailSeqNo() );
	pBuf[ 13 ] = (uint8_t)( wExpectedSeqNo >> 8 );
	pBuf[ 14 ] = (uint8_t)( wExpectedSeqNo );
	HeaderBuilder.Commit();

	uint8_t * pTmpBuf = pBuf + MY_RUDP_HEADER_LEN;
	// 2015.4.2 CYJ modify, add Mode field
	pTmpBuf[0] = MY_RUDP_ACK_MODE_ARRAY;
	pTmpBuf ++;
	for(int i=0; i<nRetCount; i++ )
	{
		uint16_t wSeqNo = awSeqNo[i];
	#ifdef _DEBUG
		MyRUDP_fprintf( "[%d] ======= Request resend: 0x%04x.\n", m_nSesstionID, wSeqNo );
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
void CMyRUDP_OnePeerObject::SendACKToPeer( const uint16_t awACKSeqNo[], int nCount )
{
	#define ACK_MAX_BUF_LEN ( MY_RUDP_HEADER_LEN + MYRUDP_MAX_ACK_SEQNO_COUNT*2 + 16 )	// keep 16 bytes

	uint8_t abyCmdBuf[ ACK_MAX_BUF_LEN ];

	uint8_t * pBuf = abyCmdBuf;

	uint16_t wExpectedSeqNo = m_DataPacketMgr.GetExpectedSeqNo();

	CMyRUDP_HeaderBuilder HeaderBuilder( pBuf, this, true );
	HeaderBuilder.SetCommand( MYRUDP_CMD_ACK );
	HeaderBuilder.SetSeqNo( m_DataPacketMgr.GetTailSeqNo() );
	pBuf[ 13 ] = (uint8_t)( wExpectedSeqNo >> 8 );
	pBuf[ 14 ] = (uint8_t)( wExpectedSeqNo );
	HeaderBuilder.Commit();

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
void CMyRUDP_OnePeerObject::SendACKToPeer( uint16_t wHeadSeqNo, uint16_t wTailSeqNo )
{
	uint8_t abyCmdBuf[ ACK_MAX_BUF_LEN ];

	uint8_t * pBuf = abyCmdBuf;

	uint16_t wExpectedSeqNo = m_DataPacketMgr.GetExpectedSeqNo();

	CMyRUDP_HeaderBuilder HeaderBuilder( pBuf, this, true );
	HeaderBuilder.SetCommand( MYRUDP_CMD_ACK );
	HeaderBuilder.SetSeqNo( m_DataPacketMgr.GetTailSeqNo() );
	pBuf[ 13 ] = (uint8_t)( wExpectedSeqNo >> 8 );
	pBuf[ 14 ] = (uint8_t)( wExpectedSeqNo );
	HeaderBuilder.Commit();

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
/** CYJ 2015-01-29
 *
 *	On Get Sync data
 *
 * @param [in]	pBuf			sync data buffer
 * @param [in]	nLen			data length
 */
void CMyRUDP_OnePeerObject::OnSyncDataPacket( const unsigned char * pBuf, int nLen )
{
#ifdef _DEBUG
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "[%d] Get one SYN data %d bytes.\n", m_nSesstionID, nLen );
#endif // _DEBUG

	if( false == CMyRUDP_Connection_Svr_Helper::IsSynCommandData( pBuf, nLen ) )
		return;		// not a sync data, discard, and continue waiting

	// 2015.5.4 CYJ Add SyncObj
	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj );

	if( NULL == m_pConnectionHelper )
	{	// has been connect succ
	#ifdef _DEBUG
		assert( PEER_OBJECT_STATE_CONNECTED == m_nState );
	#endif // _DEBUG

		CMyRUDP_Connection_Svr_Helper::BuildAndSendSyn2( this );

		return;		// no connection helper
	}

	if( m_pConnectionHelper->OnSynData( pBuf, nLen ) )
		return;

#ifdef _DEBUG
//	MyRUDP_fprintf( "[%d] Get SYN data, connecting succ.\n", m_nSesstionID );
#endif // _DEBUG

	// connected succ
	m_nState = PEER_OBJECT_STATE_CONNECTED;
#ifdef __MYRUDP_USE_OPENSSL__
	m_pConnectionHelper->Abort();
#endif // __MYRUDP_USE_OPENSSL__
	m_pConnectionHelper->Release();
	m_pConnectionHelper = NULL;

	if( m_pEventObj )
		m_pEventObj->OnConnectionStateChanged( CMyRUDP_EventObj::MY_RUDP_CONNECTION_STATE_CONNECTED );
}

//--------------------------------------------------------------
/** CYJ 2015-01-23
 *
 *	On connecting request data packet
 *
 * @param [in]	pBuf			data buffer
 * @param [in]	nLen			data size
 *
 * @return
 *
 */
int CMyRUDP_OnePeerObject::OnConnectDataPacket( const unsigned char * pBuf, int nLen )
{
#ifdef _DEBUG
	MyRUDP_fprintf( "[%d] Get one REQ data %d bytes.\n", m_nSesstionID, nLen );
#endif // _DEBUG

	// Header(18) + UUID(16) + XorData(32) + MTUIndex(1) + PubKeyLen(2) + CRC32(4)
    if( false == CMyRUDP_Connection_Svr_Helper::IsReqCommandData(pBuf,nLen) )
		return 1;

	// 2015.5.4 CYJ Add SyncObj
	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj );

	const uint8_t * pbyReqData = pBuf + MY_RUDP_HEADER_LEN;
	memcpy( m_abyXorData, pbyReqData+16, 32 );		// copy Xor Data
	m_byMTUSizeIndex = pbyReqData[ 16+32 ];			// MTU Index

#ifdef _DEBUG
	MyRUDP_fprintf( "[%d] MTUSize=%d, m_dwSesstionTagID=0x%08x\n", m_nSesstionID, m_byMTUSizeIndex, m_dwSesstionTagID );
#endif // _DEBUG

	int nRetVal = CMyRUDP_Packet_Sender::Initialize( m_byMTUSizeIndex, m_nSesstionID+1, m_dwSesstionTagID, m_abyXorData );
	if( nRetVal )
	{
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_WARNING, "Failed to CMyRUDP_Packet_Sender::Initialize, no memory.\n" );
	#endif // _DEBUG
		return 2;
	}

	// initialize data packet
	if( m_DataPacketMgr.Initialize( m_byMTUSizeIndex ) )
	{
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_WARNING, "Failed to initialize data packet manager, no memory.\n" );
	#endif // _DEBUG
		return 3;
	}

	if( NULL == m_pConnectionHelper )
		m_pConnectionHelper = new CMyRUDP_Connection_Svr_Helper( this );
	if( NULL == m_pConnectionHelper )
	{
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_WARNING, "Failed to allocate Connection helper.\n" );
	#endif // _DEBUG
		return 4;
	}

	if( m_pConnectionHelper->OnReqData( pBuf, nLen ) )
	{
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_WARNING, "OnReqData failed.\n" );
	#endif // _DEBUG
		delete m_pConnectionHelper;
		m_pConnectionHelper = NULL;
		return 5;
	}

	// FIXME, to deal with Security transfer

#ifdef _DEBUG
	MyRUDP_fprintf( "[%d] Change state to CONNECTING, waiting for RSP.\n", m_nSesstionID );
#endif // _DEBUG
	m_nState = PEER_OBJECT_STATE_CONNECTING;		// start connecting

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-01-23
 *
 *	Check keep alive timer
 *
 * @return	true			alive
 *			false			not alive
 */
bool CMyRUDP_OnePeerObject::CheckKeepAliveTimer()
{
	if( m_nKeepAliveCounter < KEEP_ALIVE_COUNTER_MAX_VALUE )
	{
		m_nKeepAliveCounter ++;
		return true;
	}

#ifdef _DEBUG
	assert( m_pSessionMgr );
	MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "[%d] Keep alive counter = 0, disconnect it.\n", m_nSesstionID );
#endif // _DEBUG

	SetStateToDisconnected();

	return false;
}

//--------------------------------------------------------------
/** CYJ 2015-01-16
 *
 *	On timer per 1000 ms
 *
 * @param [in]	tNow			current time tick in UTC
 *
 */
void CMyRUDP_OnePeerObject::OnTimer( time_t tNow )
{
	// 2015.5.4 CYJ Add SyncObj
	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj );

	if( false == CheckKeepAliveTimer() )
		return;			// not alive, disconnect from the session

	if( m_pConnectionHelper )
	{	// wait syn data
		if( m_pConnectionHelper->OnWaitSynTimer( tNow ) )
			return;
		// wait syn data timeout
	#ifdef _DEBUG
		MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "[%d] Wait Sync data timeout.\n", m_nSesstionID );
	#endif //_DEBUG
		SetStateToDisconnected();
		return;
	}

	SyncObj.Unlock();

	// 2015.4.3 CYJ Add, force to send Keep Alive to peer
	// 2015.4.12 CYJ Modify using m_nSecondsForceToSendKeepAlive instead of const define value
	// 2015.4.23 CYJ Add, if m_nSecondsForceToSendKeepAlive = 0, then not force to send KeepAlive Packet to peer
	if( m_nSecondsForceToSendKeepAlive )
	{
		m_byTimeToSendNopCmdToPeer ++;
		if( m_byTimeToSendNopCmdToPeer >= m_nSecondsForceToSendKeepAlive )
		{
			m_bSend2ndKeepAlivePacket = true;
			SendKeepAliveCmdToPeer();	// m_byTimeToSendNopCmdToPeer will be reset in SendRawDataToPeer
		}
		else if( m_bSend2ndKeepAlivePacket )
		{	// 2015.4.12 CYJ add, send 2nd Keep Alive packet to avoid previous Keep alive packet lost
			m_bSend2ndKeepAlivePacket = false;
			SendKeepAliveCmdToPeer();
		}
	}

	if( m_nSecondsShouldGetACK )
	{
		m_nSecondsShouldGetACK --;
		if( 0 == m_nSecondsShouldGetACK )
		{
		#ifdef _DEBUG
			MyLog_Printf( MY_LOG_LEVEL_DEBUG_MSG, "not get ACK from peer after %d seconds, set offline.\n", SECONS_SHOULD_GET_ACK_FROM_PEER );
		#endif //_DEBUG
		// TO BE DONE, set state to offline ?
		}
	}

	CMyRUDP_Packet_Sender::OnTimer( tNow );

#if 0
	if( m_bHasGetDataFromPeer )
	{

	}
#endif // 0

#ifdef _DEBUG
//	CMyRUDP_Packet_Sender::DebugDump();
//	m_DataPacketMgr.DebugDump();
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-02-21
 *
 *	Set state to be disconnected
 */
void CMyRUDP_OnePeerObject::SetStateToDisconnected()
{
	if( m_pConnectionHelper )
	{
	#ifdef __MYRUDP_USE_OPENSSL__
		m_pConnectionHelper->Abort();
	#endif // __MYRUDP_USE_OPENSSL__
		m_pConnectionHelper->Release();
		m_pConnectionHelper = NULL;
	}

	m_nState = PEER_OBJECT_STATE_DISCONNECTED;			// start connecting
	if( m_pEventObj )
		m_pEventObj->OnConnectionStateChanged( CMyRUDP_EventObj::MY_RUDP_CONNECTION_STATE_DISCONNECTING );

	// 2015.4.5 CYJ Add
	m_pThreadPoolObj->AddTask( m_pDelayNotifyDisconnectTask, true );

	m_pSessionMgr->DisconnectSession( m_nSesstionID );
}

//--------------------------------------------------------------
/** CYJ 2015-01-28
 *
 *	Send raw data to peer
 *
 * @param [in]	pBuf			data buffer to be sent
 * @param [in]	nLen			data length
 *
 * @return		0				succ
 *				other			error code
 */
int CMyRUDP_OnePeerObject::SendRawDataToPeer( const uint8_t * pBuf, int nLen )
{
	m_byTimeToSendNopCmdToPeer = 0;
	return m_pSessionMgr->SendTo( pBuf, nLen, (sockaddr*)&m_PeerSockAddr, sizeof(m_PeerSockAddr) );
}

//--------------------------------------------------------------
/** CYJ 2015-01-17
 *
 *	Check session
 *
 * @param [in]	pBuf			command data buffer
 * @param [in]	dwSessionTagID	session tag ID
 *
 * @return		true			session is OK
 *				false
 */
bool CMyRUDP_OnePeerObject::CheckSessionValid( const unsigned char *pBuf, uint32_t dwSessionTagID )
{
#ifdef _DEBUG
	uint32_t dwOrgSessionTagID = dwSessionTagID;
#endif // _DEBUG

	if( m_nState < PEER_OBJECT_STATE_CONNECTING )
	{
	#ifdef _DEBUG
	//	MyRUDP_fprintf( "CheckSessionValid, m_nState( %d ) < %d\n", m_nState, PEER_OBJECT_STATE_CONNECTING );
	#endif // _DEBUG
		return false;
	}

	CMyRUDP_HeaderReader Reader( pBuf );
	uint32_t dwSeqNo = Reader.GetSeqNo();
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
	if( dwSessionTagID != m_dwSesstionTagID )
	{
		MyRUDP_fprintf( "CheckSessionValid %08x != %08x, SeqNo: 0x%04x, OrgSessionID=0x%08x\n", dwSessionTagID, m_dwSesstionTagID, dwSeqNo, dwOrgSessionTagID );
		DbgDumpData( "InputData: ", pBuf, 18 );
		DbgDumpData( "XorData: ", m_abyXorData, 32 );
	}
#endif // _DEBUG

	return ( dwSessionTagID == m_dwSesstionTagID );
}

//--------------------------------------------------------------
/** CYJ 2015-02-15
 *
 *	Notify one packet has been send succ
 *
 * @param [in]	pUserData			user data
 */
void CMyRUDP_OnePeerObject::OnPacketHasBeenSendSucc( void * pUserData )
{
	// 2015.4.2 CYJ Add, if pUserData == NULL, not send event
	if( NULL == m_pSendTask || NULL == m_pEventObj || NULL == pUserData )
		return;

	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_SendTask );
	m_listDataHasBeenSendSucc.push_back( pUserData );
	SyncObj.Unlock();
	m_pThreadPoolObj->AddTask( m_pSendTask );
}

//--------------------------------------------------------------
/** CYJ 2015-04-02
 *
 *	Notify one packet has been send succ
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
void CMyRUDP_OnePeerObject::OnPacketSendFailed( uint16_t wHeadSeqNo, uint16_t wTailSeqNo, void * pUserData )
{
	#define SKIP_MAX_BUF_LEN ( MY_RUDP_HEADER_LEN + 4 + 16 )	// keep 16 bytes

	uint8_t abyCmdBuf[ SKIP_MAX_BUF_LEN ];

	uint8_t * pBuf = abyCmdBuf;

	CMyRUDP_HeaderBuilder HeaderBuilder( pBuf, this, true );
	HeaderBuilder.SetCommand( MYRUDP_CMD_SKIP );
	HeaderBuilder.Commit();

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
	if( m_pEventObj && pUserData && m_pSendTask )
	{
		CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_SendTask );
		m_listDataHasBeenSendFailed.push_back( pUserData );
		SyncObj.Unlock();
		m_pThreadPoolObj->AddTask( m_pSendTask );
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-21
 *
 *	On data has been send succ task
 */
void CMyRUDP_OnePeerObject::ThreadTask_OnDataSend()
{
	CMyRUDP_OnePeerObject_AutoLock AutoLock( this );

	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_SendTask );

	if( NULL == m_pEventObj )
		return;

	while( false == m_listDataHasBeenSendSucc.empty() )
	{
		void * pUserData = m_listDataHasBeenSendSucc.front();
		m_listDataHasBeenSendSucc.pop_front();
		if( pUserData )
			m_pEventObj->OnDataSentSucc( pUserData );
	}

	// 2015.4.2 CYJ Add
	while( false == m_listDataHasBeenSendFailed.empty() )
	{
		void * pUserData = m_listDataHasBeenSendFailed.front();
		m_listDataHasBeenSendFailed.pop_front();
		if( pUserData )
			m_pEventObj->OnSendDataFailed( pUserData );
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-21
 *
 *	On data has been recieved task
 */
void CMyRUDP_OnePeerObject::ThreadTask_OnDataReceived()
{
	CMyRUDP_OnePeerObject_AutoLock AutoLock( this );

	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_ReceiveTask, false );
	if( false == SyncObj.TryLock() )
	{	// try later
		if( m_pReceiveTask )
			m_pThreadPoolObj->AddDelayTask( m_pReceiveTask, 50 );
		return;				// try lock failed, some other thread locked the sync object
	}

	if( NULL == m_pEventObj )
		return;

	while( 1 )
	{
		if( false == m_DataPacketMgr.IsValid() )
			break;
		int nNextDataLen = m_DataPacketMgr.GetNextPacketDataLen();
		if( nNextDataLen <= 0 )
			break;			// no data
		nNextDataLen += 64;
		uint8_t * pBuf = m_pEventObj->OnDataReceived_Allocate( nNextDataLen );
		if( NULL == pBuf )
			break;		// no memory
		int nRetVal = m_DataPacketMgr.CopyData( pBuf, nNextDataLen );
		if( 0 == nRetVal )
		{				// no data
			m_pEventObj->OnDataReceived_Free( pBuf );
			return;
		}
		if( nRetVal < 0 )
		{
			m_pEventObj->OnDataReceived_Free( pBuf );
			if( false == m_DataPacketMgr.SkipData() )
				return;		// no more data
		}
		else
			m_pEventObj->OnDataReceived_Commit( pBuf, nRetVal, nNextDataLen );
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-26
 *
 *	Register Fast timer callback function
 */
void CMyRUDP_OnePeerObject::RegisterFastTimer()
{
#ifdef _DEBUG
	assert( m_pSessionMgr );
	assert( m_nSesstionID >= 0 );
#endif // _DEBUG

	m_pSessionMgr->AddFastTimer( m_nSesstionID );
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
 */
int CMyRUDP_OnePeerObject::Send( const uint8_t * pBuf, int nLen, void * pUserData, int nTimeOut )
{
	return CMyRUDP_Packet_Sender::Send( pBuf, nLen, pUserData, nTimeOut );
}

//--------------------------------------------------------------
/** CYJ 2015-03-12
 *
 *	Get Associated Server Drv
 */
CMyRUDP_ServerDrv * CMyRUDP_OnePeerObject::GetServerDrv()
{
	return static_cast<CMyRUDP_ServerDrv *>(m_pSessionMgr);
}

//--------------------------------------------------------------
/** CYJ 2015-04-02
 *
 *	On Skip command from server
 *
 * @param [in]	pBuf				data buffer
 * @param [in]	nLen				data length
*/
void CMyRUDP_OnePeerObject::OnSkipCmdFromServer( const unsigned char *pBuf, int nLen )
{
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

		m_DataPacketMgr.OnSkipData( wHeadSeqNo, wTailSeqNo );

		// build and send fake ACK to peer
		SendACKToPeer( wHeadSeqNo, wTailSeqNo );
	}
}

//--------------------------------------------------------------
/** CYJ 2015-04-12
 *
 *	set force send out keep alive interval seconds
 *
 * @param [in]	nValue				interval in seconds
 *
 * @return		old value
 */
int CMyRUDP_OnePeerObject::SetForceSendingKeepAliveInterval( int nValue )
{
	if( nValue )
	{	// 2015.4.26 CYJ Add, if 0 == nValue, then not send out UDP keep alive any more, using tcp keep alive instead
		if( nValue < 15 )
			nValue = 15;
		else if( nValue > 30*60 )
			nValue = 30*60;
	}

	int nRetVal = m_nSecondsForceToSendKeepAlive;
	m_nSecondsForceToSendKeepAlive = nValue;
	return nRetVal;
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
int CMyRUDP_OnePeerObject::SendDataPacketToPeer( const uint8_t * pBuf, int nLen )
{
	m_nSecondsShouldGetACK = SECONS_SHOULD_GET_ACK_FROM_PEER;		// assume should get ACK from peer in 30 seconds
	return SendRawDataToPeer( pBuf, nLen );
}

#ifdef __MYRUDP_USE_OPENSSL__
//--------------------------------------------------------------
/** CYJ 2015-04-19
 *
 *	Set data encryption AES key
 *
 * @param [in]	pBuf				AES Key
 * @param [in]	nLen				AES Key Length
 */
void CMyRUDP_OnePeerObject::SetDataEncryptionAESKey( const uint8_t * pBuf, int nLen )
{
#ifdef _DEBUG
//	DbgDumpData( __FUNCTION__, pBuf, nLen );
#endif //_DEBUG

	if( nLen >= (int)sizeof(m_aDataEncryptionAESKey) )
		memcpy( m_aDataEncryptionAESKey, pBuf, sizeof(m_aDataEncryptionAESKey) );
	else
	{
		memcpy( m_aDataEncryptionAESKey, pBuf, nLen );
		memcpy( m_aDataEncryptionAESKey+nLen, m_abyXorData, sizeof(m_aDataEncryptionAESKey)-nLen );
	}

	m_DataPacketMgr.SetDataEncryptionAESKey( m_aDataEncryptionAESKey );
	CMyRUDP_Packet_Sender::SetDataEncryptionAESKey( m_aDataEncryptionAESKey );
}
#endif// #ifdef __MYRUDP_USE_OPENSSL__

