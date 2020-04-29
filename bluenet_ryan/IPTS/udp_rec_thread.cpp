#include "udp_rec_thread.h"
#include <unistd.h>
#include <stdio.h>
#include <assert.h>


CUDPRecThread::CUDPRecThread()
{
    bzero( &m_hThread, sizeof(m_hThread) );
    m_pCondVar = NULL;
    m_pMutexVar = NULL;
    m_bIsRequestQuit = false;
    bzero( &m_attr, sizeof(m_attr) );

    m_nPollFDCount = 0;
    bzero( m_aPollFD, sizeof(m_aPollFD) );
}

CUDPRecThread::~CUDPRecThread()
{
    RemoveAll();
}
///-------------------------------------------------------
/// wcl,2019-07-03
/// 函数功能:
///		 创建接收udp线程
/// 输入参数:
///		无
/// 输出参数:
///		true：成功 false：失败
bool CUDPRecThread::CreateThread()
{
    pthread_cond_t CondSynObj = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t MutexSynObj = PTHREAD_MUTEX_INITIALIZER;

    m_pCondVar = &CondSynObj;
    m_pMutexVar = &MutexSynObj;
    pthread_mutex_lock( &MutexSynObj );		//	lock firstly

    try
    {
        pthread_attr_init( &m_attr );
        pthread_attr_setdetachstate( &m_attr, PTHREAD_CREATE_JOINABLE );
        int nCreateThreadRet = pthread_create( &m_hThread,&m_attr, RunLink, this );
        if( nCreateThreadRet )
            return false;
    }
    catch( ... )
    {
#ifdef _DEBUG
        assert( false );
#endif //_DEBUG
    }

    pthread_cond_wait( m_pCondVar, &MutexSynObj );
    pthread_mutex_unlock( &MutexSynObj );

    return true;
}
///-------------------------------------------------------
/// wcl,2019-07-03
/// 函数功能:
///		 停止接收udp线程
/// 输入参数:
///		无
/// 输出参数:
///		无
void CUDPRecThread::StopThread()
{
    if( (pthread_t)0 == m_hThread )
        return;

#ifdef _DEBUG
    fprintf( stderr, "CUDPRecThread::StopThread is call., %p->m_bIsRequestQuit=%d\n",this, m_bIsRequestQuit);
#endif //_DEBUG

    m_bIsRequestQuit = true;

#ifdef _DEBUG
    fprintf( stderr, "CUDPRecThread::StopThread is call., %p->m_bIsRequestQuit=%d\n",this, m_bIsRequestQuit);
#endif //_DEBUG

    pthread_join( m_hThread, NULL );
    pthread_attr_destroy( &m_attr );
}

///-------------------------------------------------------
/// wcl,2019-07-03
/// 函数功能:
///		 添加接收udp socket的ip和端口号
/// 输入参数:
///		lpszDstIP：接收udp的ip
///     wPort：接收udp的端口号
///     lpszLocalBindIP：？？？
///     pSubNetworkEncapsulation：封装mpe数据
/// 输出参数:
///		int： 返回接收udp socket的个数
int CUDPRecThread::AddDataPort( const char * lpszDstIP, WORD wPort, const char * lpszLocalBindIP, CSubNetworkEncapsulation * pSubNetworkEncapsulation )
{
    assert( lpszDstIP && wPort>=1024 );
    if( NULL == lpszDstIP || wPort < 1024 )
        return -1;

    int nNo = FindNotLock( lpszDstIP, wPort );
    if( nNo >= 0 )
        return m_aDataPort.size();

    CUDPDataPort * pDP = new CUDPDataPort;
    if( NULL == pDP )
        return -1;

    if( false == pDP->Initialize( lpszDstIP, wPort, lpszLocalBindIP, pSubNetworkEncapsulation ) )
    {
        delete pDP;
        return -1;
    }

    QMutexLocker syncobj( &m_SyncObj );
    m_aDataPort.push_back( pDP );
    RefleshPollFD();

    return m_aDataPort.size();
}

///-------------------------------------------------------
/// wcl,2019-07-03
/// 函数功能:
///		 启动udp接收线程
/// 输入参数:
///		lpParameter：类对象指针
/// 输出参数:
///		void*
void* CUDPRecThread::RunLink(void * lpParameter)
{
    CUDPRecThread * pClassTmp = (CUDPRecThread*)lpParameter;
    pClassTmp->ChangePriority();

    if( pClassTmp->m_pCondVar )
    {
#ifdef _DEBUG
        assert( pClassTmp->m_pMutexVar );
#endif //_DEBUG
        pthread_mutex_lock( pClassTmp->m_pMutexVar );	// lock firstly
        pthread_cond_signal( pClassTmp->m_pCondVar );	// notify that return from InitInstance
        pthread_mutex_unlock( pClassTmp->m_pMutexVar );
    }

    pClassTmp->Run();

    pClassTmp->RemoveAll();
    pClassTmp->m_hThread = (pthread_t)0;

    pthread_exit( (void*)NULL );				//	exit thread
}

