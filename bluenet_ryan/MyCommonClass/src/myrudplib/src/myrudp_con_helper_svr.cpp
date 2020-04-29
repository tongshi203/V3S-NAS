/*************************************************************************
 *
 *	My RUDP Response Packet builder
 *
 *	Chen Yongjian @ Zhoi
 *	2015.1.27 @ Xi'an
 *
 *************************************************************************/

#include <stdafx.h>
#include <mydatatype.h>
#include <crc.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

#include "myrudp_onepeerobj.h"
#include "myrudp_con_helper_svr.h"
#include "myrudp_packbuilder.h"
#include "myrudp_dbgprint.h"

#ifdef __MYRUDP_USE_OPENSSL__
	#include "myrudp_openssl_svr_helper.h"
#endif // #ifdef __MYRUDP_USE_OPENSSL__

////////////////////////////////////////////////////////////////////////
static const unsigned char 	s_abyReqConnUUID[16] = { 0x8c,0xd9,0x7c,0x3d,0xaf,0xe7,0x41,0xb4,0x9e,0x91,0xf8,0x4a,0xb8,0x3e,0x57,0xc8 };
static const unsigned char 	s_abyRspConnUUID[16] = { 0xa6,0xc5,0xed,0x4f,0xda,0x08,0x47,0x21,0xb0,0x48,0xf1,0x1f,0x99,0x77,0x21,0x05 };
static const unsigned char 	s_abySynConnUUID[16] = { 0xa7,0xd3,0x02,0x0d,0x38,0xa7,0x41,0xae,0x99,0x4e,0x59,0x09,0x92,0x16,0x83,0x6a };
static const unsigned char 	s_abySyn2ConnUUID[16]= { 0x02,0xa6,0x40,0x91,0x48,0xdc,0x49,0x3b,0x92,0xa5,0x00,0x09,0x72,0xd6,0xc2,0x7c };

#ifdef __MYRUDP_USE_OPENSSL__
	extern CMyRUDPOpenSSLServerHelper	g_MyRUDPOpenSSLServerHelper;
#endif // #ifdef __MYRUDP_USE_OPENSSL__

////////////////////////////////////////////////////////////////////////
#define PUT_WORD_TO_BUF( pBuf, wData )		{ pBuf[0]=(uint8_t)((wData)>>8); pBuf[1]=(uint8_t)(wData); }
#define PUT_DWORD_TO_BUF_BE( pBuf, dwData )	for(int i=3; i>=0; i--){ pBuf[i]=(uint8_t)dwData; dwData>>=8; }
#define PUT_DWORD_TO_BUF_LE( pBuf, dwData )	for(int i=0; i<4; i++){ pBuf[i]=(uint8_t)dwData; dwData>>=8; }

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

////////////////////////////////////////////////////////////////////////
CMyRUDP_Connection_Svr_Helper::CMyRUDP_Connection_Svr_Helper( CMyRUDP_OnePeerObject * pPeerObj )
{
	m_nState = MYRUDP_CONNECTION_STATE_WAIT_REQ;
	m_pPeerObj = pPeerObj;
	m_nWaitSyncTimer = MYRUDP_CONNECTION_WAIT_SYNC_TIMES;

	m_nRefCount = 1;			// 2015.4.18 CYJ Add

#ifdef _DEBUG
	assert( pPeerObj );
#endif // _DEBUG
}


