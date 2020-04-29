/******************************************************************************
 *
 *	My RUDP Data packet manager
 *
 *	Chen Yongjian @ Zhoi
 *	2015.2.5 @ Xi'an
 *
 *****************************************************************************/

#include <stdafx.h>
#include <errno.h>

#include <my_pthread_mutex_help.h>

#ifdef __MYRUDP_USE_OPENSSL__
	#include <openssl/aes.h>
#endif // __MYRUDP_USE_OPENSSL__

#include "myrudp_packet_mgr.h"
#include "myrudp_cmd_value.h"
#include "myrudp_dbgprint.h"

#ifdef _DEBUG
extern void DbgDumpData( const char *pszTitle, const uint8_t * pBuf, int nLen );
#endif //_DEBUG


CMyRUDP_PacketMgr::CMyRUDP_PacketMgr()
{
	m_pDataBuf = NULL;								// data buffer
	memset( m_aPackets, 0, sizeof(m_aPackets) );
	m_nMTUSize = 512;
	m_wTailSeqNo = 0;
	m_wHeadSeqNo = 0;
	m_wSeqNoExpected = 0;			// SeqNo expected
	m_wSeqNoExpected_Last = 0x8000;
	m_nSeqNoExpectedSameTimes = 0;

	pthread_mutex_init( &m_SyncObj, NULL);

#ifdef __MYRUDP_USE_OPENSSL__
	m_pbyDataEncryptAESKey = NULL;				// 48 bytes
#endif //#ifdef __MYRUDP_USE_OPENSSL__
}

CMyRUDP_PacketMgr::~CMyRUDP_PacketMgr()
{
	Invalidate();
	pthread_mutex_destroy( &m_SyncObj );
}

//--------------------------------------------------------------
/** CYJ 2015-02-05
 *
 *	Initialize
 *
 * @param [in]	nMTUSize				MTU size according to the negotiation between client and server
 *										when nMTUSize < 16 then nMTUSize used as MTU size Index
 * @return		0						succ
 *				other					error code
 */
int CMyRUDP_PacketMgr::Initialize( int nMTUSize )
{
	Invalidate();

	m_wTailSeqNo = 0;
	m_wHeadSeqNo = 0;
	m_nSeqNoExpectedSameTimes = 0;
	m_wSeqNoExpected = 0;
	m_wSeqNoExpected_Last = 0x8000;

	if( nMTUSize < 16 )
		nMTUSize = MyRUDP_GetMTUSizeByIndex( nMTUSize );	// used as MTU index
	else
	{
		if( MyRUDP_GetMTUIndexBySize( nMTUSize ) < 0 )
		{
		#ifdef _DEBUG
			MyRUDP_fprintf( "CMyRUDP_PacketMgr::%s, invalid MTUSize .\n", __FUNCTION__, nMTUSize );
		#endif // _DEBUG
			return -1;
		}
	}

    m_nMTUSize = nMTUSize;

	int nBufSize = m_nMTUSize * MYRUDP_WINDOW_SIZE;
    m_pDataBuf = new uint8_t[ nBufSize ];
    if( NULL == m_pDataBuf )
    {
	#ifdef _DEBUG
    	MyRUDP_fprintf( "CMyRUDP_PacketMgr::%s, allocate memory %d bytes failed.\n", __FUNCTION__, nBufSize );
	#endif //_DEBUG
		return -ENOMEM;
    }

    memset( m_aPackets, 0, sizeof(m_aPackets) );
    uint8_t *pBuf = m_pDataBuf;
    for(int i=0; i<MYRUDP_WINDOW_SIZE; i++ )
	{
		m_aPackets[i].m_pBuf = pBuf;
		pBuf += m_nMTUSize;
	}

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-02-05
 *
 *	Invalidate and free resource
 */
void CMyRUDP_PacketMgr::Invalidate()
{
	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj );

	if( m_pDataBuf )
		delete m_pDataBuf;
	m_pDataBuf = NULL;

	memset( m_aPackets, 0, sizeof(m_aPackets) );
	m_nMTUSize = 512;
	m_wTailSeqNo = 0;
	m_wHeadSeqNo = 0;
	m_aReadyPackets.clear();

#ifdef __MYRUDP_USE_OPENSSL__
	m_pbyDataEncryptAESKey = NULL;				// 48 bytes
#endif //#ifdef __MYRUDP_USE_OPENSSL__
}

