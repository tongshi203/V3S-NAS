// MyThread.H

#ifndef __MY_THREAD_INCLUDE_20030731__
#define __MY_THREAD_INCLUDE_20030731__

#pragma pack(push,4)

#include <pthread.h>

class CMyThread
{
public:
	CMyThread();

	virtual int ExitInstance() = 0;					//	返回参数
	virtual bool InitInstance() = 0;				//	是否成功
	virtual void Run()=0;							//	运行主体
	virtual void Delete() = 0;						//	safe delete this

public:
	static void * RunLink(void * lpParameter);
	virtual bool CreateThread( bool bWaitForInitInstace = true );
	void	StopThread(int dwTimeOut=5*1000);
    static void Sleep( int nMS );

public:
	pthread_t m_hThread;
	bool   m_bAutoDelete;					// If true, then auto delete seft

protected:
	bool volatile m_bIsRequestQuit;				// if request exit, if ture mean request quit.

private:
	bool	 m_bInitInstanceRetVal;			//	InitInstance 返回结果
    pthread_cond_t * m_pCondVar;			//	condition sync object
    pthread_mutex_t * m_pMutexVar;          //  Mutex Object variant
    pthread_attr_t m_attr;				// thread attr
};

#pragma pack(pop)

#endif // __MY_THREAD_INCLUDE_20030731__
