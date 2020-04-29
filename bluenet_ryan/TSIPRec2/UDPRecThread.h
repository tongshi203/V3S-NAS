///---------------------------------------------------------
///
///      Chen Yongjian @ Xi'an Tongshi Technology Limited
///			   2004.4.6
///      This file is implemented:
///				UDP receive thread
///-----------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef __UDPRecThread_H_20040406__
#define __UDPRecThread_H_20040406__

#include <MyThread.h>
#include <MyArray.h>

#ifndef _WIN32
	#include <MySyncObj.h>
	#include <sys/poll.h>	
#else
	#include <MyMap.h>
#endif //_WIN32

#include "UDPDataPortLinux.h"
#include "IPData.h"

class CUDPRecThread : public CMyThread
{
public:
	CUDPRecThread();
    ~CUDPRecThread();

	virtual int ExitInstance();					//	返回参数
#ifdef _WIN32
	virtual BOOL InitInstance();				//	是否成功
#else
	virtual bool InitInstance();				//	是否成功
#endif //_WIN32
	virtual void Run();							//	运行主体
	virtual void Delete();

    int AddDataPort( LPCSTR lpszDstIP, int nPort, LPCSTR lpszLocalBindIP,COneDataPortItem * pDataPortItem );
    int Find( LPCSTR lpszDstIP, int nPort, bool bDoLock = true );
    void DeleteDataPort( LPCSTR lpszDstIP, int nPort );
    void RemoveAll();

    enum
    {
#ifdef _WIN32
		MAX_DATAPORT_COUNT = FD_SETSIZE,
#else
    	MAX_DATAPORT_COUNT = 128,			//	max data port count
#endif // _WIN32
    };

private:
	int volatile m_nPollFDCount;	

#ifdef _WIN32
	fd_set m_SelectHandles;
	CMyMap<int,int,COneDataPortItem * ,COneDataPortItem * > m_mapDataPortItemAssociated;
#else
	struct pollfd m_aPollFD[MAX_DATAPORT_COUNT];
	COneDataPortItem * m_aDataPortItemAssociated[MAX_DATAPORT_COUNT];
#endif //_WIN32    
	CMyArray< CUDPDataPort * > m_aDataPort;
    CCriticalSection   m_SyncObj;			// synchornization object for m_aDataPort and m_aPollFD

private:
	void WaitTillPacketBufIsEmpty(int nTimeOut = 3);
	void RefleshPollFD();
};


//---------------------------------------------------------------------------
#endif	// __UDPRecThread_H_20040406__
