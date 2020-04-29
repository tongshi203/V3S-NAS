///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-1-10
///
///		用途：
///			“堆”内存管理
///		
///			本堆管理器，只能按取消上一次的分配，而不能释放任何分配的内存。
///			设计这个类的目的是为了辅助内存中的动态数据（数据结构）
///
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视公司”内部使用!				 !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若您要使用该文件，您需要承担这方面的风险!		 !
///=======================================================

#include "stdafx.h"
#include "myheap.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyHeap::CMyHeap(int nHeapSize)
{
	m_bIsAttached = false;
	m_pHeapBuf = NULL;
	m_nHeapSize = 0;
	m_nLastAllocatedSize = 0;
	m_nTotalBytesAllocaed = 0;
	m_nHeapMaxSize = nHeapSize;

	if( nHeapSize )
		CreateHeap( nHeapSize );
}

CMyHeap::CMyHeap(PBYTE pBuf, int nHeapSize)
{
	m_bIsAttached = false;
	m_pHeapBuf = NULL;
	m_nHeapSize = 0;
	m_nLastAllocatedSize = 0;
	m_nTotalBytesAllocaed = 0;
	ASSERT( pBuf && nHeapSize );

	if( pBuf && nHeapSize )
		Attach( pBuf, nHeapSize );
}

CMyHeap::~CMyHeap()
{
	DestroyHeap();
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		Initialize and create heap
/// Input parameter:
///		nHeapSize		heap size
/// Output parameter:
///		true			succ
///		false			failed
bool CMyHeap::CreateHeap(int nHeapSize)
{
	ASSERT( nHeapSize );
	if( 0 == nHeapSize )
		return false;
	DestroyHeap();

	// 先记录堆的最大尺寸
	m_nHeapMaxSize = nHeapSize;

	if( nHeapSize > 8192 )		// 超过 8 KB，转换为自动根据需要分配
		nHeapSize = 8192;

	m_nHeapSize = 0;
	m_pHeapBuf = (PBYTE)malloc( nHeapSize );
	if( NULL == m_pHeapBuf )
		return false;
	m_nHeapSize = nHeapSize;

	Reset();

	return true;	
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		invalidate and free the memory that allocated for heap
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHeap::DestroyHeap()
{
	if( false == IsValid() )
		return;
	if( m_bIsAttached )
		return;

	free( m_pHeapBuf );

	m_pHeapBuf = NULL;
	m_nHeapSize = 0;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		attach one allocaed memory to the object
/// Input parameter:
///		pBuf				allocated memory
///		nSize				heap size
/// Output parameter:
///		true				succ
///		false				failed
bool CMyHeap::Attach(PBYTE pBuf, int nSize)
{
	ASSERT( false == IsValid() );
	if( IsValid() )
		return false;

	ASSERT( pBuf && nSize );
	if( NULL == pBuf || 0 == nSize )
		return false;

	m_pHeapBuf = pBuf;
	m_nHeapSize = nSize;
	
	m_bIsAttached = true;		//  CYJ,2005-12-12 添加，以前忘了

	Reset();

	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		Detach
/// Input parameter:
///		pnHeapSize			output heap size
/// Output parameter:
///		memory address
PBYTE CMyHeap::Detach(int *pnHeapSize)
{
	if( pnHeapSize )
		*pnHeapSize = m_nHeapSize;

	PBYTE pRetVal = m_pHeapBuf;

	m_pHeapBuf = NULL;
	m_nHeapSize = 0;
	Reset();

	return pRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		reset
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHeap::Reset()
{
	m_nLastAllocatedSize = 0;
	m_nTotalBytesAllocaed = 0;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		Allocate memory
/// Input parameter:
///		nSize			size in bytes
/// Output parameter:
///		NULL			failed
///		else			succ, memory address
PBYTE CMyHeap::Allocate(int nSize)
{
	if( nSize + m_nTotalBytesAllocaed > m_nHeapSize )
	{			// 缓冲区不够
		if( m_bIsAttached || m_nHeapSize >= m_nHeapMaxSize )
			return NULL;			// 不能在分配了
		int nHeapSize = m_nHeapSize;
		int nBufNeed = nSize + m_nTotalBytesAllocaed;
		while( nHeapSize < nBufNeed )
		{
			nHeapSize += 8192;
			if( nHeapSize > m_nHeapMaxSize )
			{
				nHeapSize = m_nHeapMaxSize;
				break;
			}
		}
		if( nBufNeed > nHeapSize )
			return NULL;			// 不能在分配了
		m_nHeapSize = 0;
		if( m_pHeapBuf )
			m_pHeapBuf = (PBYTE)realloc( m_pHeapBuf, nHeapSize );
		else
			m_pHeapBuf = (PBYTE)malloc( nHeapSize );
		if( NULL == m_pHeapBuf )
			return NULL;
		m_nHeapSize = nHeapSize;
	}

	m_nLastAllocatedSize = nSize;

	PBYTE pRetVal = m_pHeapBuf + m_nTotalBytesAllocaed;

	m_nTotalBytesAllocaed += nSize;

	return pRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		Cancel last allocate
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHeap::CancelLastAllocate()
{
	m_nTotalBytesAllocaed -= m_nLastAllocatedSize;
	m_nLastAllocatedSize = 0;

	ASSERT( m_nTotalBytesAllocaed >= 0 && m_nTotalBytesAllocaed <= m_nHeapSize );
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		Is heap object valid
/// Input parameter:
///		None
/// Output parameter:
///		true				有效
///		false				无效
bool CMyHeap::IsValid()
{
	return m_nHeapSize && m_pHeapBuf;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		Get memory
/// Input parameter:
///		None
/// Output parameter:
///		获取已经分配的字节数据，也可以说是已经写入的字节数
int CMyHeap::GetMemoryAllocated() const
{
	return m_nTotalBytesAllocaed;
}

///-------------------------------------------------------
/// CYJ,2005-1-11
/// 函数功能:
///		写入数据，先分配内存，然后再写入数据
/// 输入参数:
///		pBuf				缓冲区
///		nCount				字节大小
/// 返回参数:
///		true				成功
///		false				失败
bool CMyHeap::Write(void *pBuf, int nCount)
{
	PBYTE pDstBuf = Allocate( nCount );
	if( NULL == pDstBuf )
		return false;
	memcpy( pDstBuf, pBuf, nCount );
	return true;
}