//--------------------------------------------------------------
/** CYJ 2015-02-08
 *
 *	reset all packet info item to be no data.
 */
void CMyRUDP_PacketMgr::Reset()
{
	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj );

	memset( m_aPackets, 0, sizeof(m_aPackets) );
	m_wTailSeqNo = 0;
	m_wHeadSeqNo = 0;

	m_aReadyPackets.clear();

	memset( m_aPackets, 0, sizeof(m_aPackets) );
	if( m_pDataBuf )
	{
		uint8_t *pBuf = m_pDataBuf;
		for(int i=0; i<MYRUDP_WINDOW_SIZE; i++ )
		{
			m_aPackets[i].m_pBuf = pBuf;
			pBuf += m_nMTUSize;
		}
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-07
 *
 *	On one data packet received
 *
 * @param [in]	pBuf					data buffer
 * @param [in]	nLen					data length
 * @param [out]	awACK_SeqNo				output ACK SeqNo
 * @param [i/o]	nACKCount				input  	awACK_SeqNo buffer size,max count = 16
 *										output	Actual ACK SeqNo output to the buffer
 * @return		0						need to wait more data packet
 *				>0						full packet count that has been completed received
 *				<0						error data
 */
int CMyRUDP_PacketMgr::OnDataPacketReady( const uint8_t * pBuf, int nLen, uint16_t awACK_SeqNo[], int & nACKCount )
{
#ifdef _DEBUG
//	MyRUDP_fprintf( "--- %s ( %5d bytes, %02x -- 0x%02x%02x, %02x/%02x => %02x %02x )\n", __FUNCTION__, nLen, pBuf[2], pBuf[11], pBuf[12], pBuf[14], pBuf[13], pBuf[18], pBuf[19] );
#endif // _DEBUG

	if( NULL == pBuf || nLen < MY_RUDP_HEADER_LEN )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d\n", __FUNCTION__, __LINE__ );
	#endif // _DEBUG
		return -1;
	}

	CMyRUDP_HeaderReader Header( pBuf );

    const uint16_t wSeqNo = Header.GetSeqNo();
    if( false == IsSeqNoValid( wSeqNo ) )
	{
	#if 0 && defined(_DEBUG)
		MyRUDP_fprintf( "\n************** Invalid SeqNo=0x%04x, Tail=0x%04x, Head=0x%04x\n", wSeqNo, m_wTailSeqNo, m_wHeadSeqNo );
	//	DbgDumpData( "\n\n\n******* New On Packet Ready", pBuf, nLen );
		PACKET_INFO_ITEM & Item = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
		pBuf = Item.m_pBuf;
		nLen = Item.m_nDataLen;
		MyRUDP_fprintf( "Exist Item SeqNo: 0x%04x  / 0x%04x, DataLen=%5d, Flags=0x%02x\n", Item.m_wSeqNo, wSeqNo, Item.m_nDataLen, Item.m_byFlags );
	//	DbgDumpData( "\n\n\n******* Exist Packet Ready", pBuf, nLen );
		#if 0
			for(int i=0; i<16; i++ )
			{
				PACKET_INFO_ITEM & Item = m_aPackets[ (m_wHeadSeqNo+i) & MYRUDP_WINDOW_SIZE_MASK ];
				char cHeadIndicator = ' ';
				if( 0 == Item.m_byPacketCount )
					cHeadIndicator = '-';
				else if( 0 == Item.m_byPacketIndex )
					cHeadIndicator = 'H';
				else if( Item.m_byPacketIndex + 1 ==  Item.m_byPacketCount )
					cHeadIndicator = 't';
				MyRUDP_fprintf( "SeqNo: 0x%04x / 0x%04x, PacketCount=%3d, DataLen=%5d / %5d (%c), Flags=0x%02x\n", \
								(uint16_t)(m_wHeadSeqNo+i), Item.m_wSeqNo, Item.m_byPacketCount, Item.m_nDataLen,\
								Item.m_nFullDataLen, cHeadIndicator, Item.m_byFlags );
			}
		#endif // 0
	#endif // _DEBUG

		if( IsSeqNoValid( wSeqNo+MYRUDP_WINDOW_SIZE ) )
		{	// for example, 0001 has been received succ, but ACK-0001 lost
			// so, the client will resend SeqNo=0001, but the TailSeqNo = 0x100 now
			// so, 0x0001 is not a valid SeqNo, and the client not got ACK-0001, so it still resend 0001
			// now 1 + 0x100 => 0x101, 0x101 is a valid SeqNo, so send the ACK-0001 to the client
			// and the client will skip the 0001 and continue send subsequence packets.
		#ifdef _DEBUG
		//	MyRUDP_fprintf( "====!!!==== Maybe ACK been lost, send ACK back (0x%0x), Tail=0x%04x.\n", wSeqNo, m_wTailSeqNo );
		#endif // _DEBUG
			nACKCount = 1;
			awACK_SeqNo[0] = wSeqNo;
			return 0;
		}

		return -2;				// error seqence No.
	}
	if( Header.GetPackteIndex() >= Header.GetPackteCount() )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d\n", __FUNCTION__, __LINE__ );
	#endif // _DEBUG
		return -3;				// error data
	}

	// note: the data has been decrypted
    const uint8_t * pPayloadData = pBuf + MY_RUDP_HEADER_LEN;
    int nPayloadLen = nLen - MY_RUDP_HEADER_LEN - Header.GetPaddingDataLen();
