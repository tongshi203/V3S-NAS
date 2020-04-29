/****************************************************************************
 *
 *	My RUDP sender packet manger
 *
 *	Chen Yongjian @ zhoi
 *	2015.2.11 @ Xi'an
 *
 ***************************************************************************/

#ifndef __MY_RUDP_PACKET_SENDER_H_20150210__
#define __MY_RUDP_PACKET_SENDER_H_20150210__

#include "myrudp_cmd_value.h"

#include <stdint.h>
#include <list>

#pragma pack( push, 8 )

class CMyRUDP_Packet_Sender
{
public:
	CMyRUDP_Packet_Sender();
	virtual ~CMyRUDP_Packet_Sender();

public:
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
    int Initialize( int nMTUSize, uint32_t dwSessionID, uint32_t dwSessionTagID, uint8_t * pXorData );

    //--------------------------------------------------------------
    /** CYJ 2015-02-12
     *
     *	Invalidate and free resource
     */
    void Invalidate();

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
    int Send( const uint8_t * pBuf, int nLen, void * pUserData, int nTimeOut );

    //--------------------------------------------------------------
    /** CYJ 2015-02-12
     *
     *	On timer to resend the packet that has not received ACK
     *
     * @param [in]	tNow				current time tick
     */
    void OnTimer( time_t tNow );

    //--------------------------------------------------------------
    /** CYJ 2015-02-26
     *
     *	Fast timer callback to send out delay packets
     */
    void OnFastTimer();

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
    void OnACK( const uint16_t awACKSeqNo[], int nACKCount, uint16_t wPeerRcvHeadSeqNo, uint16_t wPeerRevTailSeqNo );

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
    void OnACK( const uint16_t wHeadSeqNo, const uint16_t wTailSeqNo, uint16_t wPeerRcvHeadSeqNo, uint16_t wPeerRevTailSeqNo );

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
    void OnResend( const uint16_t awSeqNo[], int nCount, uint16_t wPeerRcvHeadSeqNo, uint16_t wPeerRevTailSeqNo );

    //--------------------------------------------------------------
    /** CYJ 2015-02-12
     *
     *	Send raw data to peer
     *
     * @param [in]	pBuf				data buffer to be send
     * @param [in]	nLen				data length
     *
     * @return		0					succ
     *				other				error
     */
    virtual int SendRawDataToPeer( const uint8_t * pBuf, int nLen ) = 0;

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
    virtual int SendDataPacketToPeer( const uint8_t * pBuf, int nLen ) = 0;

    //--------------------------------------------------------------
    /** CYJ 2015-02-26
     *
     *	Register Fast timer callback function
     */
    virtual void RegisterFastTimer() = 0;

    bool IsValid(){ return (NULL != m_pDataBuf) && m_nMTUSize; }


#ifdef _DEBUG
    void DebugDump();
#endif //_DEBUG

protected:
	//--------------------------------------------------------------
    /** CYJ 2015-02-15
     *
     *	Notify one packet has been send succ
     *
     * @param [in]	pUserData			user data
     */
    virtual void OnPacketHasBeenSendSucc( void * pUserData ) = 0;

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
    virtual void OnPacketSendFailed( uint16_t wHeadSeqNo, uint16_t wTailSeqNo, void * pUserData ) = 0;

    //--------------------------------------------------------------
    /** CYJ 2015-05-10
     *
     *	Force to send out N packets
     *
     * @param [in]	nCount				packet count
     */
    void ForceSendoutPacket( int nCount );

    //--------------------------------------------------------------
    /** CYJ 2015-05-22
     *
     *	On data received from peer
     */
    void OnDataReceivedFromPeer();

private:
	enum
	{
		PACKET_SEND_FLAGS_GET_ACK = 0x8000,		// Get ACK
		PACKET_SEND_FLAGS_RESEND = 1,			// Resend
		PACKET_SEND_FLAGS_SEND_DELAY = 2,		// send delay, since the peer has no buffer to receive the packet
		PACKET_SEND_FLAGS_SKIP = 4,				// bit2, skipped
		PACKET_SEND_FLAGS_MUST_SUCC = 8,		// must succ
		PACKET_SEND_FLAGS_HAS_BEEN_SENT = 16,	// has been send at least 1 time
	};
	enum
	{
		PACKET_RESEND_RETRY_TIMES = 40,			// max retry 20 times

		NOT_SEND_SECNDS_NOT_GET_DATA_FROM_PEER = 120,	// 发送数据2分钟后，但还没有收到对方的任何数据，对方可能退出或者网络异常，不再发送
	};

