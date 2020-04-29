/***********************************************************
 *
 *	My RUDP packet builder
 *
 *	Chen Yongjian @ Zhoi
 *	2015.1.27 @ Xi'an
 *
 *
 ***********************************************************/

#ifndef __MY_RUDP_PACK_BUILDER_H_20150127__
#define __MY_RUDP_PACK_BUILDER_H_20150127__

#include "myrudp_cmd_value.h"

class CMyRUDP_OnePeerObject;

class CMyRUDP_HeaderBuilder : public CMyRUDP_HeaderReader
{
public:
	CMyRUDP_HeaderBuilder( unsigned char * pBuf, CMyRUDP_OnePeerObject *pPeerObj, bool bReset=true );
	virtual ~CMyRUDP_HeaderBuilder();

	void SetCommand( uint8_t byCommand );
	void SetRetransmitFlags( bool bUpdateCRC = true );
	void SetSeqNo( uint16_t wSeqNo );
	void SetPacketCount( uint8_t byCount );
	void SetPacketIndex( uint8_t byIndex );
	void SetPaddingDataLen( uint8_t byPaddingLen );
	uint8_t * GetPayloadDataBuf() { return (uint8_t *)m_pbyHeader + MY_RUDP_HEADER_LEN; }
	void Commit();
	void UpdateCRC();

protected:
	void SetSessionID( uint32_t dwSessionID );
	void SetSessionTagID( uint32_t dwSessionTagID );
	void SetMTUIndex( uint8_t byMTUIndex );

protected:
	CMyRUDP_OnePeerObject * m_pPeerObj;
};

#endif // __MY_RUDP_PACK_BUILDER_H_20150127__

