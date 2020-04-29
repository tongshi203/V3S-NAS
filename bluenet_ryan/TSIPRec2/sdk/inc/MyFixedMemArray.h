#ifndef __MY_FIXEDMEM_ARRAY_H_20031023__
#define __MY_FIXEDMEM_ARRAY_H_20031023__

#include <MyMemoryMgr.h>

//-----------------------------------------------
//	如果定义宏 __MMM_NOT_INIT_MEMORY_CONSTRUCTOR__，则构造函数时不初始化内存

template<class T, int MMM_MAX_ITEM_COUNT=1024>
class CMyFixedMemArray : public CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>
{
public:
// Construction
	CMyFixedMemArray();

// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	bool SetSize(int nNewSize, int nGrowBy = -1);

// Operations
	// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	T GetAt(int nIndex) const;
	void SetAt(int nIndex, T & newElement);
	T& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const T* GetData() const;
	T* GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, T & newElement);
	int Add(T & newElement);
	int Append(const CMyFixedMemArray& src);
	void Copy(const CMyFixedMemArray& src);

	// overloaded operator helpers
	T operator[](int nIndex) const;
	T& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, T & newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CMyFixedMemArray* pNewArray);

	void Preset();
	int GetOriginalIndex( int nIndex );

private:
	void FreeUnits(int nStart, int nCount=1);

// Implementation
protected:
	int	m_anMapTbl[MMM_MAX_ITEM_COUNT];
	int m_nSize;     // # of elements (upperBound - 1)

public:
	~CMyFixedMemArray();
};

/////////////////////////////////////////////////////////////////////////////
// CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT> inline functions

template<class T, int MMM_MAX_ITEM_COUNT>
MY_INLINE int CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::GetSize() const
	{ return m_nSize; }
template<class T, int MMM_MAX_ITEM_COUNT>
MY_INLINE int CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::GetUpperBound() const
	{ return m_nSize-1; }
template<class T, int MMM_MAX_ITEM_COUNT>
MY_INLINE void CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::RemoveAll()
	{ SetSize(0, -1); }

template<class T, int MMM_MAX_ITEM_COUNT>
MY_INLINE T CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::GetAt(int nIndex) const
{ 
	ASSERT(nIndex >= 0 && nIndex < m_nSize);

	nIndex = m_anMapTbl[nIndex];

	ASSERT( nIndex >= 0 && nIndex < MMM_MAX_ITEM_COUNT );
	assert( FALSE == (CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::m_aDataItems[nIndex].m_bIsFreed) );

	return CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::m_aDataItems[nIndex].m_Data;
}

template<class T, int MMM_MAX_ITEM_COUNT>
MY_INLINE void CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::SetAt(int nIndex, T & newElement)
{		
	ASSERT(nIndex >= 0 && nIndex < m_nSize);

	nIndex = m_anMapTbl[nIndex];

	ASSERT( nIndex >= 0 && nIndex < MMM_MAX_ITEM_COUNT );
	assert( FALSE == (CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::m_aDataItems[nIndex].m_bIsFreed) );

	CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::m_aDataItems[nIndex].m_Data = newElement; 
}

template<class T, int MMM_MAX_ITEM_COUNT>
MY_INLINE T& CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::ElementAt(int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < m_nSize);

	nIndex = m_anMapTbl[nIndex];

	ASSERT( nIndex >= 0 && nIndex < MMM_MAX_ITEM_COUNT );
	assert( FALSE == (CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::m_aDataItems[nIndex].m_bIsFreed) );

	return CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::m_aDataItems[nIndex].m_Data;
}

template<class T, int MMM_MAX_ITEM_COUNT>
MY_INLINE const T* CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::GetData() const
{
	ASSERT( FALSE );		// do not provided
	return NULL; 
}
template<class T, int MMM_MAX_ITEM_COUNT>
MY_INLINE T* CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::GetData()
{ 
	ASSERT( FALSE );		// do not provided
	return NULL; 
}

template<class T, int MMM_MAX_ITEM_COUNT>
MY_INLINE int CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::Add(T & newElement)
{	
	int nIndex = m_nSize;
	SetAtGrow(nIndex, newElement);
	return nIndex; 
}

