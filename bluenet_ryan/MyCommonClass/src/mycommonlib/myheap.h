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
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视公司”内部使用!				 !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若您要使用该文件，您需要承担这方面的风险!		 !
///=======================================================

#if !defined(AFX_MYHEAP_H__B623E84A_F7BC_4156_A597_93B5739F482F__INCLUDED_)
#define AFX_MYHEAP_H__B623E84A_F7BC_4156_A597_93B5739F482F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// 2016.12.17 CYJ Add
#pragma pack(push,8)

class CMyHeap  
{
public:
	bool Write( void * pBuf, int nCount );
	int GetMemoryAllocated() const;
	bool IsValid();
	void CancelLastAllocate();
	PBYTE Allocate( int nSize );
	void Reset();
	PBYTE Detach( int * pnHeapSize );
	bool Attach( PBYTE pBuf, int nSize );
	void DestroyHeap();
	bool CreateHeap( int nHeapSize = 4096 );
	CMyHeap(int nHeapSize=4096);
	CMyHeap(PBYTE pBuf, int nHeapSize);
	virtual ~CMyHeap();
	const PBYTE GetHeapBuf() const { return m_pHeapBuf; }
	const int GetHeapSize()const { return m_nHeapSize; }
	const int GetHeapMaxSize()const{ return m_nHeapMaxSize; }		//  CYJ,2009-6-23 增加

private:
	bool	m_bIsAttached;
	PBYTE	m_pHeapBuf;
	int		m_nHeapSize;		//	当前堆大小
	int		m_nHeapMaxSize;		//	可以分配的最大字节数
	int		m_nLastAllocatedSize;
	int		m_nTotalBytesAllocaed;
};

// 2016.12.17 CYJ Add
#pragma pack(pop)

#endif // !defined(AFX_MYHEAP_H__B623E84A_F7BC_4156_A597_93B5739F482F__INCLUDED_)
