/****************************************************************************
 *
 *	My RUDP sender packet manger
 *
 *	Chen Yongjian @ zhoi
 *	2015.2.11 @ Xi'an
 *
 ***************************************************************************/

#ifndef __USE_XOPEN2K
	#define __USE_XOPEN2K
#endif // __USE_XOPEN2K

#include <stdafx.h>
#include <errno.h>
#include <assert.h>

#include <my_pthread_mutex_help.h>
#include <crc.h>

#ifdef __MYRUDP_USE_OPENSSL__
	#include <openssl/aes.h>
#endif // __MYRUDP_USE_OPENSSL__

#include "myrudp_packet_sender.h"
#include "myrudp_dbgprint.h"


CMyRUDP_Packet_Sender::CMyRUDP_Packet_Sender()
{
	m_pDataBuf = NULL;								// data buffer
	memset(m_aPackets, 0, sizeof(m_aPackets) );
	m_nMTUSize = 0;
	m_byMTUIndex_Shifted = 0;
	m_wHeadSeqNo = 0;
	m_wTailSeqNo = 0;
	m_wPeerRcvTailSeqNo = 0;
	m_wPeerRcvHeadSeqNo = 0;
	m_nSamePeerHeaderSeqNoCount = 0;
	m_nSamePeerHeaderSeqNoGateLevel = 4;
	m_nSecondsNotGetDataFromPeer = 0;				// 2015.5.23 CYJ Add

	m_dwSessionID = 0;
	m_pXorData = NULL;
	m_nSameTailSeqNoCount = 0;

	// receive data buffer sync objects
	pthread_mutex_init( &m_SyncObj_DataBufferAvailable, NULL);
	// 2015.4.1 CYJ porting to Android
#if defined(__USE_XOPEN2K) && !defined(ANDROID)
	pthread_condattr_t		CondAttr;
	pthread_condattr_init( &CondAttr );
	pthread_condattr_setclock( &CondAttr, CLOCK_MONOTONIC );
	pthread_cond_init( &m_EventDataBufAvailable, &CondAttr);
	pthread_condattr_destroy( &CondAttr );
#else
	pthread_cond_init( &m_EventDataBufAvailable, NULL);
#endif //defined(__USE_XOPEN2K) && !defined(ANDROID)

	// user data list
	pthread_mutex_init( &m_SyncObj_UserData, NULL);
	pthread_mutex_init( &m_SendDataSyncObj, NULL );

#ifdef __MYRUDP_USE_OPENSSL__
	m_pbyDataEncryptAESKey = NULL;				// 48 bytes
#endif //#ifdef __MYRUDP_USE_OPENSSL__
}

CMyRUDP_Packet_Sender::~CMyRUDP_Packet_Sender()
{
	Invalidate();

	// receive data buffer sync objects
	pthread_cond_destroy( &m_EventDataBufAvailable );
	pthread_mutex_destroy( &m_SyncObj_DataBufferAvailable );

	pthread_mutex_destroy( &m_SyncObj_UserData );
	pthread_mutex_destroy( &m_SendDataSyncObj );
}

//--------------------------------------------------------------
/** CYJ 2015-02-12
 *
 *	Initialize
 *
 * @param [in]	nMTUSize			MTU Size, if < 16 => MTU Size Index
 * @param [in]	dwSessionID			session ID
 * @param [in]	dwSessionTagID		session Tag ID
 * @param [in]	pXorData			XOR data
 *
 * @return		0					succ
 *				other				error code
 */
int CMyRUDP_Packet_Sender::Initialize( int nMTUSize, uint32_t dwSessionID, uint32_t dwSessionTagID, uint8_t * pXorData )
{
#ifdef _DEBUG
	MyRUDP_fprintf( "----------- %s called MTUSize=%d.\n", __FUNCTION__, nMTUSize );
#endif // _DEBUG

	Invalidate();

	m_dwSessionID = dwSessionID;
	m_pXorData = pXorData;
	m_dwSessionTagID = dwSessionTagID;
	m_nSameTailSeqNoCount = 0;

	if( nMTUSize < 16 )
		m_nMTUSize = MyRUDP_GetMTUSizeByIndex( nMTUSize );
	else
		m_nMTUSize = nMTUSize;
	m_byMTUIndex_Shifted = MyRUDP_GetMTUIndexBySize( m_nMTUSize );
	m_byMTUIndex_Shifted <<= 5;

	const int nOneItemBufSize = m_nMTUSize + MY_RUDP_HEADER_LEN + 4;
	int nBufSize = nOneItemBufSize * MYRUDP_WINDOW_SIZE;	// keep 4 bytes reserved buffer
	m_pDataBuf = new uint8_t[ nBufSize ];
	if( NULL == m_pDataBuf )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "CMyRUDP_Packet_Sender::%s failed to allocate memory( %d bytes).\n", __FUNCTION__, nBufSize );
	#endif //_DEBUG
		return -ENOMEM;
	}

	uint8_t * pBuf = m_pDataBuf;
	memset( m_aPackets, 0, sizeof(m_aPackets) );

	for(int i=0; i<MYRUDP_WINDOW_SIZE; i++ )
	{
		m_aPackets[i].m_pBuf = pBuf;
		pBuf += nOneItemBufSize;
	}

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-02-12
 *
 *	Invalidate and free resource
 */
