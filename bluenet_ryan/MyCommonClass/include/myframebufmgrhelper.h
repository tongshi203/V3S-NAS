///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2009-2-10
///
///		用途：
///			My Frame Buffer Manager template
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#ifndef __MY_FRAME_BUFFER_MANAGER_HELPER_H_20090210__
#define __MY_FRAME_BUFFER_MANAGER_HELPER_H_20090210__

#include <MyList.h>
#include <unistd.h>

// 2016.12.17 CYJ Add
#pragma pack(push,8)

template <class T> class CMyFrameRingBufMgr
{
public:
	CMyFrameRingBufMgr( int nTotalBufSize = 1024L*1024 );	// default buffer size = 1MB
	virtual ~CMyFrameRingBufMgr();

	bool Initialize( int nTotalBufSize = 1024L*1024 );
	void Invalidate();
	bool IsValid() const{ return ( m_pDataBuf != NULL ); }

	void Clean();

	PBYTE Allocate( DWORD dwLen, int nTimeOut=-1 );
	void Submit( int nLen, T & RefData );
	int GetCount(){ return m_ItemList.GetCount(); }

	T * Peek( PBYTE & pBuf, DWORD & dwBufLen, int nTimeOut=-1 );
	void Free();

	DWORD GetAvaiableDataSize()const
	{
		if( m_dwWritePtr >= m_dwReadPtr )
			return m_nDataBufSize - m_dwWritePtr + m_dwReadPtr;		// [___R.....W___]
		else
			return m_dwReadPtr - m_dwWritePtr;						// [___W....R____]

	}
	int GetBufAvaiablePercent()const{ return GetAvaiableDataSize()*100 / m_nDataBufSize; }
	// 2015.4.28 CYJ Add
	void Wakeup( bool bInEvent, bool bFreeEvent );

#ifdef _DEBUG
	void Dump();
#endif //_DEBUG

protected:
	typedef struct _MFRBMH_ITEM
	{
		T		m_RefData;
		DWORD	m_dwBufPtr;
		DWORD	m_dwBufLen;
	}MFRBMH_ITEM;

protected:
	CEvent m_AddDataEvent;
	CEvent m_FreeDataEvent;
	CCriticalSection m_ListSyncObj;
	CMyList< MFRBMH_ITEM > m_ItemList;

	PBYTE m_pDataBuf;				// 数据缓冲区
	DWORD m_nDataBufSize;			// 缓冲区大小
	DWORD volatile m_dwWritePtr;	// 写入偏移
	DWORD volatile m_dwReadPtr;		// 读取偏移

	DWORD m_dwBufPreAlloc;			// 预分配的缓冲区地址
	DWORD m_dwBufPreAllocSize;		// 预分配的缓冲区大小
};

///-------------------------------------------------------
/// CYJ,2009-2-10
/// 函数功能:
///		构造函数
/// 输入参数:
///		无
/// 返回参数:
///		无
template <class T> CMyFrameRingBufMgr<T>::CMyFrameRingBufMgr( int nTotalBufSize )
{
	m_pDataBuf = NULL;				// 数据缓冲区
	m_nDataBufSize = 0;				// 缓冲区大小

	m_dwWritePtr = 0;				// 写入偏移
	m_dwReadPtr = 0;				// 读取偏移
	m_dwBufPreAlloc = 0;
	m_dwBufPreAllocSize = 0;

	if( nTotalBufSize > 0 )
		Initialize( nTotalBufSize );
}

///-------------------------------------------------------
/// CYJ,2009-2-10
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
template <class T> CMyFrameRingBufMgr<T>::~CMyFrameRingBufMgr()
{
	Invalidate();
}

///-------------------------------------------------------
/// CYJ,2009-2-10
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
template <class T> bool CMyFrameRingBufMgr<T>::Initialize( int nTotalBufSize )
{
	Invalidate();

	m_pDataBuf = (PBYTE)malloc( nTotalBufSize + 128 );
	if( NULL == m_pDataBuf )
		return false;

	m_dwWritePtr = 0;				// 写入偏移
	m_dwReadPtr = 0;				// 读取偏移
	m_dwBufPreAlloc = 0;
	m_dwBufPreAllocSize = 0;

	m_nDataBufSize = nTotalBufSize;

	return true;
}

///-------------------------------------------------------
/// CYJ,2009-2-10
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
template <class T> void CMyFrameRingBufMgr<T>::Invalidate()
{
	if( m_pDataBuf )
		free( m_pDataBuf );
	m_pDataBuf = NULL;

	m_nDataBufSize = 0;				// 缓冲区大小

	m_dwWritePtr = 0;				// 写入偏移
	m_dwReadPtr = 0;				// 读取偏移
	m_dwBufPreAlloc = 0;
	m_dwBufPreAllocSize = 0;

	m_ItemList.RemoveAll();
}

///-------------------------------------------------------
/// CYJ,2009-2-10
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
template <class T> void CMyFrameRingBufMgr<T>::Clean()
{
	CSingleLock SyncObj( &m_ListSyncObj, true );

	m_ItemList.RemoveAll();

	m_dwWritePtr = 0;				// 写入偏移
	m_dwReadPtr = 0;				// 读取偏移
	m_dwBufPreAlloc = 0;
	m_dwBufPreAllocSize = 0;
}

