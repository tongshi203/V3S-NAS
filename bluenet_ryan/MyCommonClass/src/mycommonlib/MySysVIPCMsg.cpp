//--------------------------------------------------
//  A wrapper class for System V IPC's Message
//
//      Chen Yongjian @ Xi'an Tongshi
//      2003.11.20
//
//--------------------------------------------------

#include <sys/types.h>
#include <sys/ipc.h>
#include <assert.h>
#include <errno.h>
#include "MySysVIPCMsg.h"

//---------------------------------------------------
//  constructor
//  create message queue
//      pszPath             path, default is ".";
//      nID                 default is 'a'
CMySysVIPCMsg::CMySysVIPCMsg(const char * pszPath, int nID, bool bAutoClose)
{
    m_nMsgID = -1;
    if( NULL == pszPath || 0 == *pszPath )
        pszPath = ".";
    m_nKey = ftok( pszPath, nID );  //  return -1 is failed
    if( m_nKey < 0 )
        return;
    m_nMsgID = msgget( m_nKey, IPC_CREAT|0660 ); // -1 failed
    m_bAutoClose = bAutoClose;
}

CMySysVIPCMsg::~CMySysVIPCMsg()
{
	if( IsValid() && m_bAutoClose )
		msgctl( m_nMsgID, IPC_RMID, 0 );
}

//---------------------------------------------
//  check the object is valid or not,
//  return;
//      true            succ,
//      false           failed
bool CMySysVIPCMsg::IsValid()
{
    return m_nKey >= 0 && m_nMsgID >= 0;
}

//--------------------------------------------------
//  send message
//      pBuf            the buffer to send
//      nLen            buffer size, should <= 4095
//      bWait           wait message send, default is true
//  return:
//      0               succ
//      <0              failed, see also errno
int CMySysVIPCMsg::SendMessage( struct msgbuf * pBuf, int nLen, bool bWait )
{
#ifdef _DEBUG
    assert( IsValid() );
    assert( pBuf && nLen>=sizeof(long) );
#endif //_DEBUG
    if( NULL==pBuf || nLen<sizeof(long) || false == IsValid() )
        return -1;
    nLen -= sizeof(long);          // not include msgtype(long)
    int nFlags = bWait ? 0 : IPC_NOWAIT;
    return msgsnd( m_nMsgID, pBuf, nLen, nFlags );
}

//-----------------------------------------------------
//  read message
//      pBuf            output data buffer
//      nLen            out buffer length
//      bWait           wait for message, default = true
//      nMsgType        read message type, default = 0
//  return:
//      >=0             succ, and
//      -1              error
//      -2              no data
int CMySysVIPCMsg::ReadMessage( struct msgbuf * pBuf, int nLen, bool bWait, int nMsgType )
{
#ifdef _DEBUG
    assert( pBuf && nLen >= 4 );
    assert( IsValid() );
#endif //_DEBUG
    if( NULL == pBuf || nLen < 4 || false == IsValid() )
        return -1;
    int nFlags = bWait ? 0 : IPC_NOWAIT;
    nLen -= 4;
    int nRetVal = msgrcv( m_nMsgID, pBuf, nLen, nMsgType, nFlags );
    if( nRetVal >= 0)
        return nRetVal+4;           // succ
    if( errno == ENOMSG )
        return -2;
    return -1;
}

