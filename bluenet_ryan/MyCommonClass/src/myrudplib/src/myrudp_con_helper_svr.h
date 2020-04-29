/*************************************************************************
 *
 *	My RUDP Response Packet builder
 *
 *	Chen Yongjian @ Zhoi
 *	2015.1.27 @ Xi'an
 *
 *************************************************************************/

#ifndef __MY_RUDP_CONNECTION_SVR_HELPER_H_20150127__
#define __MY_RUDP_CONNECTION_SVR_HELPER_H_20150127__

#include "myrudp_packbuilder.h"
#include "myrudp_auto_delete_buf.h"
#include <mythreadpool.h>
#include <MySyncObj.h>

class CMyRUDP_OnePeerObject;

#ifdef __MYRUDP_USE_OPENSSL__
class CMyRUDP_Connection_Svr_Helper : public CMyThreadPoolTask
#else
class CMyRUDP_Connection_Svr_Helper
#endif // __MYRUDP_USE_OPENSSL__
{
public:
	CMyRUDP_Connection_Svr_Helper( CMyRUDP_OnePeerObject * pPeerObj );
	virtual ~CMyRUDP_Connection_Svr_Helper();

	static bool IsReqCommandData( const unsigned char *pBuf, int nLen );
	static bool IsSynCommandData( const unsigned char *pBuf, int nLen );

	int OnReqData( const unsigned char *pBuf, int nLen );
	int OnWaitSynTimer( time_t tNow );
	int OnSynData( const unsigned char *pBuf, int nLen );

	static void BuildAndSendSyn2( CMyRUDP_OnePeerObject * pPeerObj );

#ifdef __MYRUDP_USE_OPENSSL__
	virtual void RunTask();
	void Abort();
#endif // #ifdef __MYRUDP_USE_OPENSSL__
	//--------------------------------------------------------------
	// live reference counter
	virtual int AddRef();
    virtual int Release();

	enum
	{
		MYRUDP_CONNECTION_STATE_WAIT_REQ = 0,			// waiting Request Command
		MYRUDP_CONNECTION_STATE_WAIT_SYN,				// waiting sync command
		MYRUDP_CONNECTION_STATE_CONNECTED,				// connected
	};
	enum
	{
		MYRUDP_CONNECTION_WAIT_SYNC_TIMES = 30,			// max wait time out is 30 second, this value can be changed according testing
	};

protected:
	int BuildResponseData();
	int SendOutResponseData();
	void UpdateResponseData_RetransmitOn();

protected:
	CMyRUDP_OnePeerObject * m_pPeerObj;

	CMyRUDP_AutoDeleteBuf   m_ReqInData;
	CMyRUDP_AutoDeleteBuf	m_RspOutData;
	CMyRUDP_AutoDeleteBuf	m_SyncInData;

	int						m_nState;
	int						m_nWaitSyncTimer;

#ifdef __MYRUDP_USE_OPENSSL__
	CMutex					m_SyncObj;			// 2015.4.18 CYJ Add
#endif // __MYRUDP_USE_OPENSSL__
	int						m_nRefCount;		// 2015.4.18 CYJ Add
};

#endif // __MY_RUDP_CONNECTION_SVR_HELPER_H_20150127__