///-------------------------------------------------------
/// wcl,2019-07-03
/// 函数功能:
///		 利用poll函数监测socket多连接，接受数据
/// 输入参数:
///		无
/// 输出参数:
///		无
void CUDPRecThread::Run()
{
    while( false == m_bIsRequestQuit )
    {
        QMutexLocker syncobj( &m_SyncObj );
        if( 0 == m_nPollFDCount )
        {
            syncobj.unlock();
            pthread_yield();
            Sleep( 10 );
            continue;
        }

        int i;
        for(i=0; i<m_nPollFDCount; i++)
        {
            m_aPollFD[i].revents = 0;
            m_aPollFD[i].events = POLLIN;
        }
        if( poll( m_aPollFD, m_nPollFDCount, 100 ) <= 0 )
        {
            syncobj.unlock();
            pthread_yield();
            Sleep( 1 );
            continue;
        }

        for( i=0; i<m_nPollFDCount; i++)
        {
            if( 0 == ( m_aPollFD[i].revents & POLLIN ) )
                continue;

            int nRetVal = recv( m_aPollFD[i].fd, m_abyDataBuf, sizeof(m_abyDataBuf), 0 );
            if( nRetVal > 0 )
            {
                OnDataReceived( i, m_abyDataBuf, nRetVal );
            }
        }
    }
}
///-------------------------------------------------------
/// wcl,2019-07-03
/// 函数功能:
///		 udp socket 接收到数据后封装mpe
/// 输入参数:
///		nIndex： udp socket 序列号
///     pBuf：接收到的数据bufer
///     nDataLen：接收到的数据长度
/// 输出参数:
///		无
void CUDPRecThread::OnDataReceived( int nIndex, uint8_t * pBuf, int nDataLen )
{
    // 封装mpe，并拆分成ts包
    CUDPDataPort * pDP = m_aDataPort[nIndex];
    CSubNetworkEncapsulation * pSubNetworkEncapsulation = pDP->GetSubNetworkEncapsulation();
    pSubNetworkEncapsulation->Encapsulation( pBuf, nDataLen );
}

///-------------------------------------------------------
/// wcl,2019-07-03
/// 函数功能:
///		 改变线程的优先级
///      注：取值范围：-20～19，默认值0，值越小优先级越高，反之值越大优先级越低
/// 输入参数:
///		无
/// 输出参数:
///		无
void CUDPRecThread::ChangePriority()
{
    nice( -5 );
}
///-------------------------------------------------------
/// wcl,2019-07-03
/// 函数功能:
///		 短延迟（纳秒级别）
/// 输入参数:
///		nMS： 毫秒
/// 输出参数:
///		无
void CUDPRecThread::Sleep( int nMS )
{
    timespec tTmp;
    tTmp.tv_sec = nMS /1000;
    tTmp.tv_nsec = (nMS %1000)*1000*1000;
    nanosleep( &tTmp, NULL );
}

int CUDPRecThread::FindNotLock( const char * lpszDstIP, WORD wPort )
{
    struct sockaddr_in tmpIP;
    tmpIP.sin_port = htons( wPort );
    tmpIP.sin_addr.s_addr = inet_addr( lpszDstIP );

    int nCount = m_aDataPort.size();
    for(int i=0; i<nCount; i++)
    {
        CUDPDataPort * pDP = m_aDataPort[i];
        if( NULL == pDP )
            continue;

        if( tmpIP.sin_port != pDP->m_DstIP.sin_port )
            continue;
        if( tmpIP.sin_addr.s_addr != pDP->m_DstIP.sin_addr.s_addr )
            continue;

        return i;
    }

    return -1;
}

void CUDPRecThread::RefleshPollFD()
{
    int nCount = m_aDataPort.size();
    if( nCount > MAX_DATAPORT_COUNT )
        nCount = MAX_DATAPORT_COUNT;

    m_nPollFDCount = 0;
    for(int i=0; i<nCount; i++)
    {
        CUDPDataPort * pDP = m_aDataPort[i];
        if( NULL == pDP )
            continue;
        m_aPollFD[m_nPollFDCount].fd = *pDP;
        m_nPollFDCount ++;
    }
}
///-------------------------------------------------------
/// wcl,2019-07-03
/// 函数功能:
///		 删除所有接收udp socket的ip和port
/// 输入参数:
///		无
/// 输出参数:
///		无
void CUDPRecThread::RemoveAll()
{
    QMutexLocker syncobj( &m_SyncObj );

    int nCount = m_aDataPort.size();
    for(int i=0; i<nCount; i++)
    {
        CUDPDataPort * pDP = m_aDataPort[i];
        if( pDP )
        {
            pDP->Invalid();
            delete pDP;
        }
        CSubNetworkEncapsulation * pSubNetworkEncapsulation = pDP->GetSubNetworkEncapsulation();
        if( pSubNetworkEncapsulation )
        {
            delete pSubNetworkEncapsulation;
            pSubNetworkEncapsulation = NULL;
        }
    }

    m_aDataPort.clear();
}
///-------------------------------------------------------
/// wcl,2019-07-03
/// 函数功能:
///		 删除接收udp socket的ip和端口号
/// 输入参数:
///		lpszDstIP：接收udp的ip
///     wPort：接收udp的端口号
/// 输出参数:
///		无
void CUDPRecThread::DeleteDataPort( const char * lpszDstIP, WORD wPort )
{
    QMutexLocker syncobj( &m_SyncObj);

    int nNo = FindNotLock( lpszDstIP, wPort );
    if( nNo < 0 )
        return;

    CUDPDataPort * pDP = m_aDataPort[nNo];
    pDP->Invalid();
    delete pDP;
    m_aDataPort.removeAt( nNo );

    RefleshPollFD();
}