CMyRUDP_Connection_Svr_Helper::~CMyRUDP_Connection_Svr_Helper()
{
#ifdef __MYRUDP_USE_OPENSSL__
	m_pPeerObj->GetThreadPoolObj()->RemoveTask( static_cast<CMyThreadPoolTask*>(this) );
#endif //_DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-01-28
 *
 *	Is Request connection command
 *
 * @param [in] 	pBuf				command buffer
 * @param [in]	nLen				command data length
 *
 * @return		true				is req data
 *				false				not a req data
 */
bool CMyRUDP_Connection_Svr_Helper::IsReqCommandData( const unsigned char *pBuf, int nLen )
{
	// ReqUUID(16) + XorData(32) + MTU(1) + ECCPubKeyLen(2) + CRC32(4)
	const int nFixHeaderLen = MY_RUDP_HEADER_LEN + 16 + 32 + 1 + 2 + 4;
	if( NULL == pBuf || nLen < nFixHeaderLen )
		return false;

	const unsigned char * pbyReqData = pBuf + MY_RUDP_HEADER_LEN;
	if( memcmp( pbyReqData, s_abyReqConnUUID, 16 ) )
		return false;				// UUID not same

#ifdef __MYRUDP_USE_OPENSSL__
	// check PubKey length
	uint16_t wPubKeyLen = pbyReqData[ 49 ] * 0x100 + pbyReqData[ 50 ];
	if( 0 == wPubKeyLen )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "ECC Public key is required.\n" );
	#endif //_DEBUG
		return false;
	}
	if( nLen < nFixHeaderLen + wPubKeyLen )
	{
	#ifdef _DEBUG
		MyRUDP_fprintf( "Request header length (%d) should >= %d bytes.\n", nLen, nFixHeaderLen + wPubKeyLen );
	#endif //_DEBUG
		return false;
	}
#endif // __MYRUDP_USE_OPENSSL__

	CMyRUDP_HeaderReader HeaderRead( pBuf );
	if( HeaderRead.GetCommand() != MYRUDP_CMD_REQ )
		return false;
	if( HeaderRead.GetSessionID() || HeaderRead.GetSessionTagID() )
		return false;
	if( HeaderRead.GetSeqNo() || HeaderRead.GetPackteCount() || HeaderRead.GetPackteIndex() )
		return false;

	if( CCRC::GetCRC32( nLen, (uint8_t*)pBuf ) )
		return false;				// should pass CRC

	return true;
}

//--------------------------------------------------------------
/** CYJ 2015-01-28
 *
 *	Is sync command data
 *
 * @param [in] 	pBuf				command buffer
 * @param [in]	nLen				command data length
 *
 * @return		true				is syn data
 *				false				not a syn data
 */
bool CMyRUDP_Connection_Svr_Helper::IsSynCommandData( const unsigned char *pBuf, int nLen )
{
	// UUID(16) + CRC(4)
	if( NULL == pBuf || nLen < MY_RUDP_HEADER_LEN + 16 + 4 )
		return false;

	const unsigned char * pbySynData = pBuf + MY_RUDP_HEADER_LEN;
	if( memcmp( pbySynData, s_abySynConnUUID, 16 ) )
		return false;				// UUID not same

	CMyRUDP_HeaderReader HeaderRead( pBuf );
	if( HeaderRead.GetCommand() != MYRUDP_CMD_SYN )
		return false;
	if( HeaderRead.GetSeqNo() || HeaderRead.GetPackteCount() || HeaderRead.GetPackteIndex() )
		return false;

	if( CCRC::GetCRC32( nLen, (uint8_t*)pBuf ) )
		return false;				// should pass CRC

	return true;
}

//--------------------------------------------------------------
/** CYJ 2015-01-28
 *
 *	On get request command data, save it and build RSP data
 *
 * @param [in]	pBuf				Req Data buffer
 * @param [in]	nLen				Req Data Length
 *
 * @return		0					succ
 *				other				failed
 */
int CMyRUDP_Connection_Svr_Helper::OnReqData( const unsigned char *pBuf, int nLen )
{
#ifdef _DEBUG
//	DbgDumpData( "--- REQ data", pBuf, nLen );
#endif // _DEBUG

#ifdef __MYRUDP_USE_OPENSSL__
	CSingleLock SyncObj( &m_SyncObj, true );
#endif // #ifdef __MYRUDP_USE_OPENSSL__

	m_nState = MYRUDP_CONNECTION_STATE_WAIT_REQ;
	if( false == m_ReqInData.Copy( pBuf, nLen ) )
		return ENOMEM;

#ifdef __MYRUDP_USE_OPENSSL__
	// use ThreadTask
	return m_pPeerObj->GetThreadPoolObj()->AddTask( static_cast<CMyThreadPoolTask*>(this), true );
#else
	int nRetVal = BuildResponseData();
	if( nRetVal )
		return nRetVal;

	m_nWaitSyncTimer = MYRUDP_CONNECTION_WAIT_SYNC_TIMES;
	m_nState = MYRUDP_CONNECTION_STATE_WAIT_SYN;

	return SendOutResponseData();
#endif // __MYRUDP_USE_OPENSSL__
}

