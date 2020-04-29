/******************************************************************************
 *
 *	My RUDP Data packet manager
 *
 *	Chen Yongjian @ Zhoi
 *	2015.2.5 @ Xi'an
 *
 * Note
 *		since RelaxTalk always using Mobile network, so the MTU size always is 512 bytes
   *
 *****************************************************************************/

#ifndef __MY_RUDP_PACKET_MGR_H_20150205__
#define __MY_RUDP_PACKET_MGR_H_20150205__

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <list>
#include <stdint.h>
#include "myrudp_cmd_value.h"


#pragma pack( push, 8 )

class CMyRUDP_PacketMgr
{
public:
	CMyRUDP_PacketMgr();
	virtual ~CMyRUDP_PacketMgr();

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
    int Initialize( int nMTUSize );

    //--------------------------------------------------------------
    /** CYJ 2015-02-05
     *
     *	Invalidate and free resource
     */
    void Invalidate();

    //--------------------------------------------------------------
    /** CYJ 2015-02-11
     *
     *	the data packet manager is initialized succ.
     *
     * @return		true					valid
     *				false					invalid
     */
    bool IsValid(){ return m_pDataBuf != NULL; }

    //--------------------------------------------------------------
    /** CYJ 2015-02-08
     *
     *	reset all packet info item to be no data.
     */
    void Reset();

    //--------------------------------------------------------------
    /** CYJ 2015-02-07
     *
     *	On one data packet received
     *
     * @param [in]	pBuf					data buffer
     * @param [in]	nLen					data length
     * @param [out]	awACK_SeqNo				output ACK SeqNo
     * @param [i/o]	nACKCount				input  	awACK_SeqNo buffer size, max count = 16
     *										output	Actual ACK SeqNo output to the buffer
     * @return		0						need to wait more data packet
     *				>0						full packet count that has been completed received
     *				<0						error data
     */
    int OnDataPacketReady( const uint8_t * pBuf, int nLen, uint16_t awACK_SeqNo[], int & nACKCount );

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
    int CopyData( uint8_t *pBuf, int nBufSize, bool bNotRemove = false );

    //--------------------------------------------------------------
    /** CYJ 2015-02-14
     *
     *	Skip next packet data
     *
     * @return		true					has more data
     *				false					no data
     */
    bool SkipData();

    //--------------------------------------------------------------
    /** CYJ 2015-02-09
     *
     *	Get next packet's data length
     *
     * @return		0						no data
     *				>0						next data packet's data length
     */
    int GetNextPacketDataLen();

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
     *				0						no SeqNo need resend
     */
    int GetResendSeqNo( uint16_t awSeqNo[], int nCount, bool bForce );

    //--------------------------------------------------------------
    /** CYJ 2015-02-27
     *
     *	Get expected SeqNo
     */
    uint16_t GetExpectedSeqNo(){ return m_wSeqNoExpected; }

    //--------------------------------------------------------------
    /** CYJ 2015-02-23
     *
     *	Has data to be read
     *
     * @return	true						has data
     *			false						no data
     */
    bool HasDataToBeRead();

    uint16_t GetTailSeqNo()const { return m_wTailSeqNo; }

    //--------------------------------------------------------------
    /** CYJ 2015-04-02
     *
     *	On data block has been skiped
     *
     * @param [in]	wHeadSeqNo				Head SeqNo to be skipped
     * @param [in]	wTailSeqNo				Head SeqNo to be skipped
     */
    void OnSkipData( uint16_t wHeadSeqNo, uint16_t wTailSeqNo );

#ifdef _DEBUG
    void DebugDump();
#endif //_DEBUG

#ifdef __MYRUDP_USE_OPENSSL__
	void SetDataEncryptionAESKey( uint8_t abyAESKey[48] );
#endif // #ifdef __MYRUDP_USE_OPENSSL__


protected:
	bool IsSeqNoValid( uint16_t wSeqNo );
	bool IsAllPacketsReceived( uint16_t wSeqNo );
	void CollectACKSeq( uint16_t wSeqNo, uint16_t awACK_SeqNo[], int & nACKCount );
	static bool IsSeqNoInRange( uint16_t wSeqNo, uint16_t wHeadSeqNo, uint16_t wTailSeqNo );
	void GetExpectSeqNoToBeResend();
	void SkipDiscardItems();

#ifdef __MYRUDP_USE_OPENSSL__
	void DoDecryptData( const uint8_t * pBuf, int nLen, uint8_t * pOutBuf );
#endif // #ifdef __MYRUDP_USE_OPENSSL__

	enum
	{
		PACKET_INFO_FLAGS_COMMIT = 1,		// data has been received completed, and complete
		PACKET_INFO_FLAGS_SKIP = 2,			// data has been skip by the peer
	};

protected:
	typedef struct tagPACKET_INFO_ITEM
	{
		uint8_t	*	m_pBuf;					// pointer to buffer
		int			m_nDataLen;				// data length
		int			m_nFullDataLen;			// complete packet data size, only valid when PacketIndex = 0
		uint16_t	m_wSeqNo;				// SeqNumber
		uint8_t		m_byPacketCount;		// packet count of the data
		uint8_t		m_byPacketIndex;		// packet index of the data
		uint8_t		m_byFlags;				// Flags
	}PACKET_INFO_ITEM,*PPACKET_INFO_ITEM;

protected:
	uint8_t *	m_pDataBuf;								// data buffer
	PACKET_INFO_ITEM m_aPackets[ MYRUDP_WINDOW_SIZE ];	// point to m_pBuf
	int			m_nMTUSize;
	uint16_t	m_wHeadSeqNo;				// Point to the data last committed
	uint16_t	m_wTailSeqNo;				// Point to the data has been read
	uint16_t	m_wSeqNoExpected;			// SeqNo expected
	uint16_t	m_wSeqNoExpected_Last;		// last SeqNo expected
	int			m_nSeqNoExpectedSameTimes;	// same expected SeqNo counter

	pthread_mutex_t m_SyncObj;
	std::list<uint16_t>	m_aReadyPackets;

#ifdef __MYRUDP_USE_OPENSSL__
	uint8_t *			m_pbyDataEncryptAESKey;				// 48 bytes
#endif //#ifdef __MYRUDP_USE_OPENSSL__
};

#pragma pack( pop )

#endif // __MY_RUDP_PACKET_MGR_H_20150205__