#ifdef _DEBUG
	assert( nPayloadLen );
#endif // _DEBUG

	if( nPayloadLen <= 0 )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "--- %s : %d\n", __FUNCTION__, __LINE__ );
	#endif // _DEBUG
		return -4;
	}

	CollectACKSeq( wSeqNo, awACK_SeqNo, nACKCount );

	PACKET_INFO_ITEM & Item = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
	if( Item.m_nDataLen && Item.m_wSeqNo == wSeqNo )
	{
	#ifdef _DEBUG
	//	MyRUDP_fprintf( "***** duplicated packet 0x%04x\n", wSeqNo );
	#endif // _DEBUG
		return 0;				// data has been received, skip it, no full packet received
	}

#ifdef __MYRUDP_USE_OPENSSL__
	if( m_pbyDataEncryptAESKey )
		DoDecryptData( pBuf, nLen, Item.m_pBuf );
	else
		memcpy( Item.m_pBuf, pPayloadData, nPayloadLen );
#else
	memcpy( Item.m_pBuf, pPayloadData, nPayloadLen );
#endif // __MYRUDP_USE_OPENSSL__

	Item.m_nDataLen = nPayloadLen;						// data length
	Item.m_wSeqNo = wSeqNo;								// SeqNumber
	Item.m_byPacketCount = Header.GetPackteCount();		// packet count of the data
	Item.m_byPacketIndex = Header.GetPackteIndex();		// packet index of the data
	Item.m_byFlags = 0;				// flags

	if( 0 == Item.m_byPacketIndex )
		Item.m_nFullDataLen += nPayloadLen;	// the first packet
	else
	{
		Item.m_nFullDataLen = 0;
		uint16_t wPacket0_SeqNo = wSeqNo - Item.m_byPacketIndex;
		PACKET_INFO_ITEM & Packet0Item = m_aPackets[ wPacket0_SeqNo & MYRUDP_WINDOW_SIZE_MASK ];
		Packet0Item.m_nFullDataLen += nPayloadLen;
	#ifdef _DEBUG
		assert( 0 == Packet0Item.m_byPacketCount || Packet0Item.m_byPacketCount == Item.m_byPacketCount );
		assert( 0 == Packet0Item.m_byPacketIndex );
		assert( Packet0Item.m_nFullDataLen <= Item.m_byPacketCount * m_nMTUSize );
	#endif // _DEBUG
	}

#ifdef _DEBUG
	assert( Item.m_byPacketCount );
	assert( Item.m_byPacketIndex <= Item.m_byPacketCount );
	assert( nPayloadLen <= m_nMTUSize );
	if( (Item.m_byPacketIndex + 1 ) != Item.m_byPacketCount )
		assert( nPayloadLen == m_nMTUSize );
	#if 0
		for(int i=0; i<16; i++ )
		{
			PACKET_INFO_ITEM & Item = m_aPackets[ (m_wHeadSeqNo+i) & MYRUDP_WINDOW_SIZE_MASK ];
			char cHeadIndicator = ' ';
			if( 0 == Item.m_byPacketCount )
				cHeadIndicator = '-';
			else if( 0 == Item.m_byPacketIndex )
				cHeadIndicator = 'H';
			else if( Item.m_byPacketIndex + 1 ==  Item.m_byPacketCount )
				cHeadIndicator = 't';
			MyRUDP_fprintf( "SeqNo: 0x%04x / 0x%04x, PacketCount=%3d, DataLen=%5d / %5d (%c), Flags=0x%02x\n", \
							(uint16_t)(m_wHeadSeqNo+i), Item.m_wSeqNo, Item.m_byPacketCount, Item.m_nDataLen,\
							Item.m_nFullDataLen, cHeadIndicator, Item.m_byFlags );
		}
	#endif // 0
