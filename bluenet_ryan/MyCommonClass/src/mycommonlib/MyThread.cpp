#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "MyThread.h"

#ifdef _WIN32
#pragma message( "Can not be compiled under Microsoft Windows." )
#endif // _WIN32

CMyThread::CMyThread()
{
    bzero( &m_hThread, sizeof(m_hThread) );
    m_bAutoDelete = true;
    m_pCondVar = NULL;			//	condition sync object
    m_pMutexVar = NULL;
    m_bIsRequestQuit = false;
    m_bInitInstanceRetVal = false;
    bzero( &m_attr, sizeof(m_attr) );
}


void* CMyThread::RunLink(void * lpParameter)
{
    CMyThread * pClassTmp = (CMyThread*)lpParameter;
    pClassTmp->m_bInitInstanceRetVal = pClassTmp->InitInstance();
    if( pClassTmp->m_pCondVar )
    {
#ifdef _DEBUG
        assert( pClassTmp->m_pMutexVar );
#endif //_DEBUG
        pthread_mutex_lock( pClassTmp->m_pMutexVar );		//	lock firstly
        pthread_cond_signal( pClassTmp->m_pCondVar );	// notify that return from InitInstance
        pthread_mutex_unlock( pClassTmp->m_pMutexVar );
    }

//#ifdef _DEBUG
//    fprintf( stderr, "CMyThread is going to enter Run, this=%p\n", pClassTmp );
//#endif //_DEBUG

    if( pClassTmp->m_bInitInstanceRetVal )
        pClassTmp->Run();

#ifdef _DEBUG
    fprintf( stderr, "CMyThread ruturn from Run, this=%p\n", pClassTmp );
#endif //_DEBUG

    long nRetVal = pClassTmp->ExitInstance();
    pClassTmp->m_hThread = (pthread_t)0;

#ifdef _DEBUG
    printf("MyThread thread exit, this=%p\n", pClassTmp );
#endif //_DEBUG

    if( pClassTmp->m_bAutoDelete )				//	delete seft
        pClassTmp->Delete();

    pthread_exit( (void*)nRetVal );				//	exit thread
}

bool CMyThread::CreateThread( bool bWaitForInitInstace )
{
    pthread_cond_t CondSynObj = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t MutexSynObj = PTHREAD_MUTEX_INITIALIZER;
    if( bWaitForInitInstace )					// wait for Initance returned
    {
        m_pCondVar = &CondSynObj;
        m_pMutexVar = &MutexSynObj;
        pthread_mutex_lock( &MutexSynObj );		//	lock firstly
    }
    else
        m_pCondVar = NULL;			//	condition sync object

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

    if( false == bWaitForInitInstace )			//	need not wait for Initance return
        return true;

    pthread_cond_wait( m_pCondVar, &MutexSynObj );	//	wait for InitInstance return
    pthread_mutex_unlock( &MutexSynObj );

    return true;
}

void CMyThread::StopThread(int dwTimeOut)
{
    if( (pthread_t)0 == m_hThread )
        return;

#ifdef _DEBUG
    fprintf( stderr, "CMyThread::StopThread is call., %p->m_bIsRequestQuit=%d\n",this, m_bIsRequestQuit);
#endif //_DEBUG

    m_bIsRequestQuit = true;

#ifdef _DEBUG
    fprintf( stderr, "CMyThread::StopThread is call., %p->m_bIsRequestQuit=%d\n",this, m_bIsRequestQuit);
#endif //_DEBUG

    pthread_join( m_hThread, NULL );
    pthread_attr_destroy( &m_attr );
}

void CMyThread::Sleep( int nMS )
{
    timespec tTmp;
    tTmp.tv_sec = nMS /1000;
    tTmp.tv_nsec = (nMS %1000)*1000*1000;
    nanosleep( &tTmp, NULL );
}
