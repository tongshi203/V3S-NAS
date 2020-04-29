/***********************************************************
 *
 *	My RUDP packet builder
 *
 *	Chen Yongjian @ Zhoi
 *	2015.1.27 @ Xi'an
 *
 *
 ***********************************************************/

#include <stdafx.h>
#include <mydatatype.h>
#include <stdint.h>
#include <crc.h>
#include <assert.h>

#include "myrudp_packbuilder.h"
#include "myrudp_onepeerobj.h"

////////////////////////////////////////////////////////////
// 2015.4.19 CYJ Modify, payload data length must be 16*n
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

//---------------------------------------------------------
CMyRUDP_HeaderBuilder::CMyRUDP_HeaderBuilder( unsigned char * pBuf, CMyRUDP_OnePeerObject *pPeerObj, bool bReset )
 : CMyRUDP_HeaderReader( pBuf )
{
#ifdef _DEBUG
	assert( pBuf && pPeerObj );
#endif // _DEBUG

	m_pPeerObj = pPeerObj;

	if( bReset )
	{
		memset( m_pbyHeader, 0, MY_RUDP_HEADER_LEN );
		m_pbyHeader[0] = MYRUDP_MAGIC_ID_HI;
		m_pbyHeader[1] = MYRUDP_MAGIC_ID_LOW;
		SetMTUIndex( pPeerObj->GetMTUSizeIndex() );
		SetSessionID( pPeerObj->GetSessionID() + 1 );
	}
}

//---------------------------------------------------------
CMyRUDP_HeaderBuilder::~CMyRUDP_HeaderBuilder()
{
}

//--------------------------------------------------------------
/** CYJ 2015-01-27
 *
 *	Set Command value
 *
 * @param [in]	byCommand			command value
 */
void CMyRUDP_HeaderBuilder::SetCommand( uint8_t byCommand )
{
	m_pbyHeader[2] &= 0xE0;
	m_pbyHeader[2] |= ( byCommand & 0x1F );
}

//--------------------------------------------------------------
/** CYJ 2015-01-27
 *
 *	Set retransmit flag on
 */
void CMyRUDP_HeaderBuilder::SetRetransmitFlags( bool bUpdateCRC )
{
	m_pbyHeader[3] |= 0x80;		// set retransmit flag on

	if( bUpdateCRC )
		UpdateCRC();
}

//--------------------------------------------------------------
/** CYJ 2015-01-27
 *
 *	Set Session ID
 *
 * @param [in]	dwSessionID			session ID
 */
void CMyRUDP_HeaderBuilder::SetSessionID( uint32_t dwSessionID )
{
	for(int i=6; i>=4; i--)
	{
		m_pbyHeader[i] = (uint8_t) dwSessionID;
		dwSessionID >>= 8;
	}
}

//--------------------------------------------------------------
/** CYJ 2015-01-27
 *
 *	Set SessionTagID
 *
 * @param [in]	dwSessionTagID		session tag ID that has been encrypted
 */
void CMyRUDP_HeaderBuilder::SetSessionTagID( uint32_t dwSessionTagID )
{
	for(int i=10; i>=7; i--)
	{
		m_pbyHeader[i] = (uint8_t) dwSessionTagID;
		dwSessionTagID >>= 8;
	}
}

//--------------------------------------------------------------
/** CYJ 2015-01-27
 *
 *	Set SeqNo
 *
 * @param [in]	wSeqNo				sequence No
 */
void CMyRUDP_HeaderBuilder::SetSeqNo( uint16_t wSeqNo )
{
	m_pbyHeader[11] = (uint8_t) ( wSeqNo >> 8 );
	m_pbyHeader[12] = (uint8_t) wSeqNo;
}

//--------------------------------------------------------------
/** CYJ 2015-01-27
 *
 *	Set Payload data packet count
 *
 * @param [in]	byCount				packet count
 */
void CMyRUDP_HeaderBuilder::SetPacketCount( uint8_t byCount )
{
	byCount &= 0x7F;
	m_pbyHeader[13] = byCount;
}

//--------------------------------------------------------------
/** CYJ 2015-01-27
 *
 *	Set Packet Index
 *
 * @param [in]	byIndex				packet index
 */
void CMyRUDP_HeaderBuilder::SetPacketIndex( uint8_t byIndex )
{
	byIndex &= 0x7F;
	m_pbyHeader[14] = byIndex;
}

//--------------------------------------------------------------
/** CYJ 2015-01-27
 *
 *	Set MTU Size index
 *
 * @param [in]	byMTUIndex			MTU Index
 */
void CMyRUDP_HeaderBuilder::SetMTUIndex( uint8_t byMTUIndex )
{
	m_pbyHeader[15] &= 0x1F;
	m_pbyHeader[15] |= ( byMTUIndex << 5 );
}

//--------------------------------------------------------------
/** CYJ 2015-01-27
 *
 *	Set Payload data padding length
 *
 * @param [in]	byPaddingLen		padding data length
 */
void CMyRUDP_HeaderBuilder::SetPaddingDataLen( uint8_t byPaddingLen )
{
	m_pbyHeader[15] &= 0xE0;
	m_pbyHeader[15] |= ( byPaddingLen & 0x1F );
}

//--------------------------------------------------------------
/** CYJ 2015-01-27
 *
 *	Set SesstionTagID and CRC16
 * @note
 *		all other parameters should have been set
 */
void CMyRUDP_HeaderBuilder::Commit()
{
	// encrypt sessionTag ID
    uint32_t dwSessionTagID = m_pPeerObj->GetSesstionTagID();
    uint32_t dwSeqNo = GetSeqNo();
    if( dwSeqNo )
	{
		// SessionTagID =  ( OrgSessionTagID ^ s_adwXorData[SeqNo] ) + (SeqNo << 10) + (PacketIndex << 4) + DataPaddLen
		uint32_t dwXorData = 0;
		uint8_t *pXorData = m_pPeerObj->GetXorData();
		for( int i=0; i<4; i++ )
		{
			dwXorData <<= 8;
			dwXorData |= pXorData[ (dwSeqNo + i) & 0x1F ];
		}
		dwSessionTagID ^= dwXorData;
		dwSessionTagID += ( dwSeqNo << 10 ) + ( GetPackteIndex() << 4 ) + GetPaddingDataLen();
	}

	// set session tag ID
	SetSessionTagID( dwSessionTagID );

	// calculate header CRC-16
	UpdateCRC();
}

//--------------------------------------------------------------
/** CYJ 2015-01-28
 *
 *	Update CRC, when set retransmit flag, then should update the CRC again
 */
void CMyRUDP_HeaderBuilder::UpdateCRC()
{
	uint16_t wCRC16 = CCRC::GetCRC16( MY_RUDP_HEADER_LEN-2, m_pbyHeader );
	m_pbyHeader[16] = (uint8_t)( wCRC16 );
	m_pbyHeader[17] = (uint8_t)( wCRC16 >> 8 );

#ifdef _DEBUG
	assert( 0 == CCRC::GetCRC16( MY_RUDP_HEADER_LEN, m_pbyHeader ) );
#endif // _DEBUG
}