void CMyRUDP_Packet_Sender::Invalidate()
{
#ifdef _DEBUG
	MyRUDP_fprintf( "----------- %s called.\n", __FUNCTION__ );
#endif // _DEBUG

	pthread_cond_broadcast( &m_EventDataBufAvailable ); // try waking up a bunch of threads that are still waiting

	CMy_pthread_Mutex_Locker SyncObjData( &m_SendDataSyncObj );

	if( m_pDataBuf )
	{
		delete m_pDataBuf;
		m_pDataBuf = NULL;
	}

	memset(m_aPackets, 0, sizeof(m_aPackets) );
	m_wHeadSeqNo = 0;
	m_wTailSeqNo = 0;
	m_nMTUSize = 0;
	m_wPeerRcvTailSeqNo = 0;
	m_wPeerRcvHeadSeqNo = 0;
	m_nSamePeerHeaderSeqNoCount = 0;
	m_nSamePeerHeaderSeqNoGateLevel = 4;

	m_dwSessionID = 0;
	m_pXorData = NULL;
	m_dwSessionTagID = 0;

	m_listUserData.clear();

	SyncObjData.Unlock();

	pthread_cond_broadcast( &m_EventDataBufAvailable ); // try waking up a bunch of threads that are still waiting

#ifdef __MYRUDP_USE_OPENSSL__
	m_pbyDataEncryptAESKey = NULL;				// 48 bytes
#endif //#ifdef __MYRUDP_USE_OPENSSL__
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
 *									-2		if no enough buffer, return without waiting, otherwise send with MUST SUCC
 *									other	time out seconds
 * @return		0					succ
 *				other				error code
 */
int CMyRUDP_Packet_Sender::Send( const uint8_t * pBuf, int nLen, void * pUserData, int nTimeOut )
{
	int nTimeOutBak = nTimeOut;

	if( 0 == m_nMTUSize || nLen <= 0 )
		return EIO;						// not initialize

	if( nLen > MYRUDP_MAX_DATA_LENGTH )
		return EFBIG;					// too long, out of range

#ifdef __MYRUDP_USE_OPENSSL__
	int nPacketNeed = ( nLen + 15 + m_nMTUSize - 1 ) / m_nMTUSize;
#else
	int nPacketNeed = ( nLen + m_nMTUSize - 1 ) / m_nMTUSize;
#endif // __MYRUDP_USE_OPENSSL__

	if( nPacketNeed >= (MYRUDP_WINDOW_SIZE/2) )
		return EFBIG;					// too long, out of range
	nPacketNeed += 2;

	int nBufAvailable = GetAvailableBufCount();

	// 2015.4.20 CYJ Add, if nTimeOut = -2, if has available buffer, then send must succ, otherwise, return directly
	if( -2 == nTimeOut )
	{
		if( nBufAvailable < nPacketNeed )
			return EAGAIN;
		nTimeOutBak = -1;
		nTimeOut = -1;
	}

	while( nBufAvailable < nPacketNeed )
	{	// not available data buffer
		if( false == IsValid() )
			return EIO;
		if( 0 == nTimeOut )
			return EAGAIN;				// no enough data buffer, would block

		// not enough data buffer, wait input event
		CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_DataBufferAvailable );
	#ifdef __USE_XOPEN2K
		if( nTimeOut > 0 )
		{
			struct timespec tv;
		#ifdef ANDROID
			// android not support CLOCK_MONOTONIC, so using CLOCK_REALTIME
			clock_gettime( CLOCK_REALTIME, &tv );
		#else
			// not android, using CLOCK_MONOTONIC
			clock_gettime( CLOCK_MONOTONIC, &tv );
		#endif //ANDROID

			tv.tv_sec += nTimeOut;

			pthread_cond_timedwait( &m_EventDataBufAvailable, &m_SyncObj_DataBufferAvailable, &tv );
			nTimeOut = 0;
		}
		else
			pthread_cond_wait( &m_EventDataBufAvailable, &m_SyncObj_DataBufferAvailable );
	#else
		if( nTimeOut > 0 )
			nTimeOut--;
		pthread_cond_wait( &m_EventDataBufAvailable, &m_SyncObj_DataBufferAvailable );
	#endif //#ifdef __USE_XOPEN2K

		SyncObj.Unlock();
		sched_yield();

		nBufAvailable = GetAvailableBufCount();
	}

	if( false == IsValid() )
		return EIO;

	CMy_pthread_Mutex_Locker SyncObj( &m_SendDataSyncObj );

	return BuildAndSendPackets( pBuf, nLen, pUserData, nTimeOutBak );
}

//--------------------------------------------------------------
/** CYJ 2015-02-12
 *
 *	Build and send packets
 *
 * @param [in]	pBuf			data buffer to be send
 * @param [in]	nLen			data length
 * @param [in]	pUserData		user data, used for notify data has been send succ.
 * @param [in]	nTimeOut		timeout value in seconds, if -1, must be send succ
 *
 * @return		0				succ
 *				other			error code
 */