template<class T, int MMM_MAX_ITEM_COUNT>
MY_INLINE T CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::operator[](int nIndex) const
	{ return GetAt(nIndex); }

template<class T, int MMM_MAX_ITEM_COUNT>
MY_INLINE T& CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::operator[](int nIndex)
	{ return ElementAt(nIndex); }

/////////////////////////////////////////////////////////////////////////////
// CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT> out-of-line functions

template<class T, int MMM_MAX_ITEM_COUNT>
CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::CMyFixedMemArray()
{
#ifndef __MMM_NOT_INIT_MEMORY_CONSTRUCTOR__
	Preset();
#endif // __MMM_NOT_INIT_MEMORY_CONSTRUCTOR__
}

template<class T, int MMM_MAX_ITEM_COUNT>
void CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::Preset()
{
	CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::Preset();

	m_nSize = 0;
	for(int i=0; i<MMM_MAX_ITEM_COUNT; i++)
	{
		m_anMapTbl[i] = CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::MMM_INVALID_PTR;
	}	
}

template<class T, int MMM_MAX_ITEM_COUNT>
CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::~CMyFixedMemArray()
{
#ifndef __MMM_NOT_INIT_MEMORY_CONSTRUCTOR__
	FreeUnits( 0, m_nSize );
#endif // __MMM_NOT_INIT_MEMORY_CONSTRUCTOR__	
}

///-------------------------------------------------------
/// CYJ,2003-10-24
/// Function:
///		Free units
/// Input parameter:
///		nStart			start index
///		nCount			count to be freed
/// Output parameter:
///		None
template<class T, int MMM_MAX_ITEM_COUNT>
void CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::FreeUnits(int nStart, int nCount)
{
	for(int i=0; i<nCount; i++)
	{
		int nIndex = m_anMapTbl[nStart];
		m_anMapTbl[nStart] = CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::MMM_INVALID_PTR;
		CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::MyFree( nIndex );
		nStart ++;
	}
}

template<class T, int MMM_MAX_ITEM_COUNT>
bool CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::SetSize(int nNewSize, int nGrowBy)
{
#ifdef _DEBUG
	ASSERT(nNewSize >= 0);
	ASSERT( (CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::m_nItemAllocaed) == m_nSize );
#endif //_DEBUG

	if (nNewSize == 0)
	{
		FreeUnits( 0, m_nSize );		// free all units
		m_nSize = 0;
#ifdef _DEBUG
		for(int i=0; i<MMM_MAX_ITEM_COUNT; i++)
		{
			assert( m_anMapTbl[i] == (CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::MMM_INVALID_PTR) );
		}
#endif //_DEBUG
	}
	else if ( nNewSize == m_nSize )
		return true;
	else if( nNewSize < m_nSize )
	{									//	free some element
		FreeUnits( nNewSize, m_nSize-nNewSize );		// ???

#ifdef _DEBUG
		for(int i=nNewSize; i<m_nSize; i++)
		{
			assert( m_anMapTbl[i] == (CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::MMM_INVALID_PTR) );
		}
#endif //_DEBUG

		m_nSize = nNewSize;
	}
	else
	{
		assert( nNewSize > m_nSize );
		for( int i=m_nSize; i<nNewSize; i++)
		{
			int nIndex = CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::MyAllocate();
			if( nIndex < 0 )
			{
#ifdef _DEBUG
				assert( FALSE );
#endif //_DEBUG

#ifdef _MFC_VER
			AfxThrowMemoryException();
#endif //_MFC_VER
				return false;
			}
			assert( m_anMapTbl[i] == (CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::MMM_INVALID_PTR) );
			m_anMapTbl[i] = nIndex;
		}
		m_nSize = nNewSize;
	}
    return true;
}

template<class T, int MMM_MAX_ITEM_COUNT>
int CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::Append(const CMyFixedMemArray& src)
{
	ASSERT(this != &src);   // cannot append to itself

	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);
	for( int i=0; i<src.m_nSize; i++)
	{
		ElementAt(nOldSize+i) = src.ElementAt( i );
	}
	return nOldSize;
}