#endif // _DEBUG

	int nRetVal = 0;
	for( int i=0; i<MYRUDP_WINDOW_SIZE; i++ )
	{
		PACKET_INFO_ITEM & Item = m_aPackets[ m_wHeadSeqNo & MYRUDP_WINDOW_SIZE_MASK ];

		// 2015.4.2 CYJ add, to support Skip command
		if( PACKET_INFO_FLAGS_SKIP == ( Item.m_byFlags & (PACKET_INFO_FLAGS_SKIP|PACKET_INFO_FLAGS_COMMIT) ) )
		{
			Item.m_nFullDataLen = 0;		// complete packet data size
			Item.m_byPacketCount = 0;		// packet count of the data
			Item.m_byPacketIndex = 0;		// packet index of the data
			Item.m_byFlags |= PACKET_INFO_FLAGS_COMMIT;

			m_wHeadSeqNo ++;
			continue;
		}

		if( 0 == Item.m_nDataLen || 0 == Item.m_byPacketCount || ( Item.m_byFlags & PACKET_INFO_FLAGS_COMMIT ) )
			break;				// no data

	#ifdef _DEBUG
		assert( 0 == Item.m_byPacketIndex );
		if( m_wHeadSeqNo != Item.m_wSeqNo )
			MyRUDP_fprintf( " m_wHeadSeqNo ( 0x%04x ) != Item.m_wSeqNo( 0x%04x )\n", m_wHeadSeqNo, Item.m_wSeqNo );
		assert( m_wHeadSeqNo == Item.m_wSeqNo );
		if( 1 == Item.m_byPacketCount )
		{
			assert( 0 == Item.m_byPacketIndex );
			assert( Item.m_nFullDataLen == Item.m_nDataLen );
		}
	#endif // _DEBUG

		if( Item.m_byPacketCount > 1 && Item.m_nFullDataLen <= (Item.m_byPacketCount-1) * m_nMTUSize )
			break;				// not received completed

		// indicated the data packet has been commit, to avoid the last packet is received lastest and commit more times
		Item.m_byFlags |= PACKET_INFO_FLAGS_COMMIT;

	#ifdef _DEBUG
		if( Item.m_byPacketCount > 1 )
			assert( IsAllPacketsReceived( m_wHeadSeqNo ) );
		assert( m_wTailSeqNo != (m_wHeadSeqNo+Item.m_byPacketCount) );
	#endif // _DEBUG

		CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj );
		m_aReadyPackets.push_back( m_wHeadSeqNo );
		m_wHeadSeqNo += Item.m_byPacketCount;

		nRetVal ++;
	}

	GetExpectSeqNoToBeResend();

	return nRetVal;
}

//--------------------------------------------------------------
/** CYJ 2015-02-26
 *
 *	Get the expected SeqNo to be resend
 */
