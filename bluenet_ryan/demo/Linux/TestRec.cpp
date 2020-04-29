// TestRec.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "../API_H/MyComDef.h"
#include "../API_H/IPRecSvr.h"

///-----------------------------------------------------
///	文件接收响应事件
class CMyFileOKEvent : public IDVBReceiverEvent
{
public:
    CMyFileOKEvent(){ m_nRef = 0; };
    virtual void OnFileOK( IFileObject * pObj, HDATAPORT hDataPort );
    virtual void OnSubFileOK( IFileObject * pObj,HDATAPORT hDataPort );
    virtual void OnProgress( HDATAPORT hDataPort, float fProgress, DWORD dwBroLoopCount, int dwFileCount, DWORD dwTotalLen, DWORD dwByteReceived, int nCountReceived, LPCSTR lpszFileName );
    virtual long AddRef(void);
    virtual long Release(void);
    virtual DWORD QueryInterface( REFIID iid, void **ppvObject);
private:
    long m_nRef;
};

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		On File OK Event
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyFileOKEvent::OnFileOK( IFileObject * pObj,HDATAPORT hDataPort )
{
   // printf("On File OK, FileName=%s, Len=%d\n", pObj->GetFileName(), pObj->GetDataLen() );
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		One Sub file ok event
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyFileOKEvent::OnSubFileOK( IFileObject * pObj,HDATAPORT hDataPort )
{
  //  printf("OnSubFileOK, FileName=%s, Len=%d\n", pObj->GetFileName(), pObj->GetDataLen() );
}

void CMyFileOKEvent::OnProgress( HDATAPORT hDataPort, float fProgress, DWORD dwBroLoopCount, int dwFileCount, DWORD dwTotalLen, DWORD dwByteReceived, int nCountReceived, LPCSTR lpszFileName )
{
   // printf("On Progress %.1f.\n", fProgress );
}

long CMyFileOKEvent::AddRef(void)
{
    return ++m_nRef;
}

long CMyFileOKEvent::Release(void)
{
    --m_nRef;
    if( m_nRef )
        return m_nRef;
    delete this;
    return 0;
}

DWORD CMyFileOKEvent::QueryInterface( REFIID iid, void **ppvObject)
{
    *ppvObject = this;
    return 0;
}

int main(int argc, char* argv[])
{
    if( argc < 2 )
    {
        printf("please input like this:./testD 1200 5053\n");
        return -1;
    }
    printf("recv port: ");
    for( int i=0; i<argc-1; i++ )
        printf("%s  ",argv[i+1]);
    printf("\n");

    IDVBFileReceiver * pReceiver = CreateDVBFileReceiver();
    if( NULL == pReceiver )
    {
        printf("Create Receiver object failed.\n");
        return 2;
    }

    //	Initialize Receiver object
    if( !pReceiver->Init() )
    {
        printf("Initialize receiver failed.\n");
        pReceiver->Release();
        return 3;
    }

    //	Set automatical save path, if the path is not empty string( valid path ),
    //	The files recived will be save to that path automatically
    pReceiver->SetAutoSavePath( "/mnt/mmcblk0p1/TS/website/tongshi" );
    CMyFileOKEvent * pEvent = new CMyFileOKEvent;
    if( pEvent )								//set the File OK event
    {
        pEvent->AddRef();
        pReceiver->RegisterEventResponser( pEvent );
    }

    //	Create data port and start receving files, more than one data port can be created
    for( int i=0; i<argc-1; i++ )
        pReceiver->CreateDataPort( "127.0.0.1", atoi(argv[i+1]), "127.0.0.1" );
    pReceiver->PutSendProgressEvent( true );

    for(;;)
    {
        //	pump the message and trigger the file OK event
        pReceiver->DoMessagePump();
        //printf("IP BPS=%d, File bps=%d\n",	pReceiver->GetIPPacketBPS(), pReceiver->GetFileBPS() );
        //		usleep( 1000 );
        sleep( 1 );
    }

    //	Close and release resources
    pReceiver->Close();

    if( pEvent )
        delete pEvent;

    return 0;
}

