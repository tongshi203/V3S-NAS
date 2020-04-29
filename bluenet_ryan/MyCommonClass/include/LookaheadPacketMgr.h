///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2001-1-1
///
///		用途：
///			LookAhead 模式管理数据包
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

///////////////////////////////////////////////////////////
//  2005-1-28		增加对 __LOOKAHEAD_CALL_CONSTRUCTURE_FOR_STRUCT__ 的支持，使得一个结构能够被初始化，相当于一个类的构造函数
//	2003.8.21		Add catch function.
//	2002.7.5		添加 GetItemCountInFreeList, GetInDataItemCount, DeAllocateEx
//	2002.1.3.22		将原来的 m_SynObj 改成 m_InDataSynObj 和 m_FreeDataSynObj，区别对待输入和释放
//					期望提高效率

#ifndef __LOOKAHEAD_PACKET_MGR_INCLUDE_20020124__
#define __LOOKAHEAD_PACKET_MGR_INCLUDE_20020124__

#ifndef _WIN32
	#include <unistd.h>
#endif //_WIN32

#pragma pack(push,4)

//ifdefine __LOOKAHEAD_DISABLE_CATCH_FUNCTION__ then all catch function is disabled
//#ifdefine __LOOKAHEAD_CALL_CONSTRUCTURE_FOR_STRUCT__	then call the OnConstructure function of the class/struct