int CMyRUDP_Packet_Sender::BuildAndSendPackets( const uint8_t * pBuf, int nLen, void * pUserData, int nTimeOut )
{
	int i;
	int nPacketCount = ( nLen + m_nMTUSize - 1 ) / m_nMTUSize;

	SEND_PACKET_USER_DATA AssociatedUserData;
	AssociatedUserData.m_nDataLen = nLen;
	AssociatedUserData.m_pUserData = pUserData;
	AssociatedUserData.m_wFirstSeqNo = m_wHeadSeqNo;
	AssociatedUserData.m_wLastSeqNo = m_wHeadSeqNo + nPacketCount - 1;
	AssociatedUserData.m_nPacketCount = nPacketCount;

	pthread_mutex_lock( &m_SyncObj_UserData );
	m_listUserData.push_back( AssociatedUserData );
	PSEND_PACKET_USER_DATA pItemRefPtr = &m_listUserData.back();
	pthread_mutex_unlock( &m_SyncObj_UserData );

	// 2015.4.2 CYJ Add, to support sending timeout
	uint8_t byMustSuccFlasgs = ( nTimeOut < 0 ) ? PACKET_SEND_FLAGS_MUST_SUCC : 0;
	uint8_t byResendTimeOutVal;
	if( nTimeOut > 0 )
	{
		if( nTimeOut > 255 )
			nTimeOut = 255;
		byResendTimeOutVal = (uint8_t)nTimeOut;
	}
	else
		byResendTimeOutVal = PACKET_RESEND_RETRY_TIMES;

	uint16_t wSeqNo = m_wHeadSeqNo;
	for( i=0; i<nPacketCount; i++ )
	{
		SEND_PACKET_ITEM & Item = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
	#ifdef _DEBUG
		assert( 0 == Item.m_nDataLen || 0 == Item.m_wFlags );
		assert( nLen > 0 );
		assert( (m_nMTUSize & 0xF) == 0 );
	#endif // _DEBUG

		int nPayloadLen = ( (i+1) == nPacketCount ) ? nLen : m_nMTUSize;

		Item.m_nDataLen = nPayloadLen;
		memcpy( Item.m_pBuf + MY_RUDP_HEADER_LEN, pBuf, nPayloadLen );
		pBuf += nPayloadLen;
		nLen -= nPayloadLen;

		Item.m_byMaxResendTimeout = byResendTimeOutVal;
		Item.m_byTimeOut = 2;
		Item.m_byResendCount = 1;
		Item.m_wFlags = byMustSuccFlasgs;				// 2015.4.2 CYJ Modify
		Item.m_wSeqNo = (uint16_t)( wSeqNo );
		Item.m_pUserDataItem = pItemRefPtr;

		wSeqNo ++;

	#ifdef _DEBUG
		assert( nPayloadLen <= m_nMTUSize );
	#endif // _DEBUG

		BuildOnePacket( nPacketCount, i, Item );
	}

	// must send packets after all sub-block are build, to avoid get ACK fast when m_wHeadSeqNo not updated
	wSeqNo = AssociatedUserData.m_wFirstSeqNo;
	for( i=0; i<nPacketCount; i++ )
	{
		SEND_PACKET_ITEM & Item = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
		m_wHeadSeqNo ++;
		if( IsSeqNoInRange( wSeqNo, m_wPeerRcvTailSeqNo + MYRUDP_WINDOW_SIZE - 1, m_wPeerRcvTailSeqNo ) )
			SendDataItemToPeer( Item );
		else
		{	// out of the peer receiving range, send delay
		#ifdef _DEBUG
		//	MyRUDP_fprintf( "SeqNo is out of range 0x%04x --> 0x%04x\n", Item.m_wSeqNo, m_wPeerRcvTailSeqNo );
		#endif // _DEBUG
			Item.m_wFlags |= PACKET_SEND_FLAGS_SEND_DELAY;
			RegisterFastTimer();
		}
		wSeqNo ++;
	}

#ifdef _DEBUG
	assert( m_wHeadSeqNo == (uint16_t)(AssociatedUserData.m_wLastSeqNo + 1) );
#endif // _DEBUG

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-02-12
 *
 *	Build and send one packet
 *
 * @param [in]	nPacketCount			total packet count
 * @param [in]	nIndex					packet index
 * @param [in]	Item					item to be send
 *
 * @return		0						succ
 *				other					error code
 */
int CMyRUDP_Packet_Sender::BuildOnePacket( int nPacketCount, int nIndex, SEND_PACKET_ITEM & Item )
{
	int i;

	uint8_t * pBuf = Item.m_pBuf;
	memset( pBuf, 0, MY_RUDP_HEADER_LEN );

	// build header
	pBuf[ 0 ] = MYRUDP_MAGIC_ID_HI;
	pBuf[ 1 ] = MYRUDP_MAGIC_ID_LOW;
	// current version is 0
	pBuf[ 2 ] = MYRUDP_CMD_DATA;
	pBuf[ 3 ] = 0;

	uint32_t dwTmp = m_dwSessionID;
	for( i=2; i>=0; i-- )
	{
		pBuf[4+i] = (uint8_t)dwTmp;
		dwTmp >>= 8;
	}

	pBuf[11] = (uint8_t)( Item.m_wSeqNo >> 8 );
	pBuf[12] = (uint8_t)( Item.m_wSeqNo );
	pBuf[13] = (uint8_t)( nPacketCount & 0x7F );
	pBuf[14] = (uint8_t)( nIndex & 0x7F );
	// 2015.4.19 CYJ Add, set MTU Index and Padding Length
	uint8_t byMTU_PadLen = m_byMTUIndex_Shifted;
#ifdef __MYRUDP_USE_OPENSSL__
	if( m_pbyDataEncryptAESKey )
	{	// do data encryption
		unsigned char abyIV[16];
		memcpy( abyIV, m_pbyDataEncryptAESKey+32, 16 );
		abyIV[ 6 ] = pBuf[11];
		abyIV[ 7 ] = pBuf[12];
		uint8_t * pbyAESKey = m_pbyDataEncryptAESKey + ( Item.m_wSeqNo & 0xF );

		int nEncryptedDataLen = ( Item.m_nDataLen + 15 ) & (~15);
		uint8_t byPaddingLen = nEncryptedDataLen - Item.m_nDataLen;
		Item.m_nDataLen = nEncryptedDataLen;
		byMTU_PadLen |= byPaddingLen;
		Item.m_pUserDataItem->m_nDataLen += byPaddingLen;

		AES_KEY aes;
		AES_set_encrypt_key( pbyAESKey, 256, &aes);
		uint8_t * pbyPayloadData = pBuf+MY_RUDP_HEADER_LEN;
		AES_cbc_encrypt( pbyPayloadData, pbyPayloadData, nEncryptedDataLen, &aes, abyIV, 1 );	// do encrypt
	}
#endif //#ifdef __MYRUDP_USE_OPENSSL__

	pBuf[15] = byMTU_PadLen;

	// encrypt sessionTag ID
    uint32_t dwSessionTagID = m_dwSessionTagID;
    uint32_t dwSeqNo = Item.m_wSeqNo;
    if( dwSeqNo )
	{
		// SessionTagID =  ( OrgSessionTagID ^ s_adwXorData[SeqNo] ) + (SeqNo << 10) + (PacketIndex << 4) + DataPadLen
		uint32_t dwXorData = 0;
		uint8_t *pXorData = m_pXorData;
		for( int i=0; i<4; i++ )
		{
			dwXorData <<= 8;
			dwXorData |= pXorData[ (dwSeqNo + i) & 0x1F ];
		}
		dwSessionTagID ^= dwXorData;
		dwSessionTagID += ( dwSeqNo << 10 ) + ( pBuf[14] << 4 ) + (pBuf[15] & 0x1F);
	}

	for( i=3; i>=0; i-- )
	{
		pBuf[7+i] = (uint8_t)dwSessionTagID;
		dwSessionTagID >>= 8;
	}

	UpdateCRC( pBuf );

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-01-29
 *
 *	update CRC16
 *
 * @param [in]	pBuf			common header buffer
 *
 */
void CMyRUDP_Packet_Sender::UpdateCRC( uint8_t * pBuf )
{
	uint16_t wCRC16 = CCRC::GetCRC16( MY_RUDP_HEADER_LEN-2, pBuf );
	pBuf[16] = (uint8_t)( wCRC16 );
	pBuf[17] = (uint8_t)( wCRC16 >> 8 );

#ifdef _DEBUG
	assert( 0 == CCRC::GetCRC16( MY_RUDP_HEADER_LEN, pBuf ) );
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-02-12
 *
 *	Get Available buffer count
 *
 * @return	available data buffer count
 */
int CMyRUDP_Packet_Sender::GetAvailableBufCount()
{
	uint16_t wTail = m_wTailSeqNo & MYRUDP_WINDOW_SIZE_MASK;
	uint16_t wHead = m_wHeadSeqNo & MYRUDP_WINDOW_SIZE_MASK;
	// [___________T-----H_____]
	if( wTail <= wHead )
		return MYRUDP_WINDOW_SIZE - 1 - wHead + wTail;
	// [-------H_______T-------]
     return wTail - wHead - 1;
}

//--------------------------------------------------------------
/** CYJ 2015-02-12
 *
 *	On timer to resend the packet that has not received ACK
 *
 * @param [in]	tNow				current time tick
 */
void CMyRUDP_Packet_Sender::OnTimer( time_t tNow )
{
	// to wait the event, 1 second, to realize N second timeout
	pthread_mutex_lock( &m_SyncObj_DataBufferAvailable );
	pthread_cond_signal( &m_EventDataBufAvailable );
	pthread_mutex_unlock( &m_SyncObj_DataBufferAvailable );

	// 2015.5.22 CYJ Add, too many data on the way, abort
	if( m_nSecondsNotGetDataFromPeer > NOT_SEND_SECNDS_NOT_GET_DATA_FROM_PEER )
		return;
	m_nSecondsNotGetDataFromPeer ++;

	if( m_wTailSeqNo == m_wHeadSeqNo )
		return;			// no data

	CMy_pthread_Mutex_Locker SyncObj( &m_SendDataSyncObj, false );
	if( false == SyncObj.TryLock() )
	{
		RegisterFastTimer();
		return;			// m_SendDataSyncObj is locked by other thread, return directly
	}

	// 2015.5.5 CYJ Modify, only send out one packet
	DoSendOnePacketByTimerOrACK( 1 );
}

//--------------------------------------------------------------
/** CYJ 2015-05-05
 *
 *	Do resend one packet to peer
 *
 * @note
 *	assume the caller has do sync
 */
void CMyRUDP_Packet_Sender::DoSendOnePacketByTimerOrACK( int nCount )
{
	if( m_wTailSeqNo == m_wHeadSeqNo )
		return;			// no data
	if( 0 == nCount )
		nCount = 1;

	uint16_t wSeqNo = m_wTailSeqNo;
	while( wSeqNo != m_wHeadSeqNo )
	{	// check from tail to head
		uint16_t wIndex = wSeqNo & MYRUDP_WINDOW_SIZE_MASK;
		SEND_PACKET_ITEM & Item = m_aPackets[ wIndex ];

	#ifdef _DEBUG
		assert( wSeqNo == Item.m_wSeqNo );
	#endif // _DEBUG

		wSeqNo ++;

		if( Item.m_wFlags & PACKET_SEND_FLAGS_GET_ACK )
			continue;		// has get ACK, not need to resend

		// 2015.4.2 CYJ add, if SKIP, not send again
		if( Item.m_wFlags & PACKET_SEND_FLAGS_SKIP )
		{	// maybe SKIP or ACK for SKIP lost.
		#ifdef _DEBUG
			assert( Item.m_pUserDataItem );
		#endif // _DEBUG
			OnPacketSendFailed( Item.m_pUserDataItem->m_wFirstSeqNo, Item.m_pUserDataItem->m_wLastSeqNo, NULL );
			continue;
		}

	#ifdef _DEBUG
		assert( Item.m_nDataLen );
	#endif // _DEBUG

		// 2015.4.2 CYJ Add, if send timeout, set failed
		if( ( 0 == Item.m_byMaxResendTimeout ) && ( 0 == ( Item.m_wFlags & PACKET_SEND_FLAGS_MUST_SUCC ) ) )
		{	// can be time out
			OnPacketSendTimeOut( Item );
			continue;
		}

		// 2015.4.2 CYJ Modify, if has been send and not set the retransmit flag, set the flag
		if( PACKET_SEND_FLAGS_HAS_BEEN_SENT == (Item.m_wFlags & (PACKET_SEND_FLAGS_HAS_BEEN_SENT|PACKET_SEND_FLAGS_RESEND) ) )
		{	// resend, first resend, modify flags
			Item.m_pBuf[3] |= 0x80;
			Item.m_wFlags |= PACKET_SEND_FLAGS_RESEND;
			UpdateCRC( Item.m_pBuf );

			if( IsSeqNoInRange( Item.m_wSeqNo, m_wPeerRcvTailSeqNo + MYRUDP_WINDOW_SIZE - 1, m_wPeerRcvTailSeqNo ) )
			{
				SendDataItemToPeer( Item );
				return;
			}
		}
	#ifdef _DEBUG
		assert( Item.m_byTimeOut );
	#endif // _DEBUG
		if( Item.m_byTimeOut )
			Item.m_byTimeOut --;
		if( Item.m_byTimeOut )
			continue;

		Item.m_byResendCount ++;
		if( Item.m_byResendCount < 4 )
			Item.m_byTimeOut = 2 + Item.m_byResendCount;
		else
			Item.m_byTimeOut = 5;
		if( Item.m_byMaxResendTimeout > Item.m_byTimeOut )
			Item.m_byMaxResendTimeout -= Item.m_byTimeOut;
		else
			Item.m_byMaxResendTimeout = 0;
	#if 0 && defined(_DEBUG)
		MyRUDP_fprintf( "\n====== Resend timeout (%2d) 0x%04x, local Head/Tail: 0x%04x / 0x%04x, Peer Head/Tail: 0x%04x / 0x%04x", \
						tNow%60, Item.m_wSeqNo, m_wHeadSeqNo, m_wTailSeqNo, m_wPeerRcvHeadSeqNo, m_wPeerRcvTailSeqNo );
	#endif // _DEBUG
		SendDataItemToPeer( Item );

		nCount --;
		if( nCount <= 0 )
			return;
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-26
 *
 *	Fast timer callback to send out delay packets
 */
void CMyRUDP_Packet_Sender::OnFastTimer()
{
	if( m_wTailSeqNo == m_wHeadSeqNo )
		return;			// no data
	// 2015.5.22 CYJ Add, too many data on the way, abort
	if( m_nSecondsNotGetDataFromPeer > NOT_SEND_SECNDS_NOT_GET_DATA_FROM_PEER )
		return;

	CMy_pthread_Mutex_Locker SyncObj( &m_SendDataSyncObj, false );
	if( false == SyncObj.TryLock() )
	{
		RegisterFastTimer();
		return;			// not wait
	}

	uint16_t wSeqNo = m_wTailSeqNo;
	while( wSeqNo != m_wHeadSeqNo )
	{	// check from tail to head
		uint16_t wIndex = wSeqNo & MYRUDP_WINDOW_SIZE_MASK;
		SEND_PACKET_ITEM & Item = m_aPackets[ wIndex ];

	#ifdef _DEBUG
		assert( wSeqNo == Item.m_wSeqNo );
	#endif // _DEBUG

		wSeqNo ++;

		// 2015.4.2 CYJ, Add PACKET_SEND_FLAGS_SKIP flags, when skip, not send again
		if( Item.m_wFlags & (PACKET_SEND_FLAGS_GET_ACK|PACKET_SEND_FLAGS_SKIP) )
			continue;		// has get ACK or has resend, not need to send again
		if( 0 == (Item.m_wFlags & PACKET_SEND_FLAGS_SEND_DELAY) )
			continue;		// not a delay-sending

		// 2015.5.9 CYJ Modify, using Item.m_wSeqNo instead of wSeqNo
		if( false == IsSeqNoInRange( Item.m_wSeqNo, m_wPeerRcvTailSeqNo + MYRUDP_WINDOW_SIZE - 1, m_wPeerRcvTailSeqNo ) )
		{
			RegisterFastTimer();
			return;
		}

		Item.m_wFlags &= (~PACKET_SEND_FLAGS_SEND_DELAY);

	#ifdef _DEBUG
		//MyRUDP_fprintf( "Delay send out 0x%04x\n", Item.m_wSeqNo );
	#endif // _DEBUG

		SendDataItemToPeer( Item );
		return;			// 2015.5.5 CYJ Add, only send out one packet
	}
}

//--------------------------------------------------------------
/** CYJ 2015-05-10
 *
 *	Force to send out N packets
 *
 * @param [in]	nCount				packet count
 */
void CMyRUDP_Packet_Sender::ForceSendoutPacket( int nCount )
{
	if( m_wTailSeqNo == m_wHeadSeqNo )
		return;			// no data

	CMy_pthread_Mutex_Locker SyncObj( &m_SendDataSyncObj, false );
	if( false == SyncObj.TryLock() )
		return;			// not wait

	uint16_t wSeqNo = m_wTailSeqNo;
	while( wSeqNo != m_wHeadSeqNo )
	{	// check from tail to head
		uint16_t wIndex = wSeqNo & MYRUDP_WINDOW_SIZE_MASK;
		SEND_PACKET_ITEM & Item = m_aPackets[ wIndex ];
	#ifdef _DEBUG
		assert( wSeqNo == Item.m_wSeqNo );
	#endif // _DEBUG
		wSeqNo ++;

		// 2015.4.2 CYJ, Add PACKET_SEND_FLAGS_SKIP flags, when skip, not send again
		if( Item.m_wFlags & (PACKET_SEND_FLAGS_GET_ACK|PACKET_SEND_FLAGS_SKIP) )
			continue;		// has get ACK or has resend, not need to send again

		// 2015.5.9 CYJ Modify, using Item.m_wSeqNo instead of wSeqNo
		if( false == IsSeqNoInRange( Item.m_wSeqNo, m_wPeerRcvTailSeqNo + MYRUDP_WINDOW_SIZE - 1, m_wPeerRcvTailSeqNo ) )
			return;

		SendDataItemToPeer( Item );

		if( --nCount < 0 )
			return;
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-26
 *
 *	Set Peer Expected SeqNo
 *
 * @param [in]	wSeqNo			expected SeqNo
 */
void CMyRUDP_Packet_Sender::SetPeerExpectedSeqNo( uint16_t wSeqNo )
{
	if( m_wPeerRcvHeadSeqNo == wSeqNo )
	{
		m_nSamePeerHeaderSeqNoCount ++;
		if( m_nSamePeerHeaderSeqNoCount < m_nSamePeerHeaderSeqNoGateLevel )
			return;
		m_nSamePeerHeaderSeqNoCount = 0;
		if( m_nSamePeerHeaderSeqNoGateLevel <= 16 )
			m_nSamePeerHeaderSeqNoGateLevel <<= 1;

		SEND_PACKET_ITEM & Item = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
	#if 0 && defined(_DEBUG)
		MyRUDP_fprintf( "Same Peer Head SeqNo 0x%04x more then %d times, Item: 0x%04x,Len:%d, Flags:0x%02x\n",\
			wSeqNo, m_nSamePeerHeaderSeqNoGateLevel/2, Item.m_wSeqNo, Item.m_nDataLen, Item.m_wFlags );
	#endif //_DEBUG
		if( Item.m_wSeqNo == wSeqNo && Item.m_nDataLen && 0 == ( Item.m_wFlags & PACKET_SEND_FLAGS_GET_ACK ) )
		{	// resend
			Item.m_wFlags |= PACKET_SEND_FLAGS_SEND_DELAY;
			RegisterFastTimer();
		}
	}
	else
	{
		m_nSamePeerHeaderSeqNoCount = 0;
		m_nSamePeerHeaderSeqNoGateLevel = 4;
		m_wPeerRcvHeadSeqNo = wSeqNo;
	}
}

//--------------------------------------------------------------
/** CYJ 2015-04-02
 *
 *	On ACK data packet recieved
 *
 * @param [in]	wHeadSeqNo			Head SeqNo
 * @param [in]	wTailSeqNo			Tail SeqNo
 * @param [in]	wPeerRcvHeadSeqNo	Peer receive head SeqNo
 * @param [in]	wPeerRevTailSeqNo	Peer Receive tail SeqNo
 */
void CMyRUDP_Packet_Sender::OnACK( const uint16_t wHeadSeqNo, const uint16_t wTailSeqNo, uint16_t wPeerRcvHeadSeqNo, uint16_t wPeerRevTailSeqNo )
{
	CMy_pthread_Mutex_Locker SyncObj( &m_SendDataSyncObj );

#if 0 && defined _DEBUG
	MyRUDP_fprintf( "---     ACK, Local= 0x%04x / 0x%04x, PeerTailSeqNo= 0x%04x / 0x%04x\n", m_wHeadSeqNo, m_wTailSeqNo, wPeerRcvHeadSeqNo, wPeerRevTailSeqNo );
	MyRUDP_fprintf( "OnACK:  0x%04x - 0x%04x \n", wHeadSeqNo, wTailSeqNo );
#endif // _DEBUG

	uint16_t wSeqNo = wHeadSeqNo - 1;
	while( wSeqNo != wTailSeqNo )
	{
		wSeqNo ++;
		OnOneSeqNoACK( wSeqNo );
	}

	DoOnACK( wPeerRcvHeadSeqNo, wPeerRevTailSeqNo );
}

//--------------------------------------------------------------
/** CYJ 2015-02-12
 *
 *	On ACK data packet recieved
 *
 * @param [in]	awACKSeqNo			ACK Sequence No
 * @param [in]	nACKCount			ACK count
 * @param [in]	wPeerRcvHeadSeqNo	Peer receive head SeqNo
 * @param [in]	wPeerRevTailSeqNo	Peer Receive tail SeqNo
 */
void CMyRUDP_Packet_Sender::OnACK( const uint16_t awACKSeqNo[], int nACKCount, uint16_t wPeerRcvHeadSeqNo, uint16_t wPeerRevTailSeqNo )
{
	CMy_pthread_Mutex_Locker SyncObj( &m_SendDataSyncObj );

#if 0 && defined _DEBUG
	MyRUDP_fprintf( "---     ACK, Local= 0x%04x / 0x%04x, PeerTailSeqNo= 0x%04x / 0x%04x\n", m_wHeadSeqNo, m_wTailSeqNo, wPeerRcvHeadSeqNo, wPeerRevTailSeqNo );
	char szTmp[256];
	char *pszT = szTmp;
	for(int i=0; i<nACKCount; i++ )
	{
		sprintf( pszT, "%04x ", awACKSeqNo[i] );
	}
	MyRUDP_fprintf( "OnACK:  %s\n", szTmp );
#endif // _DEBUG

	for( int i=0; i<nACKCount; i++ )
	{
		OnOneSeqNoACK( awACKSeqNo[i] );
	}

	DoOnACK( wPeerRcvHeadSeqNo, wPeerRevTailSeqNo );
}

//--------------------------------------------------------------
/** CYJ 2015-04-02
 *
 *	Do On ACK
 *
 * @param [in]	wPeerRcvHeadSeqNo	Peer receive head SeqNo
 * @param [in]	wPeerRevTailSeqNo	Peer Receive tail SeqNo
 *
 * @return
 *
 */
void CMyRUDP_Packet_Sender::DoOnACK( uint16_t wPeerRcvHeadSeqNo, uint16_t wPeerRevTailSeqNo )
{
	m_wPeerRcvTailSeqNo = wPeerRevTailSeqNo;
	RepaireACKByPeerRcvTailSeqNo( wPeerRcvHeadSeqNo );
	SetPeerExpectedSeqNo( wPeerRcvHeadSeqNo );

	uint16_t wOldTailSeqNo = m_wTailSeqNo;
    for( int i=0; i<MYRUDP_WINDOW_SIZE; i++ )
	{
		SEND_PACKET_ITEM & Item = m_aPackets[ m_wTailSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
		if( 0 == ( Item.m_wFlags & PACKET_SEND_FLAGS_GET_ACK ) )
			break;

	#ifdef _DEBUG
		if( m_wHeadSeqNo == m_wTailSeqNo )
			MyRUDP_fprintf( "--- %s, HeadSeqNo= 0x%04x, TailSeqNo= 0x%04x\n", __FUNCTION__, m_wHeadSeqNo, m_wTailSeqNo );
		assert( Item.m_wSeqNo == m_wTailSeqNo );
		assert( m_wHeadSeqNo != m_wTailSeqNo );
	#endif // _DEBUG

		// has get ACK
		Item.m_nDataLen = 0;			// clear the flags
		Item.m_wFlags = 0;
		Item.m_byMaxResendTimeout = 0;
		Item.m_byTimeOut = 0;
		Item.m_byResendCount = 0;

		m_wTailSeqNo ++;
	}

	if( wOldTailSeqNo != m_wTailSeqNo )
	{	// some data has been Received
		m_nSameTailSeqNoCount = 0;
		CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_DataBufferAvailable );
		pthread_cond_signal( &m_EventDataBufAvailable );
	}
	else if( m_nSameTailSeqNoCount >= 0 && m_wHeadSeqNo != m_wTailSeqNo )
	{
		m_nSameTailSeqNoCount ++;
		if( m_nSameTailSeqNoCount > 4 )
		{
			m_nSameTailSeqNoCount = -1;

			SEND_PACKET_ITEM & Item = m_aPackets[ m_wTailSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
			if( Item.m_nDataLen && 0 == ( Item.m_wFlags & (PACKET_SEND_FLAGS_GET_ACK|PACKET_SEND_FLAGS_SEND_DELAY) ) )
			{	// resend
				Item.m_wFlags |= PACKET_SEND_FLAGS_SEND_DELAY;
				RegisterFastTimer();
			}
		}
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-26
 *
 *	repare ACK by peer rcv tail SeqNo, to avoid ACK lost
 */
void CMyRUDP_Packet_Sender::RepaireACKByPeerRcvTailSeqNo( uint16_t wPeerRcvHeadSeqNo )
{
	uint16_t wRepareSeqNo = m_wPeerRcvTailSeqNo;

	for(int i=0; i<2; i++ )
	{
		if( 0 == wRepareSeqNo )
		{
			wRepareSeqNo = wPeerRcvHeadSeqNo;
			continue;
		}

		uint16_t wSeqNo = m_wTailSeqNo;
		uint16_t wPeerTailSeqNo = wRepareSeqNo - MYRUDP_WINDOW_SIZE;
		uint16_t wPeerHeadSeqNo = wRepareSeqNo - 1;
		for( int i=0; i<MYRUDP_WINDOW_SIZE && wSeqNo != m_wHeadSeqNo; i++, wSeqNo++ )
		{
			SEND_PACKET_ITEM & Item = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
			if( Item.m_wFlags & PACKET_SEND_FLAGS_GET_ACK )
				continue;
			if( IsSeqNoInRange( wSeqNo, wPeerHeadSeqNo, wPeerTailSeqNo ) )
			{	// maybe ACK lost
			#ifdef _DEBUG
				MyRUDP_fprintf( "Repare ACK 0x%04x, TailSeq: 0x%04x, PeerTailSeqNo; 0x%04x / 0x%04x\n", wSeqNo, m_wTailSeqNo, m_wPeerRcvHeadSeqNo, m_wPeerRcvTailSeqNo );
				if( m_wHeadSeqNo == wSeqNo )
					MyRUDP_fprintf( "--- %s, HeadSeqNo= 0x%04x, TailSeqNo= 0x%04x\n", __FUNCTION__, m_wHeadSeqNo, wSeqNo );
				assert( Item.m_wSeqNo == wSeqNo );
				assert( m_wHeadSeqNo != wSeqNo );
			#endif // _DEBUG

				OnOneSeqNoACK( wSeqNo );
			}
		}

		wRepareSeqNo = wPeerRcvHeadSeqNo;
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-23
 *
 *	On resend request
 *
 * @param [in]	awSeqNo				SeqNo to be resend
 * @param [in]	nCount				SeqNo count
 * @param [in]	wPeerRcvHeadSeqNo	Peer receive head SeqNo
 * @param [in]	wPeerRevTailSeqNo	Peer Receive tail SeqNo
 */
void CMyRUDP_Packet_Sender::OnResend( const uint16_t awSeqNo[], int nCount, uint16_t wPeerRcvHeadSeqNo, uint16_t wPeerRevTailSeqNo )
{
#ifdef _DEBUG
//	MyRUDP_fprintf( "==== OnResend( 0x%04x ) / %d\n", awSeqNo[0], nCount );
#endif // _DEBUG

	CMy_pthread_Mutex_Locker SyncObj( &m_SendDataSyncObj );

	m_wPeerRcvTailSeqNo = wPeerRevTailSeqNo;
	RepaireACKByPeerRcvTailSeqNo( wPeerRcvHeadSeqNo );
	SetPeerExpectedSeqNo( wPeerRcvHeadSeqNo );

	for(int i=0; i<nCount; i++ )
	{
		uint16_t wSeqNo = awSeqNo[i];
		if( false == IsSeqNoValid( wSeqNo ) )
		{
		#ifdef _DEBUG
			MyRUDP_fprintf( "==== OnResend( 0x%04x ), invalid SeqNo, Head: 0x%04x, Tail: 0x%04x\n", wSeqNo, m_wHeadSeqNo, m_wTailSeqNo );
		#endif // _DEBUG
			continue;				// not a valid SeqNo, skip
		}

		uint16_t wIndex = wSeqNo & MYRUDP_WINDOW_SIZE_MASK;
		SEND_PACKET_ITEM & Item = m_aPackets[ wIndex ];
		if( wSeqNo != Item.m_wSeqNo )
		{
		#ifdef _DEBUG
			MyRUDP_fprintf( "==== OnResend( 0x%04x != 0x%04x\n", wSeqNo, Item.m_wSeqNo );
		#endif // _DEBUG
			continue;
		}

		// 2015.4.2 CYJ add, to support data SKIP since send failed
		if( Item.m_wFlags & PACKET_SEND_FLAGS_SKIP )
		{
		#ifdef _DEBUG
			assert( Item.m_pUserDataItem );
		#endif // _DEBUG

			OnPacketSendFailed( Item.m_pUserDataItem->m_wFirstSeqNo, Item.m_pUserDataItem->m_wLastSeqNo, NULL );
			continue;
		}

		if( Item.m_wFlags & PACKET_SEND_FLAGS_GET_ACK )
		{
		#ifdef _DEBUG
			MyRUDP_fprintf( "==== OnResend( 0x%04x ) has got ACK\n", wSeqNo );
		#endif // _DEBUG
			continue;		// has get ACK, not need to resend
		}

	#ifdef _DEBUG
		assert( Item.m_nDataLen );
	#endif // _DEBUG

		// 2015.4.2 CYJ Modify, has been send and not set the ReTransmit Flag, set the retransmit flag
		if( PACKET_SEND_FLAGS_HAS_BEEN_SENT == (Item.m_wFlags & (PACKET_SEND_FLAGS_HAS_BEEN_SENT|PACKET_SEND_FLAGS_RESEND) ) )
		{
			Item.m_pBuf[3] |= 0x80;
			UpdateCRC( Item.m_pBuf );
			Item.m_wFlags |= PACKET_SEND_FLAGS_RESEND;
		}
		SendDataItemToPeer( Item );
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-15
 *
 *  Is SeqNo in the range
 *
 * @param [in]	wSeqNo				SeqNo to be checked
 * @param [in]	wHeadSeqNo			head SeqNo
 * @param [in]	wTailSeqNo			tail SeqNo
 *
 * @return		true				in the range
 *				false				not in the range
 */
bool CMyRUDP_Packet_Sender::IsSeqNoInRange( uint16_t wSeqNo, uint16_t wHeadSeqNo, uint16_t wTailSeqNo )
{
	// [____________T----H___]
	// [----H__________T-----]
	// FIXME, "<= wHeadSeqNo" or "< wHeadSeqNo" ???
	if( wTailSeqNo <= wHeadSeqNo )
		return ( wSeqNo >= wTailSeqNo && wSeqNo <= wHeadSeqNo );
	else
		return ( wSeqNo >= wTailSeqNo || wSeqNo <= wHeadSeqNo );
}

//--------------------------------------------------------------
/** CYJ 2015-02-15
 *
 *	Is SeqNo valid
 *
 * @param [in]	wSeqNo				SeqNo to be checked
 *
 * @return		true				in the range
 *				false				not in the range
 */
bool CMyRUDP_Packet_Sender::IsSeqNoValid( uint16_t wSeqNo )
{
	return IsSeqNoInRange( wSeqNo, m_wHeadSeqNo, m_wTailSeqNo );
}

//--------------------------------------------------------------
/** CYJ 2015-02-15
 *
 *	On one SeqNo ACK Received
 *
 * @param [in]	wSeqNo					SeqNo
 */
void CMyRUDP_Packet_Sender::OnOneSeqNoACK( uint16_t wSeqNo )
{
	if( false == IsSeqNoValid( wSeqNo ) )
		return;				// not a valid SeqNo, may be delayed reached ACK

	uint16_t wIndex = wSeqNo & MYRUDP_WINDOW_SIZE_MASK;
	SEND_PACKET_ITEM & Item = m_aPackets[ wIndex ];

#ifdef _DEBUG
	assert( Item.m_wSeqNo == wSeqNo );
	assert( Item.m_pUserDataItem );
#endif // _DEBUG

	if( NULL == Item.m_pUserDataItem || 0 == Item.m_nDataLen || (Item.m_wFlags&PACKET_SEND_FLAGS_GET_ACK) )
		return;	// if 0 == Item.m_nDataLen => Has get the ACK and clear it

#ifdef _DEBUG
//	MyRUDP_fprintf( "Get ACK 0x%04x, TailSeq: 0x%04x\n", wSeqNo, m_wTailSeqNo );
#endif //_DEBUG

	Item.m_wFlags |= PACKET_SEND_FLAGS_GET_ACK;

	PSEND_PACKET_USER_DATA pUserDataItem = Item.m_pUserDataItem;

	pUserDataItem->m_nDataLen -= Item.m_nDataLen;
	pUserDataItem->m_nPacketCount --;

#ifdef _DEBUG
	assert( pUserDataItem->m_nDataLen >= 0 );
	if( pUserDataItem->m_nPacketCount )
		assert( pUserDataItem->m_nDataLen > 0 );
	assert( pUserDataItem->m_nPacketCount >= 0 );
	if( 0 == pUserDataItem->m_nPacketCount )
		assert( 0 == pUserDataItem->m_nDataLen );
	assert( IsSeqNoInRange( wSeqNo, pUserDataItem->m_wLastSeqNo, pUserDataItem->m_wFirstSeqNo ) );
	bool bIsInRange = false;
	for( uint16_t wS = pUserDataItem->m_wFirstSeqNo; ; wS ++ )
	{
		if( IsSeqNoValid( wS) )
		{
			bIsInRange = true;
			break;
		}
		if( wS == pUserDataItem->m_wLastSeqNo )
			break;
	}
	assert( bIsInRange );
#endif // _DEBUG

	if( pUserDataItem->m_nPacketCount )
		return;

	if( 0 == ( Item.m_wFlags & PACKET_SEND_FLAGS_SKIP ) )
	{
		// full packet has been send succ, notify one packet has been sent succ
		OnPacketHasBeenSendSucc( pUserDataItem->m_pUserData );
	}

	// 2015.4.2 CYJ Modify, move to function RemoveUserDataItem
	RemoveUserDataItem( pUserDataItem );
}

//--------------------------------------------------------------
/** CYJ 2015-04-02
 *
 *	remove user data item, since data block send succ, or skip
 *
 * @param [in]	pUserDataItem			user data item
 */
void CMyRUDP_Packet_Sender::RemoveUserDataItem( PSEND_PACKET_USER_DATA pUserDataItem )
{
#ifdef _DEBUG
	bool bFound = false;
#endif // _DEBUG

	// all sub block data has been sent, notify one packet has been send succ
	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj_UserData );

	uint16_t wFirstSeqNo = pUserDataItem->m_wFirstSeqNo;
	for(std::list<SEND_PACKET_USER_DATA>::iterator it = m_listUserData.begin(); it != m_listUserData.end(); )
	{
		SEND_PACKET_USER_DATA & Item = *it;
		if( Item.m_wFirstSeqNo != wFirstSeqNo )
		{
			++it;
			continue;
		}

	#ifdef _DEBUG
		assert( Item.m_wLastSeqNo == pUserDataItem->m_wLastSeqNo );
		assert( &Item == pUserDataItem );
		bFound = true;
	#endif // _DEBUG

		it = m_listUserData.erase( it );
		break;
	}

#ifdef _DEBUG
	assert( bFound );
	assert( m_listUserData.size() < 16 + MYRUDP_WINDOW_SIZE );
#endif // _DEBUG
}

#ifdef _DEBUG
//--------------------------------------------------------------
/** CYJ 2015-02-13
 *
 *	Debug dump data info
 */
void CMyRUDP_Packet_Sender::DebugDump()
{
	CMyRUDP_fprintf_SyncHelper	fprintf_Sync;

	fprintf( stderr, "----------------------- PACKET SENDER----------------\n" );

	fprintf( stderr, "  HEAD_SQE: 0x%04x, TAIL_SEQ: 0x%04x\n", m_wHeadSeqNo, m_wTailSeqNo );

	for(int i=0; i<MYRUDP_WINDOW_SIZE; i++ )
	{
		SEND_PACKET_ITEM & Item = m_aPackets[ i & MYRUDP_WINDOW_SIZE_MASK ];
		fprintf( stderr, " ( %4d, 0x%04x, 0x%04x, %3d ), ", Item.m_nDataLen, Item.m_wSeqNo, Item.m_wFlags, Item.m_byMaxResendTimeout );

		if( (i&3) == 3 )
			fprintf( stderr, "\n" );
	}

	fprintf( stderr, "\n" );
}
#endif // _DEBUG

//--------------------------------------------------------------
/** CYJ 2015-04-02
 *
 *	On packet send timeout
 *
 * @param [in]	Item				item send timeout
 *
 * @note
 *		set the full packet to be Skip and send SKIP command to peer
 */
void CMyRUDP_Packet_Sender::OnPacketSendTimeOut( SEND_PACKET_ITEM & Item )
{
#ifdef _DEBUG
	assert( Item.m_pUserDataItem );
	assert( 0 == (Item.m_wFlags & PACKET_SEND_FLAGS_MUST_SUCC ) );
	assert( 0 == Item.m_byMaxResendTimeout );
#endif // _DEBUG

	if( NULL == Item.m_pUserDataItem )
		return;

	void * pUserData = Item.m_pUserDataItem->m_pUserData;

	uint16_t wHeadSeqNo = Item.m_pUserDataItem->m_wFirstSeqNo;
	uint16_t wTailSeqNo = Item.m_pUserDataItem->m_wLastSeqNo;

	uint16_t wSeqNo = wHeadSeqNo - 1;
	do
	{
		wSeqNo ++;
		SEND_PACKET_ITEM & TmpItem = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];

	#ifdef _DEBUG
		assert( 0 == ( TmpItem.m_wFlags & PACKET_SEND_FLAGS_MUST_SUCC ) );
	#endif // _DEBUG

		TmpItem.m_wFlags |= PACKET_SEND_FLAGS_SKIP;
	}while( wSeqNo != wTailSeqNo );

	// send event to app, data packet send failed.
	OnPacketSendFailed( wHeadSeqNo, wTailSeqNo, pUserData );
}

#ifdef __MYRUDP_USE_OPENSSL__
void CMyRUDP_Packet_Sender::SetDataEncryptionAESKey( uint8_t abyAESKey[] )
{
	m_pbyDataEncryptAESKey = abyAESKey;				// 48 bytes
}
#endif //#ifdef __MYRUDP_USE_OPENSSL__

//--------------------------------------------------------------
/** CYJ 2015-05-22
 *
 *	On data received from peer
 */
void CMyRUDP_Packet_Sender::OnDataReceivedFromPeer()
{
	m_nSecondsNotGetDataFromPeer = 0;
}

//--------------------------------------------------------------
/** CYJ 2015-05-22
 *
 *	Send data packet to peer
 *
 * @param [in]	Item			item to peer
 */
void CMyRUDP_Packet_Sender::SendDataItemToPeer( SEND_PACKET_ITEM & Item )
{
	SendDataPacketToPeer( Item.m_pBuf, Item.m_nDataLen + MY_RUDP_HEADER_LEN );
	Item.m_wFlags |= PACKET_SEND_FLAGS_HAS_BEEN_SENT;
}

