//--------------------------------------------------
//  A wrapper class for System V IPC's Message
//
//      Chen Yongjian @ Xi'an Tongshi
//      2003.11.20
//
//--------------------------------------------------

#ifndef __MY_SYS_V_IPC_MSG_H_20031120__
#define __MY_SYS_V_IPC_MSG_H_20031120__

#pragma pack(push,4)

#include <sys/types.h>
#include <sys/msg.h>
class CMySysVIPCMsg
{
public:
    CMySysVIPCMsg(const char * pszPath=NULL, int nID='a', bool bAutoClose = true);
    ~CMySysVIPCMsg();
    int SendMessage( struct msgbuf * pBuf, int nLen, bool bWait=true );
    int ReadMessage( struct msgbuf * pBuf, int nLen, bool bWait=true, int nMsgType=0 );
    bool IsValid();

public:
    bool m_bAutoClose;

private:
    key_t m_nKey;           // return value of ftok
    int   m_nMsgID;         // return value of msgget
};

#pragma pack(pop)

#endif // __MY_SYS_V_IPC_MSG_H_20031120__