//	T 类必需要要有方法 AddRef 和 Release
//	T 类必须提供 Preset 函数进行预置数据，!!! Preset 函数中，不能对引用次数进行修改。
//	T 类可以提供一个函数 OnConstructure 来实现类的构造函数，该情形适用于T类是一个结构，以实现类的构造函数
template <class T> class CLookaheadPacketMgr
{
public:
	CLookaheadPacketMgr( int nMaxItem = 2048, unsigned int nCatchCount=0 )	//	默认最多可以分配2K个单元
	{
		m_nMaxItemCount = nMaxItem;				//	最大可以分配的数据包个数
		m_nPacketAllocated = 0;					//	已经分配的数据个数
	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
        m_nCatchItemCount = nCatchCount;
        if( m_nCatchItemCount > (nMaxItem/4) )	//	can not too large
			m_nCatchItemCount = (nMaxItem/4);
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
	};

	~CLookaheadPacketMgr()
	{											//	析构函数，释放空间
		Reset();
	};

	//	清除所有数据
	void Clean(BOOL bDoLock = TRUE)
	{
		CSingleLock insynobj( &m_InDataSynObj, bDoLock );
		CSingleLock freesynobj( &m_FreeDataSynObj, bDoLock );
		if( !m_InDataList.IsEmpty() )
		{
			m_FreePacketList.AddTail( &m_InDataList );
			m_InDataList.RemoveAll();
        }

	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
		//	release all catched data
        if( !m_DeAllocateCatchList.IsEmpty() )
        {
	        m_FreePacketList.AddTail( &m_DeAllocateCatchList );
    	    m_DeAllocateCatchList.RemoveAll();
        }

		if( !m_AddPacketCatchList.IsEmpty() )
        {
	        m_FreePacketList.AddTail( &m_AddPacketCatchList );
    	    m_AddPacketCatchList.RemoveAll();
        }

		if( !m_PeekDataCatchList.IsEmpty() )
        {
	        m_FreePacketList.AddTail( &m_PeekDataCatchList );
    	    m_PeekDataCatchList.RemoveAll();
        }
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
	};

	//	释放所有数据包
	void Reset(BOOL bDoLock = TRUE )
	{
		CSingleLock synobj( &m_FreeDataSynObj, bDoLock );
		Clean( FALSE );
		int nCount = m_FreePacketList.GetCount();
		for(int i=0; i<nCount; i++)
		{
			T * pPacket = m_FreePacketList.RemoveHead();
			ASSERT( pPacket );
			if( pPacket )
				pPacket->Release();
		}
		m_nPacketAllocated = 0;					//	已经分配的数据个数

	#ifdef _WIN32
		m_FreePacketEvent.ResetEvent();			//	有数据释放，即可以利用
		m_AddDataEvent.ResetEvent();			//	添加数据
	#endif //_WIN32  // I do not known, should I reset the event under Linux ?
	};

	//	添加数据包
	BOOL	AddPacket( T * pPacket, BOOL bDoLock = TRUE, BOOL bFlush = FALSE )
	{
		ASSERT( pPacket );
		if( NULL == pPacket )
			return FALSE;

		pPacket->AddRef();

	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
		if( m_nCatchItemCount > 1 )
        {							//	support deallocate
    	    if( m_AddPacketCatchList.AddTail( pPacket ) )
            {
				if( FALSE == bFlush && m_AddPacketCatchList.GetCount() < m_nCatchItemCount )
                	return TRUE;
             	CSingleLock synobj( &m_InDataSynObj, bDoLock );
                m_InDataList.AddTail( &m_AddPacketCatchList );
                m_AddPacketCatchList.RemoveAll();

			#ifdef _WIN32
                SetInDataEvent();
			#endif // #ifdef _WIN32
				return TRUE;
            }
		}
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__

		CSingleLock synobj( &m_InDataSynObj, bDoLock );

		if( NULL == m_InDataList.AddTail( pPacket ) )
        {
        	pPacket->Release();
        	return FALSE;
        }
	#ifdef _WIN32
		SetInDataEvent();
	#endif //_WIN32
		return TRUE;
	};

	///----------------------------------------------------
	///	刷新 AddPacket 缓冲区
	void FlushAddCache( BOOL bDoLock = TRUE )
	{
	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
		CSingleLock synobj( &m_InDataSynObj, bDoLock );
		if( FALSE == m_AddPacketCatchList.IsEmpty() )	//  CYJ,2006-2-26 添加条件判断，防止没有输入时也产生信号
		{
			m_InDataList.AddTail( &m_AddPacketCatchList );
			m_AddPacketCatchList.RemoveAll();

		#ifdef _WIN32
			SetInDataEvent();
		#endif // #ifdef _WIN32
		}
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
	}

	//	分配数据包
	//	入口参数
	//		dwTimeOut		超时时间，默认一直等待
	//	返回参数
	//		NULL			失败
	T * AllocatePacket( DWORD dwTimeOut = -1, BOOL bDoLock = TRUE )
	{
		CSingleLock synobj( &m_FreeDataSynObj );
		if( bDoLock && FALSE == synobj.Lock( dwTimeOut ) )
			return NULL;						//	超时

		if( 0 == m_FreePacketList.GetCount() )
		{										//	空闲队列中没有数据，从堆中分配
			if( m_nPacketAllocated >= m_nMaxItemCount )
			{									//	不能再从堆中分配，只好等等看看有没有释放回来
				if( 0 == dwTimeOut )
					return NULL;				//	不等待，直接返回
				if( bDoLock )
					synobj.Unlock();

			#ifdef _WIN32
				if( FALSE == WaitForFreePacketEvent( dwTimeOut ) )
					return NULL;				//	超时，还没有数据包释放
			#else
				// linux
				while( 1 )
				{
					if( GetItemCountInFreeList( bDoLock ) )
						break;
					if( 0xFFFFFFFF != dwTimeOut )
					{
						if( 1 == dwTimeOut )
							return NULL;
						dwTimeOut --;
					}
					usleep( 1000 );
				}
			#endif //_WIN32

				if( bDoLock && FALSE == synobj.Lock( dwTimeOut ) )
					return NULL;

				if( 0 == m_FreePacketList.GetCount() )
					return NULL;				//	再次判断
			}
			else
			{									//	还可以分配
				T * pPacket = new T;
				if( NULL == pPacket )
					return NULL;
			#ifdef __LOOKAHEAD_CALL_CONSTRUCTURE_FOR_STRUCT__
				pPacket->OnConstructure();		//  CYJ,2005-1-28 模仿实现一个类的构造函数。
			#endif // __LOOKAHEAD_CALL_CONSTRUCTURE_FOR_STRUCT__
				m_nPacketAllocated ++;
				pPacket->AddRef();
			#ifdef _DEBUG
				m_pLastAllocObject = pPacket;
			#endif // _DEBUG
				pPacket->Preset();
				return pPacket;
			}
		}
		ASSERT( m_nPacketAllocated <= m_nMaxItemCount );
		ASSERT( m_FreePacketList.GetCount() );
		T * pPacket = m_FreePacketList.RemoveHead();
		ASSERT( pPacket );						//	因为 DeAllocate 已经调用 AddRef;
	#ifdef _DEBUG
		m_pLastAllocObject = pPacket;
	#endif // _DEBUG
		pPacket->Preset();
		return pPacket;
	};

	//	释放数据包
	void DeAllocate( T * pPacket, BOOL bDoLock = TRUE  )
	{
		ASSERT( pPacket );
		pPacket->AddRef();

	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
		if( m_nCatchItemCount > 1 )
        {							//	support deallocate
	        if( m_DeAllocateCatchList.AddTail( pPacket ) )
            {						//	If add tail fail, try another way
                if( m_DeAllocateCatchList.GetCount() < m_nCatchItemCount )
                    return;

                CSingleLock synobj( &m_FreeDataSynObj, bDoLock );
                m_FreePacketList.AddTail( &m_DeAllocateCatchList );
                m_DeAllocateCatchList.RemoveAll();
			#ifdef _WIN32
                SetFreePacketEvent();
			#endif //_WIN32
                return;
            }
        }
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__

		CSingleLock synobj( &m_FreeDataSynObj, bDoLock );
		if( m_FreePacketList.AddTail( pPacket ) )
		{
		#ifdef _WIN32
			SetFreePacketEvent();
		#endif // #ifdef _WIN32
		}
        else
        {
         	m_nPacketAllocated --;			//	add to free list fail, so release it directly
            pPacket->Release();
        }
	};

	//	2002.7.5 添加
	//	释放数据包
	void DeAllocateEx( T * pPacket, BOOL bDiscard = FALSE, BOOL bDoLock = TRUE  )
	{
		ASSERT( pPacket );
		if( bDiscard )
		{
			CSingleLock synobj( &m_FreeDataSynObj, bDoLock );
			m_nPacketAllocated --;			//	丢弃了，算是未曾分配该对象
		}
		else
			DeAllocate( pPacket, bDoLock );
	};

#ifdef _WIN32
	//	等待数据输入事件
	BOOL WaitForInDataEvent(DWORD dwTimeOut = -1)
	{
		ASSERT( dwTimeOut );
		CSingleLock synobj( &m_AddDataEvent );
		return synobj.Lock( dwTimeOut );
	};
#endif // #ifdef _WIN32

#ifdef _WIN32
	//	等待释放数据事件
	BOOL WaitForFreePacketEvent( DWORD dwTimeOut = -1 )
	{
		ASSERT( dwTimeOut );
		CSingleLock synobj( &m_FreePacketEvent );
		return synobj.Lock( dwTimeOut );
	};
#endif // #ifdef _WIN32

	//	获取数据
	//	入口参数
	//		dwTimeOut			0	表示不等待
	//							-1  一直等待
	T * PeekData( DWORD dwTimeOut = -1, BOOL bDoLock = TRUE  )
	{
	    T * pPacket;

	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
	    if( FALSE == m_PeekDataCatchList.IsEmpty() )
        {
			pPacket = m_PeekDataCatchList.RemoveHead();
			return pPacket;							//	不用调用 AddRef 因为 AddPacket 已经调用 AddRef
        }
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__

		CSingleLock synobj( &m_InDataSynObj );
		if( bDoLock && FALSE == synobj.Lock( dwTimeOut ) )
			return NULL;

		if( FALSE == m_InDataList.IsEmpty() )
		{
        	pPacket = m_InDataList.RemoveHead();

	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
			if( FALSE == m_InDataList.IsEmpty() )
            {
		    	m_PeekDataCatchList.AddTail( &m_InDataList );
    	        m_InDataList.RemoveAll();
            }
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__

			return pPacket;							//	不用调用 AddRef 因为 AddPacket 已经调用 AddRef
		}
		if( 0 == dwTimeOut )
			return NULL;			//	不等待，若没有则立即返回

		if( bDoLock )
			synobj.Unlock();		//	释放，等待新的数据

	#ifdef _WIN32
		if( FALSE == WaitForInDataEvent( dwTimeOut ) )
			return NULL;

		ASSERT( FALSE == m_InDataList.IsEmpty() );
	#else	// Linux, wait myself
		while( 1 )
		{
			if( GetInDataItemCount( bDoLock ) )
				break;

			if( 0xFFFFFFFF != dwTimeOut )
			{
				if( 1 == dwTimeOut )
					return NULL;			// timeout
				dwTimeOut --;
			}
			usleep( 1000 );	// wait 1 ms
		}
	#endif // #ifdef _WIN32

		if( bDoLock && FALSE == synobj.Lock( dwTimeOut ) )
			return NULL;

		return m_InDataList.RemoveHead();			//	不用调用 AddRef 因为 AddPacket 已经调用 AddRef
	};

#ifdef _WIN32
	//	有数据输入
	void	SetInDataEvent()
	{
		m_AddDataEvent.PulseEvent();
	};
#endif // #ifdef _WIN32

#ifdef _WIN32
	//	数据包释放事件
	void	SetFreePacketEvent()
	{
		m_FreePacketEvent.PulseEvent();
	};
#endif // #ifdef _WIN32

	////////////////////////////////////////////////////
	//	2002.7.5 添加
	//	获取空闲列表中的项目数
	int		GetItemCountInFreeList(BOOL bDoLock = TRUE)
	{
		CSingleLock synobj( &m_FreeDataSynObj, bDoLock );
		return m_FreePacketList.GetCount();
	}

	////////////////////////////////////////////////////
	//	2002.7.5 添加
	//	获取空闲列表中的项目数
	int		GetInDataItemCount(BOOL bDoLock = TRUE)
	{
		CSingleLock synobj( &m_InDataSynObj, bDoLock );

	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
		return m_InDataList.GetCount() + m_PeekDataCatchList.GetCount();
	#else
		return m_InDataList.GetCount();
	#endif //__LOOKAHEAD_DISABLE_CATCH_FUNCTION__
	}

public:
	CCriticalSection	m_InDataSynObj;		//	InData,线程控制权同步对象
	CCriticalSection	m_FreeDataSynObj;	//	FreeData,线程控制权同步对象

protected:
#ifdef _WIN32
	CEvent	m_FreePacketEvent;				//	有数据释放，即可以利用
	CEvent	m_AddDataEvent;					//	添加数据
#endif // #ifdef _WIN32

#ifdef _WIN32
	CList< T *, T* > m_FreePacketList;    	//	当前可以利用的数据包
	CList< T *, T* > m_InDataList;	    	//	有数据的数据包
#else // LInux
    CMyList< T * > m_FreePacketList;    	//	当前可以利用的数据包
	CMyList< T * > m_InDataList;	    	//	有数据的数据包
#endif // _WIN32

	int	m_nMaxItemCount;					//	最大可以分配的数据包个数
	int	m_nPacketAllocated;					//	已经分配的数据个数

#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
  	int	m_nCatchItemCount;					//	Catch Item count, if 0, do not catch any item
#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__

#ifdef _DEBUG
	T * m_pLastAllocObject;
#endif // _DEBUG

private:
#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__

  #ifdef _WIN32									//	2003.8.21 add, catch list
	CList< T *, T* > m_DeAllocateCatchList;		//	catch for Deallocate function
    CList< T *, T* > m_AddPacketCatchList;  	//	catch for AddPacket function
    CList< T *, T* > m_PeekDataCatchList;  		//	catch for Peek Data function
  #else	//Linux
	CMyList< T * > m_DeAllocateCatchList;  		//	catch for allocate function
    CMyList< T * > m_AddPacketCatchList;  		//	catch for AddPacket function
    CMyList< T *>  m_PeekDataCatchList;  			//	catch for Peek Data function
  #endif // _WIN32

#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
};

#pragma pack(pop)

#endif // __LOOKAHEAD_PACKET_MGR_INCLUDE_20020124__

