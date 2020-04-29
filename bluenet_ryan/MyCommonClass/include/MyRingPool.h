///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2003-1-17
///
///		环形缓冲区模板
///
///=======================================================

#ifndef __MY_RING_POOL_TEMPLATE_20030116__
#define __MY_RING_POOL_TEMPLATE_20030116__

#pragma pack(push,4)

template <class T>
class CMyRingPool
{
public:
	CMyRingPool( int nInitSize = 0);
	~CMyRingPool();

	///-------------------------------------------------------
	/// 2003-1-17
	/// 功能：
	///		初始化缓冲区
	/// 入口参数：
	///		nInitSize			初始化的大小
	/// 返回参数：
	///		TRUE				成功
	///		FALSE				失败
	BOOL Initialize( int nInitSize )
	{
		Invalidate();
		ASSERT( nInitSize );
		if( 0 == nInitSize )
			return FALSE;

		m_pBuf = new T[nInitSize];
		if( NULL == m_pBuf )
			return FALSE;
		m_nTotalUnitCount = nInitSize;
		ASSERT( m_nHeadPtr == 0 );				//	头指针
		ASSERT( m_nTailPtr == 0 );				//	尾指针

		return TRUE;
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// 功能：
	///		是否有效
	/// 入口参数：
	///		无
	/// 返回参数：
	///		TRUE					有效
	///		FALSE					无效
	BOOL IsValid()
	{
		return NULL != m_pBuf;
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// 功能：
	///		使之无效
	/// 入口参数：
	///		无
	/// 返回参数：
	///		无
	void Invalidate()
	{
		if( m_pBuf )
			delete m_pBuf;
		m_pBuf = NULL;
		m_nTotalUnitCount = 0;		//	总单元数
		m_nHeadPtr = 0;				//	头指针
		m_nTailPtr = 0;				//	尾指针
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// 功能：
	///		读取数据
	/// 入口参数：
	///		pBuf				输出数据的缓冲区
	///		nCount				个数，单元个数，不是字节数
	/// 返回参数：
	///		实际读取的单元个数
	int Read( T * pBuf, int nCount )
	{
		ASSERT( pBuf && nCount );
		if( NULL == pBuf || 0 == nCount )
			return 0;
		int nUnitCountHasData = AvailableUnit();
		if( 0 == nUnitCountHasData )
			return 0;
		if( nCount > nUnitCountHasData )
			nCount = nUnitCountHasData;
		T * pSrcBuf = m_pBuf + m_nTailPtr;
		if( m_nHeadPtr >= m_nTailPtr )
		{							//	头指针 >= 尾指针
			memcpy( pBuf, pSrcBuf, nCount*sizeof(T) );
			m_nTailPtr += nCount;
			ASSERT( m_nTailPtr <= m_nHeadPtr );
			ASSERT( m_nTailPtr < m_nTotalUnitCount );
			return nCount;
		}
		int nCount1 = m_nTotalUnitCount - m_nTailPtr;
		if( nCount1 > nCount )
			nCount1 = nCount;
		memcpy( pBuf, pSrcBuf, nCount1*sizeof(T) );
		m_nTailPtr += nCount1;
		pBuf += nCount1;

		ASSERT( m_nTailPtr <= m_nTotalUnitCount );
		if( m_nTailPtr < m_nTotalUnitCount )
		{
			ASSERT( nCount == nCount1 );
			return nCount;
		}
		nCount -= nCount1;
		if( nCount > m_nHeadPtr )
			nCount = m_nHeadPtr;
		pSrcBuf = m_pBuf;
		if( nCount )
			memcpy( pBuf, pSrcBuf, nCount*sizeof(T) );
		m_nTailPtr = nCount;
		return nCount + nCount1;
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// 功能：
	///		写入数据
	/// 入口参数：
	///		pBuf				缓冲区
	///		nCount				记录数
	/// 返回参数：
	///		实际写入的记录数
	int Write( T * pBuf, int nCount )
	{
		int nFreeUnitCount = FreeUnitCount();
		if( 0 == nFreeUnitCount )
			return 0;
		if( nCount > nFreeUnitCount )
			nCount = nFreeUnitCount;
		ASSERT( nCount < m_nTotalUnitCount );
		T * pDstBuf = m_pBuf + m_nHeadPtr;
		if( m_nHeadPtr < m_nTailPtr )
		{
			ASSERT( nFreeUnitCount == (m_nTailPtr - m_nHeadPtr - 1) );
			memcpy( pDstBuf, pBuf, nCount*sizeof(T) );
			m_nHeadPtr += nCount;
			ASSERT( m_nHeadPtr < m_nTotalUnitCount );
			ASSERT( m_nHeadPtr < m_nTailPtr );
			ASSERT( m_nTailPtr < m_nTotalUnitCount );
			return nCount;
		}
		ASSERT( nFreeUnitCount == ( m_nTotalUnitCount - m_nHeadPtr + m_nTailPtr - 1 ) );

		int nCount1 = m_nTotalUnitCount - m_nHeadPtr;
		if( 0 == m_nTailPtr )
			nCount1 --;
		if( nCount1 > nCount )
			nCount1 = nCount;
		memcpy( pDstBuf, pBuf, nCount1*sizeof(T) );
		pBuf += nCount1;
		nCount -= nCount1;
		m_nHeadPtr += nCount1;
		if( m_nHeadPtr < m_nTotalUnitCount )
			return nCount1;

		ASSERT( m_nHeadPtr == m_nTotalUnitCount );
		ASSERT( nCount < m_nTotalUnitCount );
		ASSERT( nCount <= FreeUnitCount() );

		pDstBuf = m_pBuf;
		if( nCount )
			memcpy( pDstBuf, pBuf, nCount*sizeof(T) );
		m_nHeadPtr = nCount;

		return nCount1 + nCount;
	}

	///-------------------------------------------------------
	/// CYJ,2005-3-10
	/// 函数功能:
	///		获取下次要写的数据单元
	/// 输入参数:
	///		无
	/// 返回参数:
	///		NULL			失败，没有数据
	///		其他			数据单元
	T * GetNextWriteUnit()
	{
		if( FreeUnitCount() == 0 )
			return NULL;
		return m_pBuf + m_nHeadPtr;
	}

	///-------------------------------------------------------
	/// CYJ,2005-3-10
	/// 函数功能:
	///		增长写指针，该函数通常与 GetNextWriteUnit 配合使用
	/// 输入参数:
	///		无
	/// 返回参数:
	///		无
	void IncreaseWritePtr()
	{
		ASSERT( FreeUnitCount() );
		m_nHeadPtr ++;
		if( m_nHeadPtr >= m_nTotalUnitCount )
			m_nHeadPtr = 0;
	}

	///-------------------------------------------------------
	/// CYJ,2005-3-10
	/// 函数功能:
	///		获取当前可读到数据单元，通常与 Skip 配合使用
	/// 输入参数:
	///		无
	/// 返回参数:
	///		NULL				没有数据可读
	///		其他				数据
	T * GetCurrentReadUnit()
	{
		if( AvailableUnit() == 0 )
			return NULL;
		return m_pBuf + m_nTailPtr;
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// 功能：
	///		跳过 n 个记录
	/// 入口参数：
	///		nCount			跳过的记录数
	/// 返回参数：
	///		无
	void Skip( int nCount = 1 )
	{
		int nCountInBuf = AvailableUnit();
		ASSERT( nCountInBuf < m_nTotalUnitCount );
		if( nCount > nCountInBuf )
			nCount = nCountInBuf;
		m_nTailPtr += nCount;
		m_nTailPtr %= m_nTotalUnitCount;
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// 功能：
	///		清空数据
	/// 入口参数：
	///		无
	/// 返回参数：
	///		无
	void Empty()
	{
		m_nHeadPtr = 0;				//	头指针
		m_nTailPtr = 0;				//	尾指针
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// 功能：
	///		有数据的单元个数
	/// 入口参数：
	///		无
	/// 返回参数：
	///		无
	int AvailableUnit()
	{
		if( m_nHeadPtr >= m_nTailPtr )
			return m_nHeadPtr - m_nTailPtr;
		return m_nHeadPtr + m_nTotalUnitCount - m_nTailPtr;
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// 功能：
	///		空闲缓冲区个数
	/// 入口参数：
	///		无
	/// 返回参数：
	///		空闲的缓冲区个数
	///	注：
	///		总有一个单元为保留单元，作为隔离区
	int FreeUnitCount()
	{
		return m_nTotalUnitCount - AvailableUnit() - 1;
	}

#ifdef _DEBUG
	void Dump()
	{
		TRACE("HeadPtr=%d, TailPtr=%d, TotalCount=%d,", m_nHeadPtr, m_nTailPtr, m_nTotalUnitCount );
		TRACE("DataInBuf=%d, FreeUnit=%d\n", AvailableUnit(), FreeUnitCount() );
	}
#endif // _DEBUG

protected:
	T * m_pBuf;					//	缓冲区
	int m_nTotalUnitCount;		//	总单元数
	int m_nHeadPtr;				//	头指针
	int m_nTailPtr;				//	尾指针
};

template <class T>
CMyRingPool<T>::CMyRingPool( int nInitSize )
{
	m_pBuf = NULL;
	m_nTotalUnitCount = 0;		//	总单元数
	m_nHeadPtr = 0;				//	头指针
	m_nTailPtr = 0;				//	尾指针
	if( nInitSize )
		Initialize( nInitSize );
}

template <class T>
CMyRingPool<T>::~CMyRingPool()
{
	Invalidate();
}

#pragma pack(pop)

#endif // __MY_RING_POOL_TEMPLATE_20030116__
