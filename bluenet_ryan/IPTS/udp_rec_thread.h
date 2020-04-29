#ifndef __UDP_REC_THREAD_H_20190702__
#define __UDP_REC_THREAD_H_20190702__

#include <sys/poll.h>
#include <QVector>
#include <QMutex>
#include "sub_network_encapsulation.h"
#include "udp_data_port.h"

class CUDPRecThread
{
public:
    CUDPRecThread();
    ~CUDPRecThread();

    bool CreateThread();
    void StopThread();

    int AddDataPort( const char * lpszDstIP, WORD wPort, const char * lpszLocalBindIP, CSubNetworkEncapsulation * pSubNetworkEncapsulation );

private:
    static void * RunLink(void * lpParameter);
    void Run();	//	运行主体
    void OnDataReceived( int nIndex, uint8_t * pBuf, int nDataLen );
    void ChangePriority();
    static void Sleep( int nMS );
    int FindNotLock( const char * lpszDstIP, WORD wPort );
    void RefleshPollFD();
    void RemoveAll();
    void DeleteDataPort( const char * lpszDstIP, WORD wPort );

private:
    enum
    {
        MAX_DATAPORT_COUNT = 128,			//	max data port count
    };
    pthread_t       m_hThread;
    bool volatile   m_bIsRequestQuit;				// if request exit, if ture mean request quit.
    int volatile     m_nPollFDCount;
    pthread_cond_t * m_pCondVar;			//	condition sync object
    pthread_mutex_t * m_pMutexVar;          //  Mutex Object variant
    pthread_attr_t     m_attr;				// thread attr

    struct pollfd               m_aPollFD[MAX_DATAPORT_COUNT];
    QVector<CUDPDataPort *>     m_aDataPort;
    QMutex                      m_SyncObj;			// synchornization object for m_aDataPort and m_aPollFD
    BYTE                        m_abyDataBuf[ 65536 ];
};

#endif // __UDP_REC_THREAD_H_20190702__