template<class T, int MMM_MAX_ITEM_COUNT>
void CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::Copy(const CMyFixedMemArray& src)
{
	ASSERT(this != &src);   // cannot append to itself

	SetSize(src.m_nSize);
	for( int i=0; i<src.m_nSize; i++)
	{
		ElementAt(i) = src.ElementAt( i );
	}
}

template<class T, int MMM_MAX_ITEM_COUNT>
void CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::FreeExtra()
{
#ifdef _DEBUG
	assert( FALSE );
#endif //_DEBUG
}

template<class T, int MMM_MAX_ITEM_COUNT>
void CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::SetAtGrow(int nIndex, T & newElement)
{
#ifdef _DEBUG
	ASSERT( nIndex >= 0 && nIndex<MMM_MAX_ITEM_COUNT );
	ASSERT( (CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::m_nItemAllocaed) == m_nSize );
#endif //_DEBUG

	if (nIndex >= m_nSize)
		SetSize(nIndex+1, -1);
	ElementAt(nIndex) = newElement;
}

template<class T, int MMM_MAX_ITEM_COUNT>
void CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::InsertAt(int nIndex, T & newElement, int nCount /*=1*/)
{
	ASSERT( nIndex >= 0 && nIndex<MMM_MAX_ITEM_COUNT );    // will expand to meet need
	ASSERT(nCount > 0);     // zero or negative size not allowed
	if( nCount <= 0 )
		return;

	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		SetSize(nIndex + nCount, -1);   // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		int nMoveCount = m_nSize - nIndex;
		memmove( &m_anMapTbl[nIndex+nCount], &m_anMapTbl[nIndex], nMoveCount*sizeof(int) );
		BOOL bFailed = FALSE;
		for( int i=0; i<nCount; i++)
		{
			int nAllocateNo = CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::MyAllocate();
			m_anMapTbl[i+nIndex] = nAllocateNo;
			if( nAllocateNo >= 0 )
				continue;
			bFailed = TRUE;
			assert( FALSE );
		}
		if( bFailed )
		{
#ifdef _MFC_VER
			AfxThrowMemoryException();
#endif //_MFC_VER
			return;
		}
		m_nSize += nCount;		
	}

	// insert new value in the gap
	ASSERT(nIndex + nCount <= m_nSize);
	while (nCount--)
		ElementAt( nIndex++ ) = newElement;
}

template<class T, int MMM_MAX_ITEM_COUNT>
void CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::RemoveAt(int nIndex, int nCount)
{
	ASSERT( nIndex >= 0 && nIndex<MMM_MAX_ITEM_COUNT );
	ASSERT(nCount >= 0);
	ASSERT(nIndex + nCount <= m_nSize);

	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);
	FreeUnits( nIndex, nCount );					// free memeory
	if (nMoveCount)
		memmove( &m_anMapTbl[nIndex], &m_anMapTbl[nIndex + nCount], nMoveCount*sizeof(int) );

	int nOldSize = m_nSize;
	m_nSize -= nCount;
	for(int i=m_nSize; i<nOldSize; i++)
	{
		m_anMapTbl[i] = CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::MMM_INVALID_PTR;
	}
}

template<class T, int MMM_MAX_ITEM_COUNT>
void CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::InsertAt(int nStartIndex, CMyFixedMemArray* pNewArray)
{
	ASSERT(pNewArray != NULL);
	ASSERT(nStartIndex >= 0);

	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}

///-------------------------------------------------------
/// CYJ,2003-10-25
/// Function:
///		Get original index, that m_anMapTbl[nNo] = nRetVal;
/// Input parameter:
///		None
/// Output parameter:
///		None
template<class T, int MMM_MAX_ITEM_COUNT>
int CMyFixedMemArray<T,MMM_MAX_ITEM_COUNT>::GetOriginalIndex( int nIndex )
{
	ASSERT( nIndex >= 0 && nIndex<MMM_MAX_ITEM_COUNT );
	if( nIndex < 0 || nIndex >= MMM_MAX_ITEM_COUNT )
		return CMyFixedMemoryManage<T,MMM_MAX_ITEM_COUNT>::MMM_INVALID_PTR;
	return m_anMapTbl[nIndex];
}

#endif // __MY_FIXEDMEM_ARRAY_H_20031023__

