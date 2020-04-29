/*********************************************************************
 *
 *	My RUDP command
 *
 *	Chen Yongjian @ Zhoi
 *	2015.1.17 @ Xi'an
 *
 *********************************************************************/

#ifndef __MY_RUDP_COMMAND_VALUE_H_20150117__
#define __MY_RUDP_COMMAND_VALUE_H_20150117__

#include <stdint.h>

#define MYRUDP_CMD_MAX_VERSION		0

#define MYRUDP_MAGIC_ID_HI		0x5A			// 'Z'
#define MYRUDP_MAGIC_ID_LOW		0x59			// 'Y'

#define MY_RUDP_HEADER_LEN		18

#define MYRUDP_WINDOW_SIZE			256			// must be n power of 2, (2^n)
#define MYRUDP_WINDOW_SIZE_MASK		( MYRUDP_WINDOW_SIZE - 1 )
#define MYRUDP_MAX_ACK_SEQNO_COUNT	16

#define MYRUDP_MAX_DATA_LENGTH		0x8000		// 32 KB

#define MYRUDP_MAX_PACKET_COUNT		127

typedef enum
{
	MYRUDP_CMD_NOP = 0,		// no operation, for keeping alive
	MYRUDP_CMD_REQ,			// request connecting
	MYRUDP_CMD_RSP,			// response for connecting
	MYRUDP_CMD_SYN,			// sync for connecting
	MYRUDP_CMD_DATA,		// sending data
	MYRUDP_CMD_ACK,			// sending ACK
	MYRUDP_CMD_RESEND,		// request resend data packet
	MYRUDP_CMD_SKIP,		// skip data since send timeout
	MYRUDP_CMD_CLOSE,		// close the connection
	MYRUDP_CMD_SYN_2,		// ACK of sync

	MYRUDP_CMD_MAX_VALUE,	// max command value
}MY_RUDP_COMMAND_VALUE_ENUM;

// payload data length must be 16*n
typedef enum
{
	MY_RUDP_MTU_SIZE_256 = 0,
	MY_RUDP_MTU_SIZE_512,
	MY_RUDP_MTU_SIZE_1024,
	MY_RUDP_MTU_SIZE_1392,
}MY_RUDP_MTU_SIZE_INDEX;

typedef enum
{
	MY_RUDP_ACK_MODE_ARRAY = 0,
	MY_RUDP_ACK_MODE_RANGE,
}MY_RUDP_ACK_MODE_ENUM;

uint16_t MyRUDP_GetMTUSizeByIndex( int nIndex );
uint8_t MyRUDP_GetMTUIndexBySize( int nSize );


////////////////////////////////////////////////////////////
class CMyRUDP_HeaderReader
{
public:
	CMyRUDP_HeaderReader( const unsigned char *pBuf )
	{
		m_pbyHeader = (unsigned char *)pBuf;
	}

	virtual ~CMyRUDP_HeaderReader(){}

	unsigned short GetMagicWord()const { return m_pbyHeader[0]*0x100 + m_pbyHeader[1]; }
	bool IsMagicWordOK()const { return MYRUDP_MAGIC_ID_HI == m_pbyHeader[0] && MYRUDP_MAGIC_ID_LOW == m_pbyHeader[1];  }
	unsigned char GetVersion()const { return m_pbyHeader[2] >> 5; }
	unsigned char GetCommand() const { return m_pbyHeader[2] & 0x1F; }
	unsigned char GetFlags() const { return m_pbyHeader[3]; }
	unsigned char IsRetransmitted()const { return m_pbyHeader[3] & 0x80;}
	uint32_t	  GetSessionID() const { return m_pbyHeader[4] * 0x10000L + m_pbyHeader[5] * 0x100 + m_pbyHeader[6]; }
	uint32_t	  GetSessionTagID() const { return (unsigned int)( m_pbyHeader[7] * 0x1000000L + m_pbyHeader[8] * 0x10000L + m_pbyHeader[9] * 0x100 + m_pbyHeader[10] ); }
	uint16_t	  GetSeqNo()const { return m_pbyHeader[11] * 0x100 + m_pbyHeader[12]; }
	uint8_t		  GetPackteCount()const { return m_pbyHeader[13] & 0x7F; }
	uint8_t		  GetPackteIndex()const { return m_pbyHeader[14] & 0x7F; }
	uint8_t		  GetMTUIndex()const { return m_pbyHeader[15] >> 5; }
	int			  GetMTUSize()const	{ return MyRUDP_GetMTUSizeByIndex( (m_pbyHeader[15] >> 5) ); }
	uint8_t		  GetPaddingDataLen()const { return m_pbyHeader[15] & 0x1F; }
	int			  GetActualPayloadDataLen( int nLen ) const { return nLen - (m_pbyHeader[15] & 0x1F); }

	static uint16_t GetSeqNo( const uint8_t * pBuf ){ return pBuf[11] * 0x100 + pBuf[12]; }

protected:
	unsigned char * m_pbyHeader;
};

#endif // __MY_RUDP_COMMAND_VALUE_H_20150117__
