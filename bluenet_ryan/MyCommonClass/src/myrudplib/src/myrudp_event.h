/******************************************************************
 *
 *	My RUDP event interface
 *
 *	Chen Yongjian @ zhoi
 *	2015.2.14 @ Xi'an
 *
 ******************************************************************/

#ifndef __MY_RUDP_EVENT_H_20150214__
#define __MY_RUDP_EVENT_H_20150214__

#pragma pack( push, 8 )

class CMyRUDP_EventObj
{
public:
	CMyRUDP_EventObj(){}
	virtual ~CMyRUDP_EventObj(){}

	//--------------------------------------------------------------
	/** CYJ 2015-02-14
	 *
	 *	Connection state changed
	 *
	 * @param [in]	nState			new State, see also MY_RUDP_CONNECTION_STATE_xxx
	 *
	 * @note	may be called in different thread
	 *			for server, only 2 states: connected or disconnected
	 */
    virtual void OnConnectionStateChanged( int nState ){}

    //--------------------------------------------------------------
    /** CYJ 2015-02-14
     *
     *	on one data packet sent succ
     *
     * @param [in]	pUserData		user data
     *
	 * @note	may be called in different thread
     */
    virtual void OnDataSentSucc( void * pUserData ){}

    //--------------------------------------------------------------
    /** CYJ 2015-02-14
     *
     *	On send data failed
     *
     * @param [in]	pUserData		user data
     *
	 * @note	may be called in different thread
     */
    virtual void OnSendDataFailed( void * pUserData ){}

    //--------------------------------------------------------------
    /** CYJ 2015-02-25
     *
     *	Allocate memory
     *
     * @param [in]	nBufSize			data buffer size
	 *
     * @return		NULL				succ
     *				other				buffer pointer
     */
    virtual unsigned char * OnDataReceived_Allocate( int nBufSize ) = 0;

    //--------------------------------------------------------------
    /** CYJ 2015-02-25
     *
     *	Free memory, since some error occur
     *
     * @param [in]	pBuf				buffer to be free
     */
    virtual void OnDataReceived_Free( const unsigned char * pBuf ){}

	//--------------------------------------------------------------
    /** CYJ 2015-02-14
     *
     *	On data received
     *
     * @param [in]	pBuf			data buffer
     * @param [in]	nDataLen		data length
     * @param [in]	nBufSize		buffer size
     *
	 * @note	may be called in different thread
     */
    virtual void OnDataReceived_Commit( const unsigned char * pBuf, int nDataLen, int nBufSize ) = 0;

    //--------------------------------------------------------------
    /** CYJ 2015-05-07
     *
     *	On Keep alive packet received, to indicate that the user is online
     *
     * @param [in] nSecondsLastRecived	seconds last received from the peer
     */
    virtual void OnKeepAlivePacketReceived( int nSecondsLastRecived ){}

    virtual int AddRef() = 0;
    virtual int Release() = 0;

	enum
	{
		MY_RUDP_CONNECTION_STATE_DISCONNECTED = 0,		// disconnected
		MY_RUDP_CONNECTION_STATE_CONNECTING,			// connecting
		MY_RUDP_CONNECTION_STATE_DISCONNECTING,			// disconnecting
		MY_RUDP_CONNECTION_STATE_CONNECTED,				// connected
	};
};

#pragma pack( pop )

#endif // __MY_RUDP_EVENT_H_20150214__