//--------------------------------------------------------------
/** CYJ 2015-01-28
 *
 *	Build Response data
 *
 * @return		0					succ
 *				other				failed
 *
 * @note
 *	Reponse data packet format:
 *	[ RUDP_Header ][  UUID  ][ MTU ][ ECC PubKeyLen ][ PubKey Data] [ CRC32_XorData ] [ CRC32 ]
 *         18	       16       1		  2					N			4				  4
 */
int CMyRUDP_Connection_Svr_Helper::BuildResponseData()
{
#ifdef __MYRUDP_USE_OPENSSL__
	// ReqUUID(16) + XorData(32) + MTU(1) + ECCPubKeyLen(2) + Data(N)+ CRC32(4)
	uint8_t * pInReqData = m_ReqInData.GetBuffer();
	pInReqData += MY_RUDP_HEADER_LEN;		// skip header length

	uint8_t * pbyXorData = pInReqData + 16;	// skip UUID
	uint16_t wPubKeyLen = pInReqData[49] * 0x100 + pInReqData[50];
 #ifdef _DEBUG
	assert( wPubKeyLen >= 17 );
	assert( (wPubKeyLen & 0xF) == 1 );
 #endif//_DEBUG
	uint8_t * pbyPubKeyData = pInReqData + 51;

	uint8_t byECCType;
	std::vector<uint8_t> aAESKey = g_MyRUDPOpenSSLServerHelper.GetAESKey( pbyPubKeyData, wPubKeyLen, pbyXorData, byECCType );
	std::vector<uint8_t> aPubKey = g_MyRUDPOpenSSLServerHelper.GetEncryptedPubKey( byECCType, pbyXorData, m_pPeerObj->GetSessionID()+1 );
#ifdef _DEBUG
	assert( aAESKey.size() );
	assert( aPubKey.size() );
#endif //_DEBUG
	if( aAESKey.empty() || aPubKey.empty() )
		return EINVAL;

	m_pPeerObj->SetDataEncryptionAESKey( aAESKey.data(), aAESKey.size() );

#endif // #ifdef __MYRUDP_USE_OPENSSL__

	// output response data length
	// FIXME, how length should be
	// 2015.5.7 CYJ Add CRC32_XorData
	int nResponeDataPacketLen = MY_RUDP_HEADER_LEN + 16 + 1 + 2 + 4 + 4;

#ifdef __MYRUDP_USE_OPENSSL__
	nResponeDataPacketLen += aPubKey.size();
#endif // #ifdef __MYRUDP_USE_OPENSSL__

	if( false == m_RspOutData.Allocate( nResponeDataPacketLen + 16 ) )		// 16 bytes to keep space
		return ENOMEM;

	uint8_t * pBuf = m_RspOutData.GetBuffer();
	memset( pBuf, 0, nResponeDataPacketLen + 4 );

	CMyRUDP_HeaderBuilder HeaderBuilder( pBuf, m_pPeerObj, true );
	HeaderBuilder.SetCommand( MYRUDP_CMD_RSP );
	HeaderBuilder.SetSeqNo( 0 );
	HeaderBuilder.Commit();

	uint8_t * pTmpBuf = pBuf + MY_RUDP_HEADER_LEN;
	memcpy( pTmpBuf, s_abyRspConnUUID, 16 );
	pTmpBuf += 16;
	*pTmpBuf = m_pPeerObj->GetMTUSizeIndex();
	pTmpBuf ++;

#ifdef __MYRUDP_USE_OPENSSL__
	wPubKeyLen = aPubKey.size();
	PUT_WORD_TO_BUF( pTmpBuf, wPubKeyLen );
	pTmpBuf += 2;
	memcpy( pTmpBuf, aPubKey.data(), wPubKeyLen );
	pTmpBuf += wPubKeyLen;
#else
	PUT_WORD_TO_BUF( pTmpBuf, 0 );
	pTmpBuf += 2;
#endif // #ifdef __MYRUDP_USE_OPENSSL__

	// 2015.5.7 CYJ Add CRC32_XorData
	uint32_t dwCRC32_XorData = CCRC::GetCRC32( 32, pbyXorData );
	PUT_DWORD_TO_BUF_BE( pTmpBuf, dwCRC32_XorData );
	pTmpBuf += 4;

	uint32_t dwCRC32 = CCRC::GetCRC32( nResponeDataPacketLen-4, (uint8_t*)pBuf );
	PUT_DWORD_TO_BUF_LE( pTmpBuf, dwCRC32 );
	m_RspOutData.SetDataLen( nResponeDataPacketLen );

#if 0 && defined(_DEBUG)
//	DbgDumpData( "RSP Data", pBuf, nResponeDataPacketLen );

	assert( 0 == pBuf[nResponeDataPacketLen]   );
	assert( 0 == pBuf[nResponeDataPacketLen+1] );
	assert( 0 == pBuf[nResponeDataPacketLen+2] );
	assert( 0 == pBuf[nResponeDataPacketLen+3] );
	assert( 0 == CCRC::GetCRC32( nResponeDataPacketLen, (uint8_t*)pBuf ) );
#endif // _DEBUG

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-01-28
 *
 *	Send out response data
 *
 * @return		0					succ
 *				other				error code
 */
int CMyRUDP_Connection_Svr_Helper::SendOutResponseData()
{
#ifdef _DEBUG
//	DbgDumpData( "--- Send", m_RspOutData.GetBuffer(), m_RspOutData.GetDataLen() );
#endif // _DEBUG

	return m_pPeerObj->SendRawDataToPeer( m_RspOutData.GetBuffer(), m_RspOutData.GetDataLen() );
}

//--------------------------------------------------------------
/** CYJ 2015-01-28
 *
 *	set re-transmit flag On
 */
void CMyRUDP_Connection_Svr_Helper::UpdateResponseData_RetransmitOn()
{
	uint8_t * pBuf = m_RspOutData.GetBuffer();

	CMyRUDP_HeaderBuilder HeaderBuilder( pBuf, m_pPeerObj, false );
	HeaderBuilder.SetRetransmitFlags( true );

	uint32_t dwCRC32 = CCRC::GetCRC32( m_RspOutData.GetDataLen()-4, (uint8_t*)pBuf );
	pBuf += ( m_RspOutData.GetDataLen()-4 );
	PUT_DWORD_TO_BUF_LE( pBuf, dwCRC32 );

#ifdef _DEBUG
	assert( 0 == CCRC::GetCRC32( m_RspOutData.GetDataLen(), m_RspOutData.GetBuffer() ) );
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-01-28
 *
 *	On Timer to check should re-send RSP data or not
 *
 * @param [in]	tNow				current time tick
 *
 * @return		0					timeout, waiting sync time out, should abort the connection
 *				other				continue waiting
 * @note
 *		it'll send out Response data per second until get the Syn packet
 */
int CMyRUDP_Connection_Svr_Helper::OnWaitSynTimer( time_t tNow )
{
#ifdef __MYRUDP_USE_OPENSSL__
	CSingleLock SyncObj( &m_SyncObj, true );
#endif // #ifdef __MYRUDP_USE_OPENSSL__

	if( MYRUDP_CONNECTION_STATE_WAIT_REQ == m_nState )
		return 1;		// waiting for Req
	else if( MYRUDP_CONNECTION_STATE_WAIT_SYN == m_nState )
	{
		if( 0 == m_nWaitSyncTimer )
			return 0;

		if( MYRUDP_CONNECTION_WAIT_SYNC_TIMES == m_nWaitSyncTimer )
			UpdateResponseData_RetransmitOn();
		m_nWaitSyncTimer --;

		// resend after 2 seconds
		if( m_nWaitSyncTimer < (MYRUDP_CONNECTION_WAIT_SYNC_TIMES-2) )
			SendOutResponseData();		// send out Response data again, to avoid the Response data is lose

		return m_nWaitSyncTimer + 1;
	}

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-01-28
 *
 *	On get sync data
 *
 * @param [in]	pBuf				Sync data buffer
 * @param [in]	nLen				Sync data length
 *
 * @return		0					succ
 *				other				failed
 */
int CMyRUDP_Connection_Svr_Helper::OnSynData( const unsigned char *pBuf, int nLen )
{
#ifdef _DEBUG
//	DbgDumpData( "--- SYN data", pBuf, nLen );
#endif // _DEBUG

#ifdef __MYRUDP_USE_OPENSSL__
	CSingleLock SyncObj( &m_SyncObj, true );
#endif // #ifdef __MYRUDP_USE_OPENSSL__

	if( MYRUDP_CONNECTION_STATE_WAIT_SYN != m_nState )
		return EPERM;
	if( false == m_SyncInData.Copy( pBuf, nLen ) )
		return ENOMEM;

#ifdef _DEBUG
//	MyRUDP_fprintf( "Get SYN data, change state to Connected.\n" );
#endif // _DEBUG

	// 2015.5.4 CYJ Add
	BuildAndSendSyn2( m_pPeerObj );

	m_nState = MYRUDP_CONNECTION_STATE_CONNECTED;

	return 0;
}

#ifdef __MYRUDP_USE_OPENSSL__
//--------------------------------------------------------------
void CMyRUDP_Connection_Svr_Helper::RunTask()
{
	CSingleLock SyncObj( &m_SyncObj, true );

	if( MYRUDP_CONNECTION_STATE_WAIT_REQ != m_nState )
		return;

	int nRetVal = BuildResponseData();
	if( nRetVal )
		return;

	m_nWaitSyncTimer = MYRUDP_CONNECTION_WAIT_SYNC_TIMES;
	m_nState = MYRUDP_CONNECTION_STATE_WAIT_SYN;

	SendOutResponseData();
}

//--------------------------------------------------------------
/** CYJ 2015-04-19
 *
 *	Abort
 */
void CMyRUDP_Connection_Svr_Helper::Abort()
{
	CSingleLock SyncObj( &m_SyncObj, true );

	m_nState = MYRUDP_CONNECTION_STATE_WAIT_SYN;
}

#endif // #ifdef __MYRUDP_USE_OPENSSL__

//--------------------------------------------------------------
// live reference counter
int CMyRUDP_Connection_Svr_Helper::AddRef()
{
	return InterlockedIncrement( &m_nRefCount );
}

//--------------------------------------------------------------
int CMyRUDP_Connection_Svr_Helper::Release()
{
	int nRetVal = InterlockedDecrement( &m_nRefCount );
#ifdef _DEBUG
	assert( nRetVal>= 0 );
#endif // _DEBUG
	if( nRetVal )
		return nRetVal;
	delete this;
	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-05-04
 *
 *	Build and send Sync2 command
 *
 * @param [in]	pPeerObj			Peer Object
 */
void CMyRUDP_Connection_Svr_Helper::BuildAndSendSyn2( CMyRUDP_OnePeerObject * pPeerObj )
{
	// [ CommonHeader ][ UUID ][ CRC32 ]
	#define MYRUDP_SYNC2_DATA_LEN  ( MY_RUDP_HEADER_LEN + 16 + 4 )

	uint8_t abySync2Data[ MYRUDP_SYNC2_DATA_LEN  ];
	uint8_t * pBuf = abySync2Data;

	CMyRUDP_HeaderBuilder HeaderBuilder( pBuf, pPeerObj, true );
	HeaderBuilder.SetCommand( MYRUDP_CMD_SYN_2 );
	HeaderBuilder.SetSeqNo( 0 );
	HeaderBuilder.Commit();

	uint8_t * pTmpBuf = pBuf + MY_RUDP_HEADER_LEN;
	memcpy( pTmpBuf, s_abySyn2ConnUUID, 16 );
	pTmpBuf += 16;

	uint32_t dwCRC32 = CCRC::GetCRC32( MYRUDP_SYNC2_DATA_LEN-4, (uint8_t*)pBuf );
	PUT_DWORD_TO_BUF_LE( pTmpBuf, dwCRC32 );

	pPeerObj->SendRawDataToPeer( pBuf, MYRUDP_SYNC2_DATA_LEN );
}