	typedef struct tagSEND_PACKET_USER_DATA
	{
		void *		m_pUserData;
		int			m_nDataLen;
		int			m_nPacketCount;
		uint16_t	m_wFirstSeqNo;
		uint16_t	m_wLastSeqNo;
	}SEND_PACKET_USER_DATA,*PSEND_PACKET_USER_DATA;

    typedef struct tagSEND_PACKET_ITEM
	{
		uint8_t	*	m_pBuf;					// pointer to buffer
		int			m_nDataLen;				// data length
		uint16_t	m_wSeqNo;				// SeqNo
		uint16_t	m_wFlags;				// Flags
		uint8_t		m_byMaxResendTimeout;
		uint8_t		m_byTimeOut;
		uint8_t		m_byResendCount;
		PSEND_PACKET_USER_DATA	m_pUserDataItem;
	}SEND_PACKET_ITEM,*PSEND_PACKET_ITEM;

protected:
	bool HasDataToBeSend(){ return m_wHeadSeqNo != m_wTailSeqNo; }
#ifdef __MYRUDP_USE_OPENSSL__
	void SetDataEncryptionAESKey( uint8_t abyAESKey[48] );
#endif // #ifdef __MYRUDP_USE_OPENSSL__

private:
	int GetAvailableBufCount();
	int BuildAndSendPackets( const uint8_t * pBuf, int nLen, void * pUserData, int nTimeOut );
	int BuildOnePacket( int nPacketCount, int nIndex, SEND_PACKET_ITEM & Item );
	void UpdateCRC( uint8_t * pBuf );
	void OnOneSeqNoACK( uint16_t wSeqNo );
	bool IsSeqNoInRange( uint16_t wSeqNo, uint16_t wHeadSeqNo, uint16_t wTailSeqNo );
	bool IsSeqNoValid( uint16_t wSeqNo );
	void RepaireACKByPeerRcvTailSeqNo( uint16_t wPeerRcvHeadSeqNo );
	void SetPeerExpectedSeqNo( uint16_t wSeqNo );
	void OnPacketSendTimeOut( SEND_PACKET_ITEM & Item );
	void RemoveUserDataItem( PSEND_PACKET_USER_DATA pUserDataItem );
    void DoOnACK( uint16_t wPeerRcvHeadSeqNo, uint16_t wPeerRevTailSeqNo );
    void DoSendOnePacketByTimerOrACK( int nCount );
    void SendDataItemToPeer( SEND_PACKET_ITEM & Item );

private:
	uint8_t *			m_pDataBuf;							// data buffer
	SEND_PACKET_ITEM 	m_aPackets[ MYRUDP_WINDOW_SIZE ];	// point to m_pBuf
	volatile int 		m_nMTUSize;
	uint8_t				m_byMTUIndex_Shifted;				// MTU Index, and has been shift 5 bits
	volatile uint16_t	m_wHeadSeqNo;
	volatile uint16_t	m_wTailSeqNo;
	uint32_t			m_dwSessionID;
	uint8_t *			m_pXorData;
	uint32_t			m_dwSessionTagID;
	int					m_nSameTailSeqNoCount;				// when get a ACK, if TailSeqNo Same, ++

	uint16_t 			m_wPeerRcvTailSeqNo;				// Peer Recv tail SeqNo
	uint16_t			m_wPeerRcvHeadSeqNo;				// peer recv head SeqNo
	int					m_nSamePeerHeaderSeqNoCount;		// same peer head seqno
	int					m_nSamePeerHeaderSeqNoGateLevel;	// how many same SeqNo to do resend action, 4,8,16,32

	pthread_mutex_t		m_SendDataSyncObj;

	pthread_mutex_t 	m_SyncObj_DataBufferAvailable;
	pthread_cond_t		m_EventDataBufAvailable;

	pthread_mutex_t 	m_SyncObj_UserData;
	std::list< SEND_PACKET_USER_DATA >	m_listUserData;

	volatile int		m_nSecondsNotGetDataFromPeer;		// 已经发送过的，但还没有收到对方数据的数据，如果有很多数据包，则不再发送

#ifdef __MYRUDP_USE_OPENSSL__
	uint8_t *			m_pbyDataEncryptAESKey;				// 48 bytes
#endif //#ifdef __MYRUDP_USE_OPENSSL__
};

#pragma pack( pop )

#endif // __MY_RUDP_PACKET_SENDER_H_20150210__