void CMyRUDP_PacketMgr::GetExpectSeqNoToBeResend()
{
	uint16_t wSeqNo = m_wHeadSeqNo;
	for( int i=0; i<MYRUDP_WINDOW_SIZE; i++ )
	{
		PACKET_INFO_ITEM & Item = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
		if( Item.m_byFlags & PACKET_INFO_FLAGS_COMMIT )
		{
		#ifdef _DEBUG
			assert( Item.m_wSeqNo == m_wTailSeqNo || (Item.m_byFlags&PACKET_INFO_FLAGS_SKIP) );
		#endif // _DEBUG
			break;
		}
		if( Item.m_nDataLen && Item.m_byPacketCount )
		{
			wSeqNo ++;
			continue;
		}

		// find one SeqNo to be resend
		if( m_wSeqNoExpected == wSeqNo )
		{
		#if 0 && defined _DEBUG
			MyRUDP_fprintf( "HeadSeq Same: 0x%04x, Tail: 0x%04x, Expect: 0x%04x, DataLen:%d, PacketCount:%d\n",\
						m_wHeadSeqNo, m_wTailSeqNo, m_wSeqNoExpected, Item.m_nDataLen, Item.m_byPacketCount );
		#endif // _DEBUG
			if( m_nSeqNoExpectedSameTimes < 32768 )
				m_nSeqNoExpectedSameTimes ++;
		}
		else
		{
			m_nSeqNoExpectedSameTimes = 0;
			m_wSeqNoExpected = wSeqNo;

		#if 0 && defined _DEBUG
			MyRUDP_fprintf( "HeadSeq: 0x%04x, Tail: 0x%04x, Expect: 0x%04x, DataLen:%d, PacketCount:%d\n",\
						m_wHeadSeqNo, m_wTailSeqNo, m_wSeqNoExpected, Item.m_nDataLen, Item.m_byPacketCount );
		#endif // _DEBUG
		}
		return;
	}

#ifdef _DEBUG
	MyRUDP_fprintf( "all data are ready, waiting reading data.\n" );
	for(int i=0; i<MYRUDP_WINDOW_SIZE; i++ )
	{
		PACKET_INFO_ITEM & Item = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
		assert( Item.m_nDataLen );
	}
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-02-11
 *
 *	Collect ACK seq No.
 *
 * @param [in]	wSeqNo			current packet's SeqNo
 * @param [out]	awACK_SeqNo		output SeqNo buffer
 * @param [i/o]	nACKCount		input: SeqNo Buffer size
 *								output:SeqNo count
 */
void CMyRUDP_PacketMgr::CollectACKSeq( uint16_t wSeqNo, uint16_t awACK_SeqNo[], int & nACKCount )
{
	int nCount = nACKCount - 1;

	awACK_SeqNo[ 0 ] = wSeqNo;
	nACKCount = 1;
	wSeqNo --;

    for(int i=0; i<MYRUDP_MAX_ACK_SEQNO_COUNT && nCount > 0; i++ )
	{
		PACKET_INFO_ITEM & Item = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
		if( Item.m_wSeqNo == wSeqNo && Item.m_nDataLen )				// data length
		{
			awACK_SeqNo[ nACKCount ] = wSeqNo;
			nACKCount ++;
			nCount --;
		}

		wSeqNo --;
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-11
 *
 *	Is all packets received
 *
 * @param [in]	wSeqNo				first packet's SeqNo
 *
 * @return		true				all packets are received
 *				false				some packets not received
 */
bool CMyRUDP_PacketMgr::IsAllPacketsReceived( uint16_t wSeqNo )
{
	PACKET_INFO_ITEM & Packet0Item = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
	int nPacketCount = Packet0Item.m_byPacketCount;
	for( int i=0; i<nPacketCount; i++ )
	{
		PACKET_INFO_ITEM & Item = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
		if( 0 == Item.m_nDataLen || 0 == Item.m_byPacketCount )
			return false;				// packet not received

	#ifdef _DEBUG
		assert( Item.m_byPacketCount == nPacketCount );
		assert( Item.m_byPacketIndex == i );
		assert( Item.m_wSeqNo == wSeqNo );
		if( i )
			assert( 0 == Item.m_nFullDataLen );
	#endif // _DEBUG

		wSeqNo ++;
	}

#ifdef _DEBUG
	wSeqNo --;		// get the last packet SeqNo
	PACKET_INFO_ITEM & LastItem = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
	assert( Packet0Item.m_nFullDataLen == (nPacketCount-1) * m_nMTUSize + LastItem.m_nDataLen );
#endif //_DEBUG

	return true;
}

//--------------------------------------------------------------
/** CYJ 2015-02-09
 *
 *	Is SeqNo valid, in the valid range
 *
 * @param [in]	wSeqNo			Sequence No
 *
 * @return		true			valid, in the range
 *				false			not valid
 */
bool CMyRUDP_PacketMgr::IsSeqNoValid( uint16_t wSeqNo )
{
	// ___________[T-----]__		[T, T+255]
	// --]______________[T--		[T,0xFFFF] or [ 0, 255 + T ]		; 255-(0xFFFF-T) = 255 + T - 0xFFFF = 255+ + T - 0x10000 - 1
	// e.g. T = 0xFFFF  ==> 0xFFFF or 0 - ( 255+0xFFFF ) => [0,254)
	// FF00 => FF00 ~ FFFF
	// FF01 => [ FF01 - FFFF ] or [0,0]
	if( m_wTailSeqNo <= 0xFF00 )
		return ( wSeqNo >= m_wTailSeqNo && wSeqNo <= ( m_wTailSeqNo + 0xFF ) );
	else
		return ( wSeqNo >= m_wTailSeqNo || wSeqNo <= (uint16_t)( 0xFF + m_wTailSeqNo ) );
}

//--------------------------------------------------------------
/** CYJ 2015-02-09
 *
 *	Copy the packet which sent at SeqNo, if the SeqNo has multiple sub data packet
 *	then the subsequence data packet will also be freeed
 *
 * @param [in]	pBuf					output data buffer
 * @param [in]	nBufSize				buffer size
 * @param [in]	bNotRemove				do not remove from the list
 *
 * @return		>0						data bytes has been copy
 *				=0						no data
 *				<0						data buffer too small, and -(RetVal) is the data buffer size need
 */
int CMyRUDP_PacketMgr::CopyData( uint8_t *pBuf, int nBufSize, bool bNotRemove )
{
	int nSeqNo = -1;
	int i;

	if( false == IsValid() )
		return 0;

	SkipDiscardItems();					// 2015.4.2 CYJ Add

	// verify has data or not
	pthread_mutex_lock( &m_SyncObj );
	if( false == m_aReadyPackets.empty() )
		nSeqNo = m_aReadyPackets.front();
	pthread_mutex_unlock( &m_SyncObj );

	if( nSeqNo < 0 )
		return 0;						// no data

#ifdef _DEBUG
	assert( nSeqNo == (int)m_wTailSeqNo );
#endif // _DEBUG

	nSeqNo &= MYRUDP_WINDOW_SIZE_MASK;

	int nFullDataLen = m_aPackets[ nSeqNo ].m_nFullDataLen;
	if( nBufSize < nFullDataLen )
		return -nFullDataLen;			// data buffer is too small, need more large buffer

	int nRetVal = 0;
	int nPacketCount = m_aPackets[ nSeqNo ].m_byPacketCount;
	for( i=0; i<nPacketCount; i++ )
	{
		int nIndex = ( nSeqNo + i ) & MYRUDP_WINDOW_SIZE_MASK;
		PACKET_INFO_ITEM & Item = m_aPackets[ nIndex ];
		memcpy( pBuf, Item.m_pBuf, Item.m_nDataLen );
		nRetVal += Item.m_nDataLen;
		pBuf += Item.m_nDataLen;

	#ifdef _DEBUG
		assert( Item.m_nDataLen && Item.m_byPacketCount );
		assert( nRetVal <= nFullDataLen );
		assert( Item.m_byPacketIndex == i );
		assert( Item.m_wSeqNo == (uint16_t)(m_wTailSeqNo + i) );
	#endif // _DEBUG
	}

#ifdef _DEBUG
	assert( nRetVal == nFullDataLen );
#endif // _DEBUG

	// peek the data only, not remove from the list
	if( false == bNotRemove )
		SkipData();

	return nRetVal;
}

//--------------------------------------------------------------
/** CYJ 2015-04-02
 *
 *	Modify Tail SeqNo if current TailSeqNo point to SKIP items
 */
void CMyRUDP_PacketMgr::SkipDiscardItems()
{
	// 2015.4.2 CYJ Add, to skip data packets that send failed
	for( int i=0; i<MYRUDP_WINDOW_SIZE; i++ )
	{
		int nIndex = m_wTailSeqNo & MYRUDP_WINDOW_SIZE_MASK;
		PACKET_INFO_ITEM & Item = m_aPackets[ nIndex ];
		if( 0 == (Item.m_byFlags & PACKET_INFO_FLAGS_SKIP) )
			break;
		Item.m_nFullDataLen = 0;		// complete packet data size
		Item.m_byPacketCount = 0;		// packet count of the data
		Item.m_byPacketIndex = 0;		// packet index of the data
		Item.m_byFlags = 0;				// flags

		m_wTailSeqNo ++;				// skip
	}
}

//--------------------------------------------------------------
/** CYJ 2015-02-14
 *
 *	Skip next packet data
 * @return		true					has more data
 *				false					no data
 */
bool CMyRUDP_PacketMgr::SkipData()
{
	int i;
	if( false == IsValid() )
		return false;

	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj );

	if( m_aReadyPackets.empty() )
		return false;				// no data

	int nSeqNo = m_aReadyPackets.front();

	nSeqNo &= MYRUDP_WINDOW_SIZE_MASK;
	int nPacketCount = m_aPackets[ nSeqNo ].m_byPacketCount;

	// free buffer
	for( i=0; i<nPacketCount; i++ )
	{
		int nIndex = ( nSeqNo + i ) & MYRUDP_WINDOW_SIZE_MASK;
		PACKET_INFO_ITEM & Item = m_aPackets[ nIndex ];
		Item.m_nFullDataLen = 0;		// complete packet data size
		Item.m_byPacketCount = 0;		// packet count of the data
		Item.m_byPacketIndex = 0;		// packet index of the data
		Item.m_byFlags = 0;				// flags
	}

	// update tail SeqNo
	m_wTailSeqNo += (uint16_t)nPacketCount;
	m_aReadyPackets.pop_front();

	SkipDiscardItems();					// 2015.4.2 CYJ Add

#ifdef _DEBUG
	int nTailSeqNo = m_wTailSeqNo & MYRUDP_WINDOW_SIZE_MASK;
	if( m_aPackets[nTailSeqNo].m_byPacketCount )
		assert( m_aPackets[nTailSeqNo].m_wSeqNo == m_wTailSeqNo );
	assert( 0 == m_aPackets[nTailSeqNo].m_byPacketIndex );			// next packet must be the 1st packet
#endif // _DEBUG

	return !m_aReadyPackets.empty();
}

//--------------------------------------------------------------
/** CYJ 2015-02-09
 *
 *	Get next packet's data length
 *
 * @return		0						no data
 *				>0						next data packet's data length
 */
int CMyRUDP_PacketMgr::GetNextPacketDataLen()
{
	CMy_pthread_Mutex_Locker SyncObj( &m_SyncObj );

	SkipDiscardItems();					// 2015.4.2 CYJ Add

	if( m_aReadyPackets.empty() )
		return 0;

	SkipDiscardItems();					// 2015.4.2 CYJ Add

	int nSeqNo = m_aReadyPackets.front();
#ifdef _DEBUG
	assert( nSeqNo == (int)m_wTailSeqNo );
#endif // _DEBUG

	nSeqNo &= MYRUDP_WINDOW_SIZE_MASK;

	return m_aPackets[ nSeqNo ].m_nFullDataLen;
}

//--------------------------------------------------------------
/** CYJ 2015-02-23
 *
 *	Has data to be read
 *
 * @return	true						has data
 *			false						no data
 */
bool CMyRUDP_PacketMgr::HasDataToBeRead()
{
	return( false == m_aReadyPackets.empty() );
}

#ifdef _DEBUG
void CMyRUDP_PacketMgr::DebugDump()
{
	CMyRUDP_fprintf_SyncHelper	fprintf_Sync;

	fprintf( stderr, "----------------------- IN DATA MANAGER ----------------\n" );

	fprintf( stderr, "  TAIL_SEQ: 0x%04x\n", m_wTailSeqNo );


	for(int i=0; i<MYRUDP_WINDOW_SIZE; i++ )
	{
		PACKET_INFO_ITEM & Item = m_aPackets[ i & MYRUDP_WINDOW_SIZE_MASK ];
		fprintf( stderr, " [ %4d, 0x%04x, %03d.%03d ], ", Item.m_nDataLen, Item.m_wSeqNo, Item.m_byPacketCount, Item.m_byPacketIndex );

		if( (i&3) == 3 )
			fprintf( stderr, "\n" );
	}

	fprintf( stderr, "\n" );
}
#endif //_DEBUG

//--------------------------------------------------------------
/** CYJ 2015-02-23
 *
 *	Get Resend SeqNo by current SeqNo
 *
 * @param [in]	awSeqNo					output SeqNo
 * @param [in]	nCount					awSeqNo buffer count
 * @param [in]	bForce					force to get Resend
 *
 * @return		>0						SeqNo need to request resend
 *				=0						no SeqNo need resend
 */
int CMyRUDP_PacketMgr::GetResendSeqNo( uint16_t awSeqNo[], int nCount, bool bForce )
{
	if( false == bForce )
	{
return -1;
		if( m_nSeqNoExpectedSameTimes < 8 )
			return -1;
		if( m_wSeqNoExpected_Last == m_wSeqNoExpected )
			return -1;
	}

	m_wSeqNoExpected_Last = m_wSeqNoExpected;
	if( nCount > MYRUDP_MAX_ACK_SEQNO_COUNT/4 )
		nCount = MYRUDP_MAX_ACK_SEQNO_COUNT/4;

	awSeqNo[ 0 ] = m_wSeqNoExpected;
	int nRetVal = 1;
	uint16_t wSeqNo = m_wSeqNoExpected + 1;
	for(int i=0; nRetVal<nCount && i<MYRUDP_MAX_ACK_SEQNO_COUNT/2; i++, wSeqNo ++ )
	{
		PACKET_INFO_ITEM & Item = m_aPackets[ wSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
		if( Item.m_byFlags & PACKET_INFO_FLAGS_COMMIT )
			break;				// no data
		if( Item.m_nDataLen && Item.m_byPacketCount )
			continue;
        awSeqNo[ nRetVal ] = wSeqNo;
        nRetVal ++;
	}
	return nRetVal;
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
bool CMyRUDP_PacketMgr::IsSeqNoInRange( uint16_t wSeqNo, uint16_t wHeadSeqNo, uint16_t wTailSeqNo )
{
	// [____________T----H___]
	// [----H__________T-----]
	if( wTailSeqNo <= wHeadSeqNo )
		return ( wSeqNo >= wTailSeqNo && wSeqNo <= wHeadSeqNo );
	else
		return ( wSeqNo >= wTailSeqNo || wSeqNo <= wHeadSeqNo );
}

//--------------------------------------------------------------
/** CYJ 2015-04-02
 *
 *	On data block has been skiped
 *
 * @param [in]	wHeadSeqNo				Head SeqNo to be skipped
 * @param [in]	wTailSeqNo				Head SeqNo to be skipped
 */
void CMyRUDP_PacketMgr::OnSkipData( uint16_t wHeadSeqNo, uint16_t wTailSeqNo )
{
	wHeadSeqNo --;

	while( wHeadSeqNo != wTailSeqNo )
	{
		if( IsSeqNoValid( wTailSeqNo ) )
		{
			PACKET_INFO_ITEM & Item = m_aPackets[ wTailSeqNo & MYRUDP_WINDOW_SIZE_MASK ];
			Item.m_byFlags |= PACKET_INFO_FLAGS_SKIP;	// to be skip
			Item.m_nFullDataLen = 0;		// complete packet data size
			Item.m_byPacketCount = 0;		// packet count of the data
			Item.m_byPacketIndex = 0;		// packet index of the data
		}
		wTailSeqNo --;
	}
}

#ifdef __MYRUDP_USE_OPENSSL__
//--------------------------------------------------------------
/** CYJ 2015-04-19
 *
 *	Set data decryption AES Key
 *
 * @param [in]	abyAESKey			48 bytes AES key
 */
void CMyRUDP_PacketMgr::SetDataEncryptionAESKey( uint8_t abyAESKey[] )
{
	m_pbyDataEncryptAESKey = abyAESKey;				// 48 bytes
}

//--------------------------------------------------------------
/** CYJ 2015-04-19
 *
 *	Decrypt data
 *
 * @param [in]	pBuf				packet data
 * @param [in]	nLen				packet data length
 */
void CMyRUDP_PacketMgr::DoDecryptData( const uint8_t * pBuf, int nLen, uint8_t * pOutBuf )
{
	unsigned char abyIV[16];
	memcpy( abyIV, m_pbyDataEncryptAESKey+32, 16 );
	abyIV[6] = pBuf[11];
	abyIV[7] = pBuf[12];
	uint8_t * pbyAESKey = m_pbyDataEncryptAESKey + ( pBuf[12] & 0xF );

	AES_KEY aes;
	AES_set_decrypt_key( pbyAESKey, 256, &aes);

	int nEncryptedDataLen = nLen - MY_RUDP_HEADER_LEN;
#ifdef _DEBUG
	assert( 0 == (nEncryptedDataLen&0xF) );
#endif //_DEBUG
	AES_cbc_encrypt( pBuf + MY_RUDP_HEADER_LEN, pOutBuf, nEncryptedDataLen, &aes, abyIV, 0 );	// do decrypt
}
#endif //#ifdef __MYRUDP_USE_OPENSSL__