///-------------------------------------------------------
/// CYJ,2009-2-10
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
template <class T> PBYTE CMyFrameRingBufMgr<T>::Allocate( DWORD dwLen, int nTimeOut )
{
	m_dwBufPreAllocSize = 0;

	for(int i=0; i<2; i++ )
	{
		if( m_dwWritePtr >= m_dwReadPtr )
		{		// [_________R-----W___]
			if( (m_nDataBufSize - m_dwWritePtr) > dwLen )
			{			// W____]
				m_dwBufPreAlloc = m_dwWritePtr;
				m_dwBufPreAllocSize = dwLen;
				return m_pDataBuf + m_dwBufPreAlloc;
			}
			if( m_dwReadPtr > dwLen )
			{			// [_____R
				m_dwBufPreAlloc = 0;
 				m_dwBufPreAllocSize = dwLen;
				return m_pDataBuf;
			}
		}
		else
		{		// [--------W____R-----]
			if( m_dwReadPtr - m_dwWritePtr > dwLen )
			{
				m_dwBufPreAlloc = m_dwWritePtr;
				m_dwBufPreAllocSize = dwLen;
				return m_pDataBuf + m_dwBufPreAlloc;
			}
		}

		if( 0 == nTimeOut )
			return NULL;

		m_FreeDataEvent.Lock( nTimeOut );
	}

	return NULL;			// 失败
}

///-------------------------------------------------------
/// CYJ,2009-2-10
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
template <class T> void CMyFrameRingBufMgr<T>::Submit( int nLen, T & RefData )
{
	ASSERT( m_dwBufPreAllocSize && m_dwBufPreAllocSize < m_nDataBufSize );

	MFRBMH_ITEM Item;
	memcpy( &Item.m_RefData, &RefData, sizeof(RefData) );
	Item.m_dwBufLen = nLen;
	Item.m_dwBufPtr = m_dwBufPreAlloc;

	CSingleLock SyncObj( &m_ListSyncObj, true );

	m_ItemList.AddTail( Item );

	DWORD dwNewPtr = m_dwBufPreAlloc + nLen;
	if( dwNewPtr >= m_nDataBufSize )
	{
		ASSERT( dwNewPtr == m_nDataBufSize );
		dwNewPtr = 0;
	}

	m_dwWritePtr = dwNewPtr;

	SyncObj.Unlock();

	m_AddDataEvent.SetEvent();
}

///-------------------------------------------------------
/// CYJ,2009-2-10
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
template <class T> T * CMyFrameRingBufMgr<T>::Peek( PBYTE & pBuf, DWORD & dwBufLen, int nTimeOut )
{
	CSingleLock SyncObj( &m_ListSyncObj, true );

	if( m_ItemList.IsEmpty() )
	{			// No data
		SyncObj.Unlock();

		if( 0 == nTimeOut )		//  CYJ,2009-4-27
			return NULL;

		m_AddDataEvent.Lock( nTimeOut );

		SyncObj.Lock();
		if( m_ItemList.IsEmpty() )
			return NULL;
	}

	MFRBMH_ITEM & Item = m_ItemList.GetHead();

	pBuf = Item.m_dwBufPtr + m_pDataBuf;
	dwBufLen = Item.m_dwBufLen;

	return &Item.m_RefData;
}

///-------------------------------------------------------
/// CYJ,2009-2-10
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
template <class T> void CMyFrameRingBufMgr<T>::Free()
{
	CSingleLock SyncObj( &m_ListSyncObj, true );

	if( m_ItemList.IsEmpty() )
		return;

	MFRBMH_ITEM Item = m_ItemList.RemoveHead();

	m_dwReadPtr = Item.m_dwBufPtr + Item.m_dwBufLen;

	SyncObj.Unlock();

	m_FreeDataEvent.SetEvent();
}

#ifdef _DEBUG
template <class T> void CMyFrameRingBufMgr<T>::Dump()
{
	CSingleLock SyncObj( &m_ListSyncObj, true );

	fprintf( stderr, "--------------------------------------------\n" );

	int nCount = m_ItemList.GetCount();
	fprintf( stderr, "%d Items in list.\n", nCount );
	fprintf( stderr, "Current ReadPtr = %08d, WritePtr = %08d, PtrAlloc=%d, Len=%d\n",
		m_dwReadPtr, m_dwWritePtr, m_dwBufPreAlloc, m_dwBufPreAllocSize );

	POSITION Pos = m_ItemList.GetHeadPosition();
	while( Pos )
	{
		MFRBMH_ITEM & Item = m_ItemList.GetNext( Pos );
		fprintf( stderr, "ReadPtr = %08d, Len=%08d, Next=%08d\n",
			Item.m_dwBufPtr, Item.m_dwBufLen, Item.m_dwBufPtr + Item.m_dwBufLen );
	}

	fprintf( stderr, "\n" );
}
#endif //_DEBUG

//--------------------------------------------------------------
/** CYJ 2015-04-28
 *
 *	wake up
 *
 * @param [in]	bInEvent			wake up in event
 * @param [in]	bFreeEvent			wake up free event
 */
template <class T> void CMyFrameRingBufMgr<T>::Wakeup( bool bInEvent, bool bFreeEvent )
{
	if( bInEvent )
		m_AddDataEvent.SetEvent();

	if( bFreeEvent )
		m_FreeDataEvent.SetEvent();
}

// 2016.12.17 CYJ Add
#pragma pack(pop)

#endif // __MY_FRAME_BUFFER_MANAGER_HELPER_H_20090210__